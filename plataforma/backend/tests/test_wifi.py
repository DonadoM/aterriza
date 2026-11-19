"""ESP32 por WiFi: la plataforma lee el mismo CSV por TCP (sin hardware: servidor falso)."""
import socket
import threading
import time

import pytest

from app import banco
from app.bridge import Link, wifi_url


class FakeEsp32:
    """Servidor TCP que se porta como el firmware: tras recibir 's' envía #COLS y filas,
    y responde 't' con el mensaje de tara. Acepta varias conexiones (reconexión)."""

    def __init__(self):
        self.srv = socket.socket()
        self.srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.srv.bind(("127.0.0.1", 0))
        self.srv.listen(2)
        self.port = self.srv.getsockname()[1]
        self.received = bytearray()
        self.conexiones = 0
        self.cortar = threading.Event()      # pide cerrar la conexión actual
        self.alive = True
        threading.Thread(target=self._serve, daemon=True).start()

    def _serve(self):
        while self.alive:
            try:
                conn, _ = self.srv.accept()
            except OSError:
                return
            self.conexiones += 1
            self.cortar.clear()
            threading.Thread(target=self._client, args=(conn,), daemon=True).start()

    def _client(self, conn):
        src = banco.BancoVirtualSource()
        conn.settimeout(0.01)
        try:
            while self.alive and not self.cortar.is_set():
                try:
                    data = conn.recv(64)
                    if data:
                        self.received += data
                        src.write(data)
                    elif data == b"":
                        return
                except socket.timeout:
                    pass
                line = src.readline()
                if line:
                    conn.sendall(line.encode() + b"\n")
        except OSError:
            pass
        finally:
            conn.close()

    def close(self):
        self.alive = False
        self.cortar.set()
        self.srv.close()


@pytest.fixture
def esp32():
    s = FakeEsp32()
    yield s
    s.close()


def _espera(cond, timeout=6.0):
    t0 = time.monotonic()
    while time.monotonic() - t0 < timeout:
        if cond():
            return True
        time.sleep(0.05)
    return False


def test_wifi_url_formas():
    assert wifi_url() == "socket://aterriza.local:3333"
    assert wifi_url("192.168.4.1") == "socket://192.168.4.1:3333"
    assert wifi_url("192.168.4.1:4000") == "socket://192.168.4.1:4000"
    assert wifi_url("aterriza.local", 3334) == "socket://aterriza.local:3334"


@pytest.mark.parametrize("host", ["a b", "x?logging=debug", "host/../", "-malo", ""[:0] + "bad host"])
def test_wifi_url_rechaza_entradas_raras(host):
    with pytest.raises(ValueError):
        wifi_url(host)


def test_wifi_puerto_fuera_de_rango():
    with pytest.raises(ValueError):
        wifi_url("192.168.4.1", 70000)


def test_link_wifi_lee_el_csv_y_envia_comandos(esp32):
    link = Link()
    try:
        assert link.connect("wifi", host=f"127.0.0.1:{esp32.port}")
        assert link.kind == "wifi"
        assert _espera(lambda: link.rows_total >= 30)
        assert len(link.columns) == 36                       # mismo CSV que por USB
        assert link.gaps == 0 and link.bad == 0
        link.send("t")
        assert _espera(lambda: b"t" in esp32.received)       # el comando llega al firmware
        assert _espera(lambda: link.tared_event.is_set())    # y su respuesta "#OK tara" vuelve
    finally:
        link.disconnect()


def test_link_wifi_se_reconecta_si_se_corta(esp32):
    link = Link()
    try:
        assert link.connect("wifi", host="127.0.0.1", wifi_port=esp32.port)
        assert _espera(lambda: link.rows_total >= 10)
        esp32.cortar.set()                                   # el ESP32 cierra el socket
        assert _espera(lambda: link.reconnects >= 1 and link.status == "conectado")
        antes = link.rows_total
        assert _espera(lambda: link.rows_total > antes + 10)  # vuelven los datos
        assert esp32.conexiones >= 2
    finally:
        link.disconnect()


def test_link_wifi_sin_equipo_da_un_error_claro():
    s = socket.socket()
    s.bind(("127.0.0.1", 0))
    port = s.getsockname()[1]
    s.close()                                                # puerto libre: nadie escucha
    link = Link()
    assert not link.connect("wifi", host=f"127.0.0.1:{port}")
    assert link.status == "error"
    assert "misma red" in link.error


def test_api_conectar_wifi(esp32):
    from fastapi.testclient import TestClient

    from app.main import app
    with TestClient(app) as c:
        r = c.post("/api/conectar", json={"fuente": "wifi", "host": f"127.0.0.1:{esp32.port}"})
        try:
            assert r.status_code == 200, r.text
            assert r.json()["tipo"] == "wifi"
            assert _espera(lambda: c.get("/api/estado").json()["hz"] > 40)
        finally:
            c.post("/api/desconectar")


def test_api_conectar_wifi_host_invalido():
    from fastapi.testclient import TestClient

    from app.main import app
    with TestClient(app) as c:
        r = c.post("/api/conectar", json={"fuente": "wifi", "host": "no es un host"})
        assert r.status_code == 400
        assert "no válida" in r.json()["detail"]

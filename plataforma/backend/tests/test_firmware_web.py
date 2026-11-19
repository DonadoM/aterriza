"""Compila y ejecuta las pruebas en C++ del firmware (SHA-256, autenticación Basic,
JSON de estado) y valida el JSON con un parser real. Se omite si no hay compilador."""
import json
import os
import shutil
import subprocess
from pathlib import Path

import pytest

FW = Path(__file__).resolve().parents[3] / "firmware"
CXX = shutil.which("c++") or shutil.which("g++") or shutil.which("clang++")

pytestmark = pytest.mark.skipif(CXX is None, reason="sin compilador C++")


@pytest.fixture(scope="module")
def salida(tmp_path_factory):
    exe = tmp_path_factory.mktemp("fw") / ("host_test.exe" if os.name == "nt" else "host_test")
    subprocess.run(
        [CXX, "-std=gnu++17", "-I", str(FW / "BiomecanicaESP32"), str(FW / "tests/host_test.cpp"),
         str(FW / "BiomecanicaESP32/StatusJson.cpp"), "-o", str(exe)],
        check=True, capture_output=True, text=True,
    )
    r = subprocess.run([str(exe)], capture_output=True, text=True)
    return r


def _json(salida, caso):
    for linea in salida.stdout.splitlines():
        if linea.startswith(f"JSON {caso} "):
            return json.loads(linea.split(" ", 2)[2])
    raise AssertionError(f"sin caso {caso}")


def test_pruebas_cpp_pasan(salida):
    assert salida.returncode == 0, salida.stdout
    assert salida.stdout.strip().endswith("OK")


def test_json_placa_real(salida):
    j = _json(salida, "conectado")
    assert [i["nombre"] for i in j["imu"]] == ["Muslo", "Pierna", "Lumbar"]
    assert [i["estado"] for i in j["imu"]] == ["ok", "ok", "no_conectada"]
    assert j["imu"][1]["modelo"] == "GY-85" and j["imu"][1]["magnetometro"] == "HMC5883L"
    assert j["imu"][2]["acel_g"] is None
    assert j["wifi"]["estado"] == "conectado" and j["wifi"]["rssi_dbm"] == -61


def test_json_valores_no_medibles_y_comillas(salida):
    j = _json(salida, "ap_nan_comillas")
    assert j["wifi"]["ssid"] == 'Mi "red" \\ casa'
    assert j["wifi"]["estado"] == "punto_acceso"
    assert j["imu"][0]["inclinacion_deg"] is None


def test_json_sin_red_ni_muestras(salida):
    j = _json(salida, "vacio")
    assert j["wifi"]["estado"] == "sin_red" and j["wifi"]["acceso_configurado"] is False
    assert all(i["estado"] == "no_conectada" for i in j["imu"])

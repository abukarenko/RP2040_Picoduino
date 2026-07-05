import os
import time

import serial


SSID = os.environ.get("PICODUINO_WIFI_SSID", "YOUR_WIFI_SSID")
PASSWORD = os.environ.get("PICODUINO_WIFI_PASSWORD", "YOUR_WIFI_PASSWORD")
BSSID = os.environ.get("PICODUINO_WIFI_BSSID", "")


def send(ser, command, wait_s=1, redacted=None):
    print(f">> {redacted or command}")
    ser.reset_input_buffer()
    ser.write((command + "\r\n").encode("ascii"))
    time.sleep(wait_s)
    print(ser.read(8192).decode("latin1", "replace"))


with serial.Serial("COM7", 115200, timeout=10, write_timeout=2) as ser:
    ser.dtr = True
    ser.rts = True
    time.sleep(0.5)

    send(ser, "ATE0")
    send(ser, "AT+CWMODE=1")
    send(ser, "AT+CWDHCP=1,1")
    send(
        ser,
        f'AT+CWJAP="{SSID}","{PASSWORD}"' + (f',"{BSSID}"' if BSSID else ""),
        wait_s=20,
        redacted=f'AT+CWJAP="{SSID}","**********"' + (f',"{BSSID}"' if BSSID else ""),
    )
    send(ser, "AT+CIFSR", wait_s=2)

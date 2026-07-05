import os
import time

import serial


SSID = os.environ.get("PICODUINO_WIFI_SSID", "YOUR_WIFI_SSID")
PASSWORD = os.environ.get("PICODUINO_WIFI_PASSWORD", "YOUR_WIFI_PASSWORD")


def send(ser, command, wait_s=1, redacted=None):
    print(f">> {redacted or command}", flush=True)
    ser.reset_input_buffer()
    ser.write((command + "\r\n").encode("ascii"))
    time.sleep(wait_s)
    print(ser.read(8192).decode("latin1", "replace"), flush=True)


with serial.Serial("COM7", 115200, timeout=10, write_timeout=2) as ser:
    ser.dtr = True
    ser.rts = True
    time.sleep(0.5)

    send(ser, "ATE0")
    send(ser, "AT+CWMODE=1")
    send(ser, "AT+CWDHCP=1,0")
    send(ser, 'AT+CIPSTA="192.168.0.222","192.168.0.1","255.255.255.0"')
    send(
        ser,
        f'AT+CWJAP="{SSID}","{PASSWORD}"',
        wait_s=15,
        redacted=f'AT+CWJAP="{SSID}","**********"',
    )
    send(ser, "AT+CIFSR", wait_s=2)

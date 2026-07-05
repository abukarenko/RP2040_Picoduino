import time

import serial


def send(ser, command, wait_s=1):
    print(f">> {command}", flush=True)
    ser.reset_input_buffer()
    ser.write((command + "\r\n").encode("ascii"))
    time.sleep(wait_s)
    print(ser.read(8192).decode("latin1", "replace"), flush=True)


with serial.Serial("COM7", 115200, timeout=8, write_timeout=2) as ser:
    ser.dtr = True
    ser.rts = True
    time.sleep(0.5)

    send(ser, "ATE0")
    send(ser, "AT+CWJAP?")
    send(ser, "AT+CIFSR")
    send(ser, "AT+CIPSTATUS")
    send(ser, 'AT+PING="192.168.0.1"', wait_s=4)
    send(ser, 'AT+PING="192.168.0.104"', wait_s=4)

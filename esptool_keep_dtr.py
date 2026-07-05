import sys

import serial


OriginalSerial = serial.Serial


class DtrRtsSerial(OriginalSerial):
    def open(self):
        super().open()
        self.setDTR(True)
        self.setRTS(True)

    def setDTR(self, level=True):
        return super().setDTR(True)

    def setRTS(self, level=True):
        return super().setRTS(True)


serial.Serial = DtrRtsSerial

import esptool


if __name__ == "__main__":
    sys.exit(esptool._main())

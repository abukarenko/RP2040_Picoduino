print("=== copy_uf2.py loaded ===")

Import("env")

import os
import time
import shutil
import string

try:
    import serial
except ImportError:
    serial = None


COM_PORT = "COM7"


def find_rp2_drive():
    for letter in string.ascii_uppercase:
        drive = f"{letter}:\\"
        info = os.path.join(drive, "INFO_UF2.TXT")
        if os.path.exists(info):
            return drive
    return None


def reset_to_bootsel(port):
    if serial is None:
        print("pyserial not found, skip auto reset")
        return

    print(f"Trying 1200bps reset on {port}...")

    try:
        ser = serial.Serial(port, 1200, timeout=1, write_timeout=1)
        ser.dtr = False
        ser.rts = False
        time.sleep(0.2)
        ser.close()
        print("Reset command sent")
    except Exception as e:
        print(f"Reset failed: {e}")


def wait_for_rp2_drive(timeout_sec=10):
    print("Waiting for RPI-RP2 disk...")

    start = time.time()
    while time.time() - start < timeout_sec:
        drive = find_rp2_drive()
        if drive:
            return drive
        time.sleep(0.5)

    return None


def copy_uf2_file(env, use_auto_reset):
    print("=== copy_uf2.py copy action started ===")

    uf2_path = env.subst("$BUILD_DIR/${PROGNAME}.uf2")
    print(f"UF2 path: {uf2_path}")

    if not os.path.exists(uf2_path):
        print("UF2 file not found")
        return

    drive = find_rp2_drive()
    if not drive and use_auto_reset:
        reset_to_bootsel(COM_PORT)
        drive = wait_for_rp2_drive(10)
    elif not drive:
        print("RPI-RP2 disk not found.")
        print("Hold BOOTSEL, press RESET, then run: pio run -t copyuf2")
        return
    else:
        print(f"RPI-RP2 already mounted: {drive}")

    if not drive:
        print("RPI-RP2 disk not found.")
        print("Press BOOTSEL manually and run: pio run -t uploaduf2")
        return

    print(f"RPI-RP2 found: {drive}")
    print(f"Copying {uf2_path} -> {drive}")

    shutil.copy(uf2_path, drive)

    print("UF2 copy done")


def copy_uf2(source, target, env):
    copy_uf2_file(env, use_auto_reset=True)


def copy_uf2_no_reset(source, target, env):
    copy_uf2_file(env, use_auto_reset=False)


# PlatformIO can skip "buildprog" when all artifacts are already fresh, so a
# post action there does not run on every plain "pio run". "checkprogsize" runs
# even for a no-op build, after firmware.uf2 is available, so this keeps the
# automatic UF2 upload behavior deterministic.
env.AddPostAction("checkprogsize", copy_uf2)  # type: ignore[name-defined]

env.AddCustomTarget(
    "uploaduf2",
    None,
    copy_uf2,
    title="Upload existing UF2",
    description="Copy the already built UF2 file to the RPI-RP2 bootloader drive without rebuilding",
)  # type: ignore[name-defined]

env.AddCustomTarget(
    "copyuf2",
    None,
    copy_uf2_no_reset,
    title="Copy existing UF2",
    description="Copy the already built UF2 file to an already mounted RPI-RP2 drive without rebuilding or touching COM",
)  # type: ignore[name-defined]

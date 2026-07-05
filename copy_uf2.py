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


def get_monitor_port(env):
    port = env.GetProjectOption("monitor_port", "")
    if port:
        return port

    port = env.GetProjectOption("upload_port", "")
    if port:
        return port

    return None


def list_drives():
    drives = set()
    for letter in string.ascii_uppercase:
        drive = f"{letter}:\\"
        if os.path.exists(drive):
            drives.add(drive)
    return drives


def is_uf2_drive(drive):
    return os.path.exists(os.path.join(drive, "INFO_UF2.TXT"))


def find_uf2_drive(exclude=None):
    exclude = exclude or set()
    for drive in sorted(list_drives() - exclude):
        if is_uf2_drive(drive):
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


def wait_for_new_uf2_drive(before_drives, timeout_sec=10):
    print("Waiting for a new UF2 disk...")

    start = time.time()
    while time.time() - start < timeout_sec:
        drive = find_uf2_drive(exclude=before_drives)
        if drive:
            return drive

        new_drives = sorted(list_drives() - before_drives)
        for new_drive in new_drives:
            if is_uf2_drive(new_drive):
                return new_drive

        time.sleep(0.25)

    return None


def copy_uf2_file(env, use_auto_reset):
    print("=== copy_uf2.py copy action started ===")

    uf2_path = env.subst("$BUILD_DIR/${PROGNAME}.uf2")
    print(f"UF2 path: {uf2_path}")

    if not os.path.exists(uf2_path):
        print("UF2 file not found")
        return

    before_drives = list_drives()
    drive = find_uf2_drive()
    if not drive and use_auto_reset:
        port = get_monitor_port(env)
        if not port:
            print("monitor_port is not set in platformio.ini")
            print("Set monitor_port or hold BOOTSEL manually and run: pio run -t copyuf2")
            return

        reset_to_bootsel(port)
        drive = wait_for_new_uf2_drive(before_drives, 10)
    elif not drive:
        print("UF2 disk not found.")
        print("Hold BOOTSEL, press RESET, then run: pio run -t copyuf2")
        return
    else:
        print(f"UF2 disk already mounted: {drive}")

    if not drive:
        print("New UF2 disk not found.")
        print("Press BOOTSEL manually and run: pio run -t uploaduf2")
        return

    print(f"UF2 disk found: {drive}")
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
    description="Copy the already built UF2 file to the bootloader UF2 drive without rebuilding",
)  # type: ignore[name-defined]

env.AddCustomTarget(
    "copyuf2",
    None,
    copy_uf2_no_reset,
    title="Copy existing UF2",
    description="Copy the already built UF2 file to an already mounted UF2 drive without rebuilding or touching COM",
)  # type: ignore[name-defined]

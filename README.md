# attiny_switch

Repository layout:
- `src/`: ATtiny1614 firmware (PlatformIO project)
- `pcb/`: KiCad project and gerbers

## Firmware target

- MCU: `ATtiny1614`
- `PA1`: ISP772T `IN` control
- `PA0`: dedicated `UPDI/RESET` pin, reserved for programming/debug and **not** used for switch control

Current behavior:
- On reset, drive `PA1` low immediately
- Then loop forever: OFF 50 min, ON 10 min
- MCU sleeps in `STANDBY` and wakes from RTC once per minute

## PlatformIO

`platformio.ini` is configured for bare-metal register-level C (no Arduino framework).

Build:

```powershell
pio run
```

Upload over UPDI (PA0):

1. Set your serial port in `platformio.ini` (`upload_port = COMx`).
2. Run:

```powershell
pio run -t upload
```

The configured upload command uses `avrdude` with `serialupdi`.

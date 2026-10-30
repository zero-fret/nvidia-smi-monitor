# nvidia-smi-monitor

Single-line GPU monitor for Windows console. Polls `nvidia-smi` every 2 seconds and refreshes one line in place.

## Output

```
NVIDIA GeForce RTX 4050 Laptop GPU  9/58W  0/6141MiB  0%
```

Fields: GPU name, power draw / power cap, memory used / total, GPU utilization.

## Build

MinGW:
```
g++ -O2 nvidia_tray.cpp -o nvidia_tray.exe -luser32 -lshell32
```

MSVC (Developer Command Prompt):
```
cl /O2 nvidia_tray.cpp /link user32.lib shell32.lib
```

## Run

```
nvidia_tray.exe
```

Ctrl+C to exit.

## Notes

- Data comes from `nvidia-smi --query-gpu=... --format=csv,noheader,nounits`, a stable structured interface — not parsed from the ASCII table.
- Power cap uses `enforced.power.limit` (the actual power ceiling shown in the default `nvidia-smi` output). `power.limit` returns `[N/A]` on laptops because it is the software-settable limit, which laptop firmware does not expose.
- Requires NVIDIA drivers installed and `nvidia-smi.exe` on PATH (usually in `C:\Windows\System32\`).

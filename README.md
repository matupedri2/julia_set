# Julia Set

Real-time animated Julia set fractal renderer written in C, built as a 
low-level performance study comparing C and ARM64 Assembly implementations.

## What it does

The complex parameter *c* rotates continuously over time, animating the 
fractal at runtime. Each frame is timed in milliseconds and CPU cycles 
using a custom POSIX high-precision timer, reporting cycles-per-pixel 
to measure rendering cost directly.

## Technical details

- **Tile-based pipeline:** screen is divided into 8×8 pixel tiles to 
  improve cache locality during pixel iteration
- **Escape-time algorithm:** up to 64 iterations per pixel, bail-out 
  at |z|² > 4
- **Timing:** custom `chrono` module using `gettimeofday` — reports ms, 
  total cycles, and cycles/pixel per frame
- **Platform:** Linux x86/x64 and ARM (VFP)

## Build

Requires GCC and SDL 1.x (`libSDL-dev`).

```bash
chmod +x compile.sh
./compile.sh
```
 
# Run
./julia.elf <cpu_mhz>
Example — 3.6 GHz CPU:
./julia.elf 3600

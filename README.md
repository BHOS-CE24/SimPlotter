# SimPlotter
A low-level plotting application implemented in C++ using SDL2 graphics library.

## Build

**From source:**
```bash
g++ main.cpp -o plotter $(sdl2-config --cflags --libs) -lm
```

**With CMake:**
```bash
mkdir build && cd build
cmake ..
make
```

**Prebuilt binaries** (pick your architecture):
```bash
chmod +x ./bin/X_86  # or ./bin/aarch64
./bin/X_86             # or ./bin/aarch64
```

## Usage

Run `./plotter` and follow the menu.

| Choice | What it does |
|--------|-------------|
| 1 → Regular | Plot `f(x)` — e.g. `sin(x)/x^2 red` |
| 1 → Parametric | Plot `x(t), y(t)` — e.g. `cos(x) sin(x) cyan` |
| 2 | Evaluate an expression — e.g. `2^10 + pi` |
| 3 | List expression history |
| 4 | Exit |

**Colours:** `red` `green` `blue` `yellow` `cyan` `magenta` `orange` `purple` `black`

**Functions:** `sin` `cos` `tan` `asin` `acos` `atan` `sinh` `cosh` `tanh` `sqrt` `log` `ln` `exp` `abs` `ceil` `floor` `round`

**Constants:** `pi` `e` &nbsp;|&nbsp; **Operators:** `+ - * / % ^ !`

## Controls (graph window)

| Input | Action |
|-------|--------|
| Arrow keys | Pan |
| Scroll wheel | Zoom |
| Left drag | Pan |
| Middle mouse click / Home | Reset view |
| Escape | Close |

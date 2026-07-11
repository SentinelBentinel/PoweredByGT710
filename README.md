<img src="/assets/cube.png" width=128>

Powered by the raw computational might of a GT 710, though in practice it runs on the CPU and is more of a lightweight graphics experiment than a serious GPU replacement. 


<img src="/assets/gt710.png" width = 256>

## Build from MSYS2 UCRT64

Open the MSYS2 UCRT64 shell and run:

```sh
pacman -Syu
pacman -S --needed \
  base-devel \
  git \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-SDL3 \
  mingw-w64-ucrt-x86_64-sdl3_ttf
  pkg-config
```

Then build the project:

```sh
git clone <repo>
cd PoweredByGT710
cmake -S . -B build -G Ninja
cmake --build build
```

Run the executable:

```sh
./build/gurt.exe
```

## Notes

- The project expects SDL3 to be available in your MSYS2 UCRT64 environment.
- If CMake cannot find SDL3, make sure the UCRT64 shell is active and the package above is installed.

# Process Viewer Lite

A Windows process viewer written in C++20 with the Win32 API. It is a learning
project for Windows process monitoring and desktop programming.

The process list shows process IDs, CPU and memory use, disk I/O rates, and
executable paths. The application also shows system information and refreshes
the display in the background.

The New Process and Kill Process buttons are placeholders. They do not perform
those actions.

## Build

Use Windows, a C++20 compiler, and CMake 3.30 or later. For example, with
Visual Studio 2022:

```powershell
git clone https://github.com/uoosef403/processlite.git
cd processlite
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

CMake names the executable `untitled3.exe`. With the Visual Studio generator,
it is placed in `build/Release/`.

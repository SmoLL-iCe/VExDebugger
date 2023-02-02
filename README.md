# VExDebugger

A debugger using VEH.

[![forthebadge](https://forthebadge.com/images/badges/made-with-c-plus-plus.svg)](https://forthebadge.com)

## What is it ?

is a simple access and write debugger that uses hardware breakpoints, and that manages debugging using the windows exception support system (based on the VEH of the Cheat Engine).

<h3 align="center">
  <img src="README/vex_debug_gui.png" alt="screeen" />
  <img src="README/vex_debug_form.png" alt="screeen" />
</h3>

> **Nota:** _This tool is in beta, it may have problems depending on some variant not yet tested._

## Compatibility

✔ Windows 64 bits or 32 bits

## Breakpoint types

- [x] Write
- [x] Read/Write
- [ ] Execute

## Download

[Releases](https://github.com/SmoLL-iCe/VExDebugger/releases)

## How to use this?

go to the releases part and download, attach the VExDebugger dll form or imgui in the process you want to debug.
use an injector for this.
add the addresses you want to know who writes or accesses.

## What was used to compile?

- **Use Visual Studio** - ([Download](https://visualstudio.microsoft.com/pt-br/))
- **Platform tools** - Visual Studio 2022 (v143)
- **C++ SDK version used** - 10.0.22621.0

### .NET Sample C++/CLI

- **Common Language Runtime Support** - CLR
- **.NET version** - v4.8

## Author

[SmoLL-iCe](https://github.com/SmoLL-iCe)

## Thanks

- **VEH inspired:** [cheat-engine](https://github.com/cheat-engine)

## License

[![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](https://raw.githubusercontent.com/SmoLL-iCe/VExDebugger/master/LICENSE)

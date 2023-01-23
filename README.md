# OM3D

Hello!

This is a very buggy and unproven implementation of the G-Buffer rendering. For some reason we had issues with drivers and whatnot, and the software rasterization pipeline (via debugger) was producing the intended results, while HW renderer was not. Also no API errors in either case 🙃.

## Building

Please refer to different git branches for different submissions. Refer to the README in each one of them for more details.

In all the cases, you can build and run the project in the same manner:
- With VSCode + CMake extension ⇒ just run it **in debug mode**! #ItJustWorks
- Manually:
  - On Windows:
	```ps1
	$BUILD_TYPE=Debug
	cmake -S . -B build -DCMAKE_BUILD_TYPE=$BUILD_TYPE
	cmake --build build --config $BUILD_TYPE
	./build/TP
	```
  - On Linux:
	```sh
	#!/bin/bash
	BUILD_TYPE=Debug
	cmake -S . -B build -DCMAKE_BUILD_TYPE=$BUILD_TYPE
	cmake --build build --config $BUILD_TYPE
	./build/TP
	```

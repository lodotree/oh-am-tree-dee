# OM3D

This is a complete implementation of the first TP, consisting of indexed rendering and culling.

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

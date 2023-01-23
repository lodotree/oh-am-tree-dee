# On the road to 1 Billion 🌳

## Running

The project can be built & run in the usual manner - 

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

## Playing around

We've provided a bunch of ~~heavy~~ scenes to stress the tests.
BTW, the load scene menu uses paths relative to the data folder.
So you can just type `beeg_forest.glb` to load a forest 😉.

Additional debug tools are provided for you to have fun 😃.

- The number of verts and tris drawn on screen is updated in real-time.
- "Generate chunky clustering" controls whether chunk clustering on scene load will occur at all.
  It allows you to measure the net impact of cluster construction on scene load times, and memory usage.
  (make sure to reload the scene if you change that)
- "Force full resolution" forces everything to be rendered at full resolution
- "Debug chunky clusters" puts colorful boxes around stuffs, _they probably mean something_


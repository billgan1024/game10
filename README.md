A game engine made with C++ and DirectX 11.


# Features
- GLTF scene loading
- Cascaded shadow maps
- Basic physics collision detection/resolution
- Real-time audio mixing

# Controls

WASD: move

Mouse: look around 

Q/E: move up/down

F4: play sine wave (multiple sounds can be played together)

![screenshot](screenshot.png)


# Dependencies

- .NET Developer Pack 4.2
- Inter font downloaded from https://rsms.me/inter/download/ 
- Fira Code font

# Setup guide

You need to set `"cmake.generator": "Visual Studio 18 2026"` in vscode settings otherwise cmake post build commands won't work because ninja is buggy.

When in doubt, delete `out` and `build` and use cmake: delete cache and reconfigure.
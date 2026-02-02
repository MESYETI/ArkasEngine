# Arkas Engine
WIP 3D engine with build-like maps

Currently has a demo map with two sectors and a team fortress 2 model

Controls:
- WASD for movement
- left or right arrow keys to turn camera (or use mouse)
- O to free the cursor
- P to lock the cursor back into the window

## Screenshot
![Screenshot](/img/screenshot2.png)

## Build
```
make
```
Requires SDL2 and stb (clone recursively!)

Make options:
- `PLAT=windows` - Cross compile to Windows (64-bit)
- `BUILD=release` - Compile a release build
- `ASAN=y` - Enable address santiser

## Preparation
Get a D compiler and the `dub` build system and build the `ark` tool in `tools/ark`. Then,
clone <https://github.com/mesyeti/ArkasAssets> and follow the build instructions in the
README. Then, make a folder next to the engine executable called `game` and put the
`base.ark` from the arkas assets in the folder.

## Usage
Arkas (in it's WIP state) can only be built standalone. It starts with a console with
these commands:

- `test-map`       - Load the test map
- `clear-scenes`   - Clear scenes
- `map <MAP NAME>` - Load `MAP NAME`. Maps are stored in the `maps` folder.
  (Note: textures do not work when maps are loaded with this command, will be fixed when
  the resource manager is added)
- `dl-map <MAP NAME>` - Saves the current loaded map as `MAP NAME` in the `maps` folder
- `help` - Shows a list of commands

## Graphics backends
### Implemented
- OpenGL 1.1

### Todo
- OpenGL 1.5 (for VBO)
- OpenGL 2.0 (for shaders)

## Community
Arkas Engine is discussed in the non-bloat collective: <https://discord.gg/zPf8RS2a4r>

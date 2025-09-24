# Arkas Engine
WIP 3D engine with build-like maps

Currently has a demo map with two sectors and a team fortress 2 model

Controls:
- WASD for movement
- left or right arrow keys to turn camera (or use mouse)
- O to free the cursor
- P to lock the cursor back into the window

## Screenshot
![Screenshot](/img/screenshot.png)

## Build
```
make
```
Requires SDL2 and stb (clone recursively!)

## Usage
Arkas (in it's WIP state) can only be built standalone. It starts with a console with
these commands:

- `test-map`       - Load the test map
- `clear-scenes`   - Clear scenes
- `map <MAP NAME>` - Load `MAP NAME`. Maps are stored in the `maps` folder.
  (Note: textures do not work when maps are loaded with this command, will be fixed when
  the resource manager is added)
- `dl-map <MAP NAME>` - Saves the current loaded map as `MAP NAME` in the `maps` folder

## Graphics backends
### Implemented
- OpenGL 1.1

### Todo
- OpenGL 1.5 (for VBO)
- OpenGL 2.0 (for shaders)

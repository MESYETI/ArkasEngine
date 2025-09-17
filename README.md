# Arkas Engine
WIP 3D engine with build-like maps

Currently has a demo map with two sectors and a team fortress 2 model

Controls:
- WASD for movement
- left or right arrow keys to turn camera (or use mouse)
- O to free the cursor
- P to lock the cursor back into the window

## Screenshot
![Screenshot](/screenshot.png)

## Build
```
make
```
Requires SDL2 and stb (clone recursively!)

## Usage
Arkas (in it's WIP state) can only be built standalone. It starts with a console with
these commands:

- `ae.hello_world` - Print hello world to the console
- `ae.test` - Load the test map
- `ae.clear_scenes` - Clear scenes

## Graphics backends
### Implemented
- OpenGL 1.1

### Todo
- OpenGL 1.5 (for VBO)
- OpenGL 2.0 (for shaders)

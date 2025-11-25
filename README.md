# Cnake
**Cnake** is a simple snake game written in pure C.

## How to build and run the program ?
To build it:
```console
$ cc nob.c -o nob
$ ./nob
```
To run it:
```console
$ ./build/cnake
```
## Controls
Use arrow buttons to change direction, space to pause.

## External dependencies:
- [Xlib](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html) - for graphics rendering
- [miniaudio](https://github.com/mackron/miniaudio) - for sound effects 
- [NoBuild (or nob)](https://github.com/tsoding/nob.h) - to build the project
- [FreeType ](https://github.com/freetype/freetype) - for font rendering

## TODO
- [X] Font rendering
- [X] Add start menu
- [ ] Add levels
- [X] Implement a death screen
- [ ] Make the game more smooth
- [X] Use nob.h to build the program
- [X] Add sounds effects using miniaudio
- [ ] Remove leaks 
- [ ] Optimize the code (speed and memory)

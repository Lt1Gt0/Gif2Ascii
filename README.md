# Gif2Ascii

- [Gif2Ascii](#gif2ascii)
  - [About](#about)
  - [Build info](#build-info)
  - [TODO](#todo)

## About

Convert gif data into ascii characters

__I did not use any external libraries for this project. I decided it would be fun to write my own implementation so in its current state it is very buggy__

The whole concept behind this project was to turn the raster data stored inside of a gif into an ascii representation shown in a terminal window. (An idea I want to use for [sustext](https://github.com/Lt1Gt0/sustext) in the future)

## Build info

To build the program

```bash
make
```

(I do plan on adding different build flags for debugging purposes)

To run the program

```bash
./bin/gif2ascii <filepath>
```

## TODO
  __HIGH PRIORITY__
  - [ ] Support gif87a format
  - [ ] Comment extensions
  - [ ] Frame display timing (currently set on a 1 second interval per frame)
  - [ ] Correct ascii mappings according to colors
  - [ ] Transparency 

  __MEDIUM PRIORITY__
  - [ ] Dump gif information to seperate file for viewing (maybe)
  - [ ] Image scaling (fit size of terminal window as best as possible if needed)
  - [ ] Change display method to a web browser (could be set as a flag passed in upon unning the program)
  
  __LOW PRIORITY__
  - [ ] Possible support for different unicode characters (could be set as a flag passed in upon running the program)

# Gif2Ascii

- [Gif2Ascii](#gif2ascii)
  - [About](#about)
  - [Build info](#build-info)
  - [Milestones before this repos creation](#milestones-before-this-repos-creation)

## About

Convert gif data into ascii characters

This project has been moved from one of my private repositories where I throw random things into. I decided to make a seperate respository for this project instead of cramming it in a crowded repo. The commit log will be a bit jumpy because for the most part the early commits will only contain the milestones I hit with this project up until the point where I got a working proof of concept which is my latest commit as of this project

The whole concept behind this project was to turn the raster data stored inside of a gif into an ascii representation shown in a terminal window. (An idea I want to use for [sustext](https://github.com/Lt1Gt0/sustext)) in the future

__I did not use any external libraries for this project, instead I wrote my own implementation so in its current state it is very buggy__

## Build info

To run the program

```bash
./bin/gif2ascii
```

(Currently the file that will be converted is done in the main method, this will be easy to turn into a command line argument in future versions)

## Milestones before this repos creation

(read [About](#About) if confused)

- Headers for GIF format
- Read All GIF File contents
- Working LZW (There were a lot of sad attempts at LZW between the previous commit and this one)
- Image Mapping
- Revamped LZW for 256 colors
- Working Proof on concept (Most recent progress)

#pragma once
#ifndef _TYPES_HPP_
#define _TYPES_HPP_

#include <stdint.h>

typedef uint8_t byte;

// GIF Specs use 'unsigned' to represent 2 bytes but on some
// machines it is 4 bytes, so I will just call it 'word' for simplicity
typedef uint16_t word;

struct Position {
    int X;
    int Y;
};

struct Size {
    int width;
    int height;
};

#endif // _TYPES_HPP_

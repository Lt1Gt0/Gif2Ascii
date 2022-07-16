#pragma once
#ifndef _COLOR_TO_UNICODE_H
#define _COLOR_TO_UNICODE_H

#include <vector>
#include <stdint.h>
#include "gifmeta.h"

// U+0D9E ඞ
// U+0DAC ඬ
// U+0DB0 ධ
// U+0E8F ຏ
// U+0EA5 ລ

char* BrightnessToUnicode(double avgBrightness);
char* ColorToUnicode(const Color& color);

#endif // _COLOR_TO_UNICODE_H

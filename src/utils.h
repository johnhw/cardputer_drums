#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>

#include <M5Cardputer.h>

#define RGB565(r, g, b) (((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F))

void* allocBuffer(void* buffer, int16_t len, int32_t size);

int16_t getDigitPressed(Keyboard_Class::KeysState status);
float iirAlpha(int sr, float freq);
float halfLifeTime(int sr, float t);

#endif
#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
#include "config.h"
#include "datatypes.h"
#include <M5Cardputer.h>



#define RGB565(r, g, b) (((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F))

void* allocBuffer(void* buffer, int16_t len, int32_t size);

int16_t getDigitPressed(Keyboard_Class::KeysState status);
int16_t getAlphanumericPressed(Keyboard_Class::KeysState status);   
float iirAlpha(float freq);
float halfLifeTime(int sr, float t);
int decodeString(String f);
String encodeString(int f);
int8_t getKeyIndex(const char *pattern);
void lowerMessage(const char *message);
void createArena(arena_t *arena);
void *allocArena(arena_t *arena, int32_t size);
void clearArena(arena_t *arena);
void *getArenaTop(arena_t *arena);
void setArenaTop(arena_t *arena, void *top);
int32_t getArenaFree(arena_t *arena);
float cBGain(float cb);
void drawModifierKeys(int32_t backgroundColor);

#endif
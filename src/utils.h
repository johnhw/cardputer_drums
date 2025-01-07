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
float iirAlpha(int sr, float freq);
float halfLifeTime(int sr, float t);
int decodeString(String f);
String encodeString(int f);
int8_t getKeyIndex(const char *pattern);
void lowerMessage(const char *message);

typedef struct arena_t {
    void* start;
    void* end;    
    void *top;
} arena_t;


void *allocArena(arena_t *arena, int64_t *size);
void clearArena(arena_t *arena);
int64_t getArenaFree(arena_t *arena);


#endif
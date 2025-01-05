#include "utils.h"
#include <stdlib.h>
#include <string.h>


arena_t *initInitArena(int64_t maxSize)
{
    arena_t *arena = (arena_t *)malloc(sizeof(arena_t));
    arena->start = malloc(maxSize);
    arena->top = arena->start;
    arena->end = arena->start + maxSize;
    return arena;
}

void clearArena(arena_t *arena)
{
    arena->top = arena->start;
}

int64_t getArenaFree(arena_t *arena)
{
    return (byte *)(arena->end) - (byte *)arena->top;
}

void *allocArena(arena_t *arena, int64_t *size)
{
    void *ptr = arena->top;
    if(arena->top + *size > arena->end)
    {
        *size = 0;
        return nullptr;
    }
    arena->top += *size;    
}

void* allocBuffer(void* buffer, int16_t len, int32_t size) {
    if (buffer != nullptr)
        free(buffer);
    buffer = malloc(len * size);
    if (buffer)
        memset(buffer, 0, len * size);
    return buffer;
}

/* Take a single character string 0-9a-z and return the integer value */
/* Ignore case. Return -1 for invalid characters */
int decodeString(String f) {
    if (f.length() != 1)
        return -1;
    char c = f[0];
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;
    return -1;
}

/* Take an integer value 0-35 and return the single character string */
String encodeString(int f) {
    if (f < 10)
        return String(f);
    if (f < 36)
        return String((char)('a' + f - 10));
    return String("?");
}

int16_t getDigitPressed(Keyboard_Class::KeysState status)
{
   if (M5Cardputer.Keyboard.isKeyPressed('0'))
    return 0;
  if (M5Cardputer.Keyboard.isKeyPressed('1'))
    return 1;
  if (M5Cardputer.Keyboard.isKeyPressed('2'))
    return 2;
  if (M5Cardputer.Keyboard.isKeyPressed('3')) 
    return 3;
  if (M5Cardputer.Keyboard.isKeyPressed('4')) 
    return 4;
  if (M5Cardputer.Keyboard.isKeyPressed('5'))
    return 5;
  if (M5Cardputer.Keyboard.isKeyPressed('6'))
    return 6;
  if (M5Cardputer.Keyboard.isKeyPressed('7')) 
    return 7;
  if (M5Cardputer.Keyboard.isKeyPressed('8')) 
    return 8;
  if (M5Cardputer.Keyboard.isKeyPressed('9')) 
    return 9;
  return -1;

}

float iirAlpha(int sr, float freq)
{  
  float wc = 2 * M_PI * freq / sr;
  return exp(-wc);
}

float halfLifeTime(int sr, float t)
{
  
  return exp(-log(2.0) / ((float)t * (float)sr + 0.0f));
}



// check each character in string if pressed
// return the index of the key pressed in the string
// or -1 if no match 
int8_t getKeyIndex(String pattern)
{
  int i;
  for(i=0;i<pattern.length();i++)
  {
    if(M5Cardputer.Keyboard.isKeyPressed(pattern[i]))
      return i;
  }
  return -1;
}

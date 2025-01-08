#include "utils.h"
#include <stdlib.h>
#include <string.h>

char arenaBuffer[SAMPLE_ARENA_SIZE]; // the actual global arena buffer

void createArena(arena_t *arena)
{
    arena->start = arenaBuffer; 
    arena->top = arena->start;
    arena->end = arena->start + SAMPLE_ARENA_SIZE;    
}

void clearArena(arena_t *arena)
{
    arena->top = arena->start;
}

int32_t getArenaFree(arena_t *arena)
{
    return (int32_t)((byte *)(arena->end) - (byte *)arena->top);
}

void *allocArena(arena_t *arena, int32_t size)
{
    void *ptr = arena->top;
    if(arena->top + size > arena->end)
    {        
        return nullptr;
    }
    arena->top += size;    
    return ptr;
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
  return getKeyIndex(digitChars);
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

  void lowerMessage(const char *message)
  {
    int statusHeight = 16;
    M5Cardputer.Display.setTextColor(BLACK);
    M5Cardputer.Display.setFont(&fonts::Font2);
    M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - statusHeight - 4, M5Cardputer.Display.width(), statusHeight + 4, TFT_GREEN);
    
    M5Cardputer.Display.drawString(message, 10, M5Cardputer.Display.height() - statusHeight - 2);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setFont(&fonts::Font2);
  }


// check each character in string if pressed
// return the index of the key pressed in the string
// or -1 if no match 
int8_t getKeyIndex(const char *pattern)
{
  int i;
  for(i=0;i<strlen(pattern);i++)
  {
    if(M5Cardputer.Keyboard.isKeyPressed(pattern[i]))
      return i;
  }
  return -1;
}

int16_t getAlphanumericPressed(Keyboard_Class::KeysState status)
{
  return getKeyIndex(alphaNumericChars);
}
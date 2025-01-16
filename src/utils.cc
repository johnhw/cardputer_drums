#include "utils.h"
#include <stdlib.h>
#include <string.h>

// Note: this is statically allocated as dynamic allocation leads to
// fragmentation and a much smaller total possible sample buffer
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
  memset(arena->start, 0, SAMPLE_ARENA_SIZE);
}

int32_t getArenaFree(arena_t *arena)
{
  return (int32_t)((byte *)(arena->end) - (byte *)arena->top);
}

void *allocArena(arena_t *arena, int32_t size)
{
  void *ptr = arena->top;
  if (arena->top + size >= arena->end)
  {
    return nullptr;
  }
  arena->top += size;
  return ptr;
}

void *allocBuffer(void *buffer, int16_t len, int32_t size)
{
  if (buffer != nullptr)
    free(buffer);
  buffer = malloc(len * size);
  if (buffer)
    memset(buffer, 0, len * size);
  return buffer;
}

/* Take a single character string 0-9a-z and return the integer value */
/* Ignore case. Return -1 for invalid characters */
int decodeString(String f)
{
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
String encodeString(int f)
{
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

int16_t getShiftDigitPressed(Keyboard_Class::KeysState status)
{
  return getKeyIndex(shiftDigitChars);
}

float iirAlpha(float normFreq)
{
  float wc = 2 * M_PI * normFreq;
  float alpha = exp(-wc);
  return alpha;
}

void *getArenaTop(arena_t *arena)
{
  return arena->top;
}

void setArenaTop(arena_t *arena, void *top)
{
  arena->top = top;
  if (arena->top > arena->end)
    arena->top = arena->end;
}

float halfLifeTime(int sr, float t)
{

  return exp(-log(2.0) / ((float)t * (float)sr + 0.0f));
}

// Convert centibel to a gain
float cBGain(float cb)
{
  return powf(10.0f, cb / 200.0f);
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

// using keyboardSemitoneSequence
// in theory we could microtune this
int32_t getKeyboardPianoCents()
{
  int32_t index = getKeyIndex(shiftKeyboardSemitoneSequence); // ctrl reads as shift?
  if (index == -1)
    index = getKeyIndex(keyboardSemitoneSequence);
  if (index == -1)
    return -1;
  return index * 100;
}

// buf must have at least 4 characters!
void centsToNoteName(int32_t cents, char *buf)
{
  int32_t note = (cents + 50 + 4800) / 100; // truncate to semitone
  int32_t octave = note / 12;
  int32_t noteInOctave = note % 12;
  // make sure remainder is positive
  if (noteInOctave < 0)
    noteInOctave += 12;
  const char *name = noteNames[noteInOctave];
  // limit octave to 0-9
  if (octave < 0)
    octave = 0;
  if (octave > 9)
    octave = 9;
  snprintf(buf, 4, "%s%d", name, octave);
}

// show which modifiers are pressed
void drawModifierKeys(int32_t backgroundColor)
{
  int32_t statusHeight = 12;
  int32_t statusWidth = 49;
  Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
  M5Cardputer.Display.fillRect(M5Cardputer.Display.width() - statusWidth, 0, M5Cardputer.Display.width(), statusHeight, backgroundColor);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setFont(&fonts::Font0);
  int32_t x = M5Cardputer.Display.width() - statusWidth;
  int32_t step = 9;
  int32_t y = 3;
  if (status.fn)
  {
    M5Cardputer.Display.drawString("F", x, y);
  }
  x += step;
  if (status.alt)
  {
    M5Cardputer.Display.drawString("A", x, y);
  }
  x += step;
  if (status.opt)
  {
    M5Cardputer.Display.drawString("O", x, y);
  }
  x += step;
  if (status.shift)
  {
    M5Cardputer.Display.drawString("^", x, y);
  }
  x += step;
  if (status.ctrl)
  {
    M5Cardputer.Display.drawString("C", x, y);
  }
}

// check each character in string if pressed
// return the index of the key pressed in the string
// or -1 if no match
int8_t getKeyIndex(const char *pattern)
{
  int i;
  for (i = 0; i < strlen(pattern); i++)
  {
    if (M5Cardputer.Keyboard.isKeyPressed(pattern[i]))
      return i;
  }
  return -1;
}

int16_t getAlphanumericPressed(Keyboard_Class::KeysState status)
{
  return getKeyIndex(alphaNumericChars);
}
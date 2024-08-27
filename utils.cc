#include "utils.h"
#include <stdlib.h>
#include <string.h>

void* allocBuffer(void* buffer, int16_t len, int32_t size) {
    if (buffer != nullptr)
        free(buffer);
    buffer = malloc(len * size);
    if (buffer)
        memset(buffer, 0, len * size);
    return buffer;
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

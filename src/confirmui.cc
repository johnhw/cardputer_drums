#include "confirmui.h"

void confirmModeKeys(DrumMachine &dm) {
    if (M5Cardputer.Keyboard.isChange())
  {
    if (M5Cardputer.Keyboard.isPressed())
    {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
       if (status.word.size() > 0)
        {
          if (isalpha(status.word[0]))
            if(status.word[0] == 'y' || status.word[0] == 'Y')
            {
              confirmModeUpdate(dm);
            }
            else
            {
                setPlayMode(dm, dm.lastMode);
            }
        }
    }
  }
}

void confirmModeUpdate(DrumMachine &dm)
{
    lowerMessage(dm, "Confirm [Y]?");
}

void setGraphicsModeConfirm()
{
 
 
}
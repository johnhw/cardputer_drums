#include "fileui.h"
#include "datatypes.h"
#include "flash.h"
#include "ui.h"


static fileui_t fileui;

void fileModeUpdate(DrumMachine &dm)
{
    fileModeKeys(dm);
}

void updateFileText(DrumMachine &dm)
{    
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setTextColor(WHITE);  
    // write the current file name
    M5Cardputer.Display.drawString(dm.fileName, 12, 12);
    int y = 24;
    M5Cardputer.Display.setTextColor(GREEN);  
    for (size_t i = 0; i < fileui.files.size(); i++)
    {
        //M5Cardputer.Display.drawString(filteredFiles[i], 12, y);
        y += 12;
    }
}

void fileModeKeys(DrumMachine &dm)
{

    if (M5Cardputer.Keyboard.isChange())
    {
        if (M5Cardputer.Keyboard.isPressed())
        {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            
            // enter
            if(M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER))
            {
                setPlayMode(dm, dm.lastMode);
            }   
            // backspace
            if(M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE))
            {
                if(dm.fileName.length() > 0)
                    dm.fileName = dm.fileName.substring(0, dm.fileName.length() - 1);
            }           
            // alphanumeric
            if (status.word.size() > 0)
            {
                if (isalpha(status.word[0]) || isdigit(status.word[0]))
                {
                    dm.fileName += status.word[0];
                }
            }            
            updateFileText(dm);
        }
    }
}

void initFileUI()
{
    fileui.cursorPos = 0;
    fileui.fileName = "";
    fileui.files = listFiles(basePathPattern);
}

void setGraphicsModeFile(DrumMachine &dm)
{
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setFont(&fonts::Font0);
  M5Cardputer.Display.setTextColor(GREEN);
  initFileUI();
  updateFileText(dm);
}


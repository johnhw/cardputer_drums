#include <M5Cardputer.h>
#include "previewui.h"
#include "audio.h"
#include "utils.h"
#include "ui.h"
#include "pattern.h"

void setGraphicsModePreview()
{
  M5Cardputer.Display.clearDisplay(TFT_DARKGREY);
  M5Cardputer.Display.setFont(&fonts::FreeMonoBold24pt7b);
  M5Cardputer.Display.setTextDatum(top_center);
  M5Cardputer.Display.setTextColor(WHITE);
}

// in preview mode, just play samples immediately
void previewModeUpdate(DrumMachine &dm)
{
  if(M5Cardputer.BtnA.wasClicked())
  {
    M5Cardputer.update();
     setPlayMode(dm, 0);
  }

  if (M5Cardputer.Keyboard.isChange())
  {
    if (M5Cardputer.Keyboard.isPressed())
    {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
      if (status.fn)
      {
        // pass
      }
      else
      {
        if (status.word.size() > 0)
        {
          if (isalpha(status.word[0]))
            previewSample(dm, status.word[0]);
        }

         if(M5Cardputer.Keyboard.isKeyPressed(' '))
        {
          resetDetuneSample(dm, dm.previewData.lastSample);
        }
        
        // detune adjust
        if(M5Cardputer.Keyboard.isKeyPressed(';'))
        {
          detuneSample(dm, dm.previewData.lastSample, 100);
        }
        if(M5Cardputer.Keyboard.isKeyPressed('.'))
        {
          detuneSample(dm, dm.previewData.lastSample, -100);
        }        
        if(M5Cardputer.Keyboard.isKeyPressed(','))
        {
          detuneSample(dm, dm.previewData.lastSample, 5);
        }
        if(M5Cardputer.Keyboard.isKeyPressed('/'))
        {
          detuneSample(dm, dm.previewData.lastSample, -5);
        }
        if(M5Cardputer.Keyboard.isKeyPressed('<'))
        {
          detuneSample(dm, dm.previewData.lastSample, 1);
        }
        if(M5Cardputer.Keyboard.isKeyPressed('?'))
        {
          detuneSample(dm, dm.previewData.lastSample, -1);
        }

      }
    }
  }
}

void detuneSample(DrumMachine &dm, char sample, int32_t detune)
{
  sample_t *samplePtr;
  samplePtr = getSample(dm, sample);
  if (samplePtr)
  {
    samplePtr->detune += detune;
    previewSample(dm, sample);
  }

}

void resetDetuneSample(DrumMachine &dm, char sample)
{
  sample_t *samplePtr;
  samplePtr = getSample(dm, sample);
  if (samplePtr)
  {
    samplePtr->detune = 0;
    previewSample(dm, sample);
  } 
  
}

void previewSample(DrumMachine &dm, char sample)
{
  char *preview = "X";
  sample_t *samplePtr;
  M5Cardputer.Display.clearDisplay(TFT_DARKGREY);
  preview[0] = sample;
  dm.previewData.lastSample = sample;
  M5Cardputer.Display.setFont(&fonts::FreeMonoBold24pt7b);
  M5Cardputer.Display.drawString(preview, M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() / 2-50);

  // show the detune
  String detuneMsg = "Det: " + String(getSample(dm, sample)->detune);  
  M5Cardputer.Display.setFont(&fonts::FreeMonoBold12pt7b);
  M5Cardputer.Display.drawString(detuneMsg, M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() / 2);
  
  

  samplePtr = getSample(dm, sample);
  if (samplePtr)
    previewSample(dm, samplePtr);
}

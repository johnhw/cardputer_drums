#include <M5Cardputer.h>
#include "previewui.h"
#include "audio.h"
#include "utils.h"
#include "ui.h"
#include "pattern.h"

enum SCALES
{
  SCALE_NORMAL,
  SCALE_FINE,
  SCALE_COARSE,
  SCALE_EXTRA_FINE,
  SCALE_EXTRA_COARSE
};

void initPreviewMode(DrumMachine &dm)
{
  dm.previewData.previewDirty = true;
  dm.previewData.lastSample = 'z';
  setGraphicsModePreview();
}

void setGraphicsModePreview()
{
  M5Cardputer.Display.clearDisplay(TFT_DARKGREY);
  M5Cardputer.Display.setFont(&fonts::FreeMonoBold24pt7b);
  M5Cardputer.Display.setTextDatum(top_center);
  M5Cardputer.Display.setTextColor(WHITE);
 
}

void previewFnKey(DrumMachine &dm, Keyboard_Class::KeysState status)
{
  if (M5Cardputer.Keyboard.isKeyPressed('o'))
  {
    dm.bankAction = ACTION_LOAD;
    lowerMessage("Load kit:");
  }
  if (M5Cardputer.Keyboard.isKeyPressed('s'))
  {
    dm.bankAction = ACTION_SAVE;
    lowerMessage("Save kit:");
  }
}

void resetKitParam(DrumMachine &dm)
{
  int param = dm.previewData.currentParam;
  setKitParamValue(dm, param, dm.previewData.lastSample, 0);
}

void adjustKitParam(DrumMachine &dm, int scaleMode, int adj)
{
  int param = dm.previewData.currentParam;
  int32_t scale;
  switch (scaleMode)
  {
  case SCALE_NORMAL:
    scale = 10;
    break;
  case SCALE_FINE:
    scale = 1;
    break;
  case SCALE_COARSE:
    scale = 100;
    break;
  case SCALE_EXTRA_FINE:
    scale = 1;
    break;
  case SCALE_EXTRA_COARSE:
    scale = 1000;
    break;
  }

  if (param == PARAM_DETUNE)
  {
    if (scale == 1000)
      scale = 1200; // fix to octave for this case
  }

  if(param == PARAM_TRIM_END || param == PARAM_LOOP_END)
  {
    adj = -adj; // reverse the sign for end points
  }

  int32_t value;
  getKitParamValue(dm, param, dm.previewData.lastSample, value);
  setKitParamValue(dm, param, dm.previewData.lastSample, value + adj * scale);
  previewSample(dm, dm.previewData.lastSample);
}

void nextKitParam(DrumMachine &dm)
{
  dm.previewData.currentParam++;
  if (dm.previewData.currentParam >= PARAM_N)
  {
    dm.previewData.currentParam = 0;
  }
}

void prevKitParam(DrumMachine &dm)
{
  dm.previewData.currentParam--;
  if (dm.previewData.currentParam < 0)
  {
    dm.previewData.currentParam = PARAM_N - 1;
  }
}

// in preview mode, just play samples immediately
void previewModeUpdate(DrumMachine &dm)
{
    if (M5Cardputer.BtnA.wasClicked())
    {
      M5Cardputer.update();
      setPlayMode(dm, 0);
    }

  if (M5Cardputer.Keyboard.isChange())
  {
  

    // check for actions (load/save kit)
    if (dm.bankAction != ACTION_NONE)
    {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
      int16_t bank = getAlphanumericPressed(status);
      if (bank >= 0)
      {
        if (dm.bankAction == ACTION_LOAD)
        {
          // loadCustomKit(dm, bank);
        }
        if (dm.bankAction == ACTION_SAVE)
        {
          // saveCustomKit(dm, bank);
        }
      }
    }

    if (M5Cardputer.Keyboard.isPressed())
    {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

      if (status.fn)
      {
        // function keys (load/save etc.)
        previewFnKey(dm, status);
      }
      else
      {
        if (status.word.size() > 0)
        {
          if (isalpha(status.word[0]))
          {                        
            previewSample(dm, status.word[0]);
          }
        }

        if (M5Cardputer.Keyboard.isKeyPressed(' '))
        {
          
          resetKitParam(dm);
        }

        // check for scale mode
        int16_t scaleMode = SCALE_NORMAL;

        // check if alt down = fine
        if (status.alt && !status.opt)
        {
          scaleMode = SCALE_FINE;
        }

        // opt = coarse
        if (status.opt && !status.alt)
        {
          scaleMode = SCALE_COARSE;
        }

        // opt+alt = extra coarse
        if (status.alt && status.opt)
        {
          scaleMode = SCALE_EXTRA_COARSE;
        }

        // Param adjustment
        if (M5Cardputer.Keyboard.isKeyPressed(';'))
        {
          adjustKitParam(dm, scaleMode, 1);
        }
        if (M5Cardputer.Keyboard.isKeyPressed('.'))
        {
          adjustKitParam(dm, scaleMode, -1);
        }
        if (M5Cardputer.Keyboard.isKeyPressed(','))
        {
          prevKitParam(dm);
          dm.previewData.previewDirty = true; 
          redrawPreview(dm);
        }
        if (M5Cardputer.Keyboard.isKeyPressed('/'))
        {
          nextKitParam(dm);
          dm.previewData.previewDirty = true; 
          redrawPreview(dm);
        }
      }
    }
  }
}

void getKitParamValue(DrumMachine &dm, int32_t param, char sample, int32_t &value)
{
  sample_t *samplePtr;
  samplePtr = getSample(dm, sample);

  if (!samplePtr)
  {
    value = -1;
  }

  switch (param)
  {
  case PARAM_DETUNE:
    value = samplePtr->adjustments.detune;
    break;
  case PARAM_VOLUME:
    value = samplePtr->adjustments.volume;
    break;
  case PARAM_CUTOFF:
    value = samplePtr->adjustments.cutoff;
    break;
  case PARAM_TRIM_START:
    value = samplePtr->adjustments.trimStart;
    break;
  case PARAM_TRIM_END:
    value = samplePtr->adjustments.trimEnd;
    break;
  case PARAM_LOOP_START:
    value = samplePtr->adjustments.loopStart;
    break;
  case PARAM_LOOP_END:
    value = samplePtr->adjustments.loopEnd;
    break;
  case PARAM_ATTACK_TIME:
    value = samplePtr->adjustments.attackTime;
    break;
  case PARAM_DECAY_TIME:
    value = samplePtr->adjustments.decayTime;
    break;
  case PARAM_SUSTAIN_LEVEL:
    value = samplePtr->adjustments.sustainLevel;
    break;
  case PARAM_RELEASE_TIME:
    value = samplePtr->adjustments.releaseTime;
    break;
  }
}

void setKitParamValue(DrumMachine &dm, int32_t param, char sample, int32_t value)
{
  sample_t *samplePtr;
  samplePtr = getSample(dm, sample);

  // enforce param limits
  if(value < paramLimits[param][0])
    value = paramLimits[param][0];
  if(value > paramLimits[param][1])
    value = paramLimits[param][1];

  if (!samplePtr)
  {
    return;
  }

  switch (param)
  {
  case PARAM_DETUNE:
    samplePtr->adjustments.detune = value;
    break;
  case PARAM_VOLUME:
    samplePtr->adjustments.volume = value;
    break;
  case PARAM_CUTOFF:
    samplePtr->adjustments.cutoff = value;
    break;
  case PARAM_TRIM_START:
    samplePtr->adjustments.trimStart = value;
    break;
  case PARAM_TRIM_END:
    samplePtr->adjustments.trimEnd = value;
    break;
  case PARAM_LOOP_START:
    samplePtr->adjustments.loopStart = value;
    break;
  case PARAM_LOOP_END:
    samplePtr->adjustments.loopEnd = value;
    break;
  case PARAM_ATTACK_TIME:
    samplePtr->adjustments.attackTime = value;
    break;
  case PARAM_DECAY_TIME:
    samplePtr->adjustments.decayTime = value;
    break;
  case PARAM_SUSTAIN_LEVEL:
    samplePtr->adjustments.sustainLevel = value;
    break;
  case PARAM_RELEASE_TIME:
    samplePtr->adjustments.releaseTime = value;
    break;
  }
  dm.previewData.previewDirty = true; // display needs to update
}

void playPreviewSample(DrumMachine &dm, char sample)
{
  sample_t *samplePtr;
  if(sample!=dm.previewData.lastSample)
    dm.previewData.previewDirty = true;

  dm.previewData.lastSample = sample;
  samplePtr = getSample(dm, sample);
  if (samplePtr)
    previewSample(dm, samplePtr);
}

// render a waveform to the screen
// draw the waveform with steps proportional to the width of the screen
// show red lines at the start and end of the trim region (remember end counts back from then end!)
void renderWaveform(sample_t *sample, int32_t baseY)
{    
  int32_t width = M5Cardputer.Display.width() * 0.8;
  int32_t height = 26;
  int32_t len = sample->len;
  int32_t trimStart = sample->adjustments.trimStart;
  int32_t trimEnd = sample->adjustments.trimEnd;
  int32_t actualEnd = len - trimEnd;
  int32_t x, y;
  int32_t lastX = 0;
  int32_t lastY = 0;
  int32_t i;
  int32_t step = len / width;
  int32_t baseX = (M5Cardputer.Display.width() - width) / 2;
  // black box, screen wide
  M5Cardputer.Display.fillRect(0, baseY - height, M5Cardputer.Display.width(), 2 * height, TFT_BLACK);

  // draw baseline
  M5Cardputer.Display.drawLine(baseX, baseY, baseX + width, baseY, TFT_GREEN);

  // draw the waveform
  for(i=0;i<width;i++)
  {
    x = i + baseX;
    y = baseY - (sample->samples[i*step] * height / 32768);
    if(i>0)
    {
      M5Cardputer.Display.drawLine(lastX, lastY, x, y, TFT_WHITE);
    }
    lastX = x;
    lastY = y;
  }

  // compute actual pixel start and end positions
  int32_t start = baseX + trimStart * width / len;
  int32_t end = baseX + actualEnd * width / len;
  M5Cardputer.Display.drawLine(start, baseY - height, start, baseY + height, TFT_RED);
  M5Cardputer.Display.drawLine(end, baseY - height, end, baseY + height, TFT_RED);

  bool isLoop = sample->adjustments.loopEnd!=0 && sample->adjustments.loopStart!=0;
  if(isLoop)
  {
    int32_t loopStart = baseX + sample->adjustments.loopStart * width / len;
    int32_t loopEnd = baseX + (len - sample->adjustments.loopEnd) * width / len;
    M5Cardputer.Display.drawLine(loopStart, baseY - height, loopStart, baseY + height, TFT_CYAN);
    M5Cardputer.Display.drawLine(loopEnd, baseY - height, loopEnd, baseY + height, TFT_CYAN);
  }
}

void redrawPreview(DrumMachine &dm)
{
  // skip if nothing has changed
  if(!dm.previewData.previewDirty)
    return;
  char *preview = "X";
  char sample = dm.previewData.lastSample;
  preview[0] = sample;
  M5Cardputer.Display.clearDisplay(TFT_DARKGREY);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setFont(&fonts::FreeMonoBold24pt7b);
  M5Cardputer.Display.setTextColor(BLACK);
  M5Cardputer.Display.drawString(preview, M5Cardputer.Display.width() / 2 + 1, M5Cardputer.Display.height() / 2 - 60 + 1);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.drawString(preview, M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() / 2 - 60);

  String currentParam, nextParam, prevParam;
  int32_t prevIndex, currentIndex, nextIndex;
  currentIndex = dm.previewData.currentParam;
  prevIndex = (dm.previewData.currentParam + PARAM_N - 1) % PARAM_N;
  nextIndex = (dm.previewData.currentParam + 1) % PARAM_N;

  currentParam = paramNames[currentIndex];
  nextParam = paramNames[nextIndex];
  prevParam = paramNames[prevIndex];
  
  
  // create the message (prevParamName:currentParamName:currentParamValue nextParamName:)
  char msg[256];
  int32_t current;

  getKitParamValue(dm, currentIndex, sample, current);
  
  M5Cardputer.Display.setFont(&fonts::Font2);
  M5Cardputer.Display.setTextColor(TFT_BLACK);
  snprintf(msg, 255, "%7s %7s %05d %7s", prevParam.c_str() ,  currentParam.c_str(),current, nextParam.c_str());
  M5Cardputer.Display.drawString(msg, M5Cardputer.Display.width() / 2+1-13, M5Cardputer.Display.height() / 2+1-13);
  M5Cardputer.Display.setTextColor(TFT_WHITE);
  M5Cardputer.Display.drawString(msg, M5Cardputer.Display.width() / 2-13, M5Cardputer.Display.height() / 2-13);
  
  sample_t *samplePtr = getSample(dm, sample);
  if (samplePtr)
  {
    renderWaveform(samplePtr, 110);
  }

  dm.previewData.previewDirty = false;
}

void previewSample(DrumMachine &dm, char sample)
{
  if(sample < 'a' || sample>'y')
    return;
  playPreviewSample(dm, sample);
  redrawPreview(dm);
}


#include "patternui.h"
#include "audio.h"
#include "utils.h"
#include "channels.h"
#include "ui.h"
#include "pattern.h"

void getCursorPixelPos(DrumMachine& dm, int16_t step, int16_t chan, int16_t& x, int16_t& y) {
    x = (step + (step / 4)) * 12 + 12;
    y = chan * 12 + 12;
}


void moveCursor(DrumMachine& dm, int x, int y)
{
    drawCursor(dm, 0);
    dm.cursor.step += x;
    dm.cursor.chan += y;

    // Boundaries checking
    if (dm.cursor.step < 0)
        dm.cursor.step = 0;
    if (dm.cursor.chan < 0)
        dm.cursor.chan = 0;
    if (dm.cursor.step >= nSteps)
        dm.cursor.step = nSteps - 1;
    if (dm.cursor.chan >= nChans)
        dm.cursor.chan = nChans - 1;

    dm.cursor.dirty = 1;
}


void adjSwing(DrumMachine& dm, int adj)
{
  dm.swing += adj;
  if (dm.swing < 0)
    dm.swing = 0;
  if (dm.swing > 99)
    dm.swing = 99;
  requestMix(dm);
  drawStatus(dm);
}

void adjBpm(DrumMachine& dm, int adj)
{
  dm.bpm += adj;
  if (dm.bpm < minBPM)
    dm.bpm = minBPM;
  if (dm.bpm > 320)
    dm.bpm = 320;
  recalcBPM(dm);
  mix(dm);
  drawStatus(dm);
}

// set volume
void adjVolume(DrumMachine& dm, int adj)
{
  dm.volume += adj;
  if (dm.volume < 0)
    dm.volume = 0;
  if (dm.volume > 16)
    dm.volume = 16;
  int16_t vol = dm.volume * 16;
  if(vol>255) vol = 255;
  M5Cardputer.Speaker.setVolume(vol);
  drawStatus(dm);
}

// increment/decrement the kickDelay on the current
// cursor element
void adjCursorKick(DrumMachine& dm, int adj)
{
  int index = dm.cursor.step + nSteps * dm.cursor.chan;
  int kick = dm.currentPattern[index].kickDelay + adj;
  if (kick < 0)
    kick = 0;
  if (kick >= kickSubdiv)
    kick = kickSubdiv - 1;
  dm.currentPattern[index].kickDelay = kick;
  requestMix(dm);
}




void updateCursor(DrumMachine &dm)
{
  dm.cursor.flash += 1;
  // draw/flash the cursor
  if (dm.cursor.flash >= cursorFlashTime)
  {
    dm.cursor.flash = 0;
    dm.cursor.on = !dm.cursor.on;
    dm.cursor.dirty = 1;
  }
  if(dm.cursor.dirty)
  {
    drawCursor(dm, 0); // draw the NEXT character to avoid overlap when kicking forward
    // flash current cursor
    if (dm.cursor.on)
        drawCursor(dm, 2);
    else
        drawCursor(dm, 1);
    dm.cursor.dirty = 0;    
  }
}

void patternModeKeys(DrumMachine &dm)
{

  if(M5Cardputer.BtnA.wasClicked())
  {
     M5Cardputer.update();
     setPlayMode(dm, 1);
  }
  if (M5Cardputer.Keyboard.isChange())
  {
    if (M5Cardputer.Keyboard.isPressed())
    {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

      if (status.opt)
      {
        channelKey(dm, status);
      }
      else
      {
        // function mode
        if (status.fn)
        {
          fnKey(dm, status);
        }
        else
        {
          // set characters for sample entry
          if (status.del)
          {            
            setCursorChar(dm, 0); // clear
          }
          if (status.word.size() > 0)
          {
            if (isalpha(status.word[0]))
              setCursorChar(dm, status.word[0]);
            if (isdigit(status.word[0]))
              setCursorVel(dm, status.word[0]);
          }
          // cursor
          if (M5Cardputer.Keyboard.isKeyPressed(','))
            moveCursor(dm, -1, 0);
          if (M5Cardputer.Keyboard.isKeyPressed('/'))
            moveCursor(dm, 1, 0);
          if (M5Cardputer.Keyboard.isKeyPressed(';'))
            moveCursor(dm, 0, -1);
          if (M5Cardputer.Keyboard.isKeyPressed('.'))
            moveCursor(dm, 0, 1);
          if (M5Cardputer.Keyboard.isKeyPressed(']'))
            adjCursorKick(dm, 1);
          if (M5Cardputer.Keyboard.isKeyPressed('['))
            adjCursorKick(dm, -1);
        }
      }
    }
  }
}

void patternCursorMove(DrumMachine &dm, int step)
{

}

void patternInsertCurrent(DrumMachine &dm)
{


}

void patternDeleteCurrent(DrumMachine &dm)
{


}

void patternSwitchMode(DrumMachine &dm)
{

}

void channelKey(DrumMachine& dm, Keyboard_Class::KeysState status)
{
    int16_t digit = getDigitPressed(status) - 1;
    if(digit>=0 && digit<nChans)
      {
        if(status.alt)
          toggleSolo(dm, digit);          
        else
          toggleMute(dm, digit);          
      }          

  if (M5Cardputer.Keyboard.isKeyPressed(','))
    patternCursorMove(dm, -1);
  if (M5Cardputer.Keyboard.isKeyPressed('/'))
    patternCursorMove(dm, 1);
  if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER))
      patternInsertCurrent(dm);
  if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE))
      patternDeleteCurrent(dm);
  if (M5Cardputer.Keyboard.isKeyPressed('`'))
      patternSwitchMode(dm);
  
}

void fnKey(DrumMachine& dm, Keyboard_Class::KeysState status)
{
  // bpm and swing
  
  if (M5Cardputer.Keyboard.isKeyPressed(','))
    adjSwing(dm, -1);
  if (M5Cardputer.Keyboard.isKeyPressed('/'))
    adjSwing(dm, 1);
  if (M5Cardputer.Keyboard.isKeyPressed(';'))
    adjBpm(dm, 1);
  if (M5Cardputer.Keyboard.isKeyPressed('.'))
    adjBpm(dm, -1);
  if (M5Cardputer.Keyboard.isKeyPressed('['))
    adjVolume(dm, -1);
  if (M5Cardputer.Keyboard.isKeyPressed(']'))
    adjVolume(dm, 1);
  

  if (M5Cardputer.Keyboard.isKeyPressed('c'))
    copyPattern(dm);
  if (M5Cardputer.Keyboard.isKeyPressed('v'))
    pastePattern(dm);
  if (M5Cardputer.Keyboard.isKeyPressed('x'))
  {
    copyPattern(dm);
    selectiveClearPattern(dm);
    updatePattern(dm);
  }
  if (M5Cardputer.Keyboard.isKeyPressed('d'))
  {
    selectiveClearPattern(dm);
    updatePattern(dm);
  }
 

  int16_t digit = getDigitPressed(status);
  if(digit>=0)
  {
    if(status.alt)
      setPattern(dm, digit+10);
    else
      setPattern(dm, digit);
  }
  
}

void renderCursor(DrumMachine& dm, int state)
{
    char s[2] = "X";
    int16_t drawX, drawY, kickDelay, kickWidth;

    // Fetch cursor position and properties
    getCursorPixelPos(dm, dm.cursor.step, dm.cursor.chan, drawX, drawY);
    kickDelay = getKickDelay(dm, dm.cursor.step, dm.cursor.chan);
    kickWidth = 12 * kickDelay / kickSubdiv;
    drawY += dm.cursor.height / 2;
    drawX += kickWidth; // shift left for delayed onset

    // Fetch cursor character and velocity
    char ch;
    int16_t vel;
    getCursorChar(dm, ch, vel);
    s[0] = ch;

    int16_t charColor = RGB565(0, vel * 3 - ((dm.cursor.step % 2) ? 0 : 2), 0);
    int boxOffX = -4;
    int boxOffY = -2;
    int boxX = drawX + boxOffX;
    int boxY = drawY + boxOffY;

    // Render cursor based on state
    if (state == 0) // no flash
    {
        M5Cardputer.Display.fillRect(boxX - kickWidth, boxY, dm.cursor.width + kickWidth, dm.cursor.height, charColor);        
        M5Cardputer.Display.drawString(s, drawX, drawY - dm.cursor.height / 2);
        
    }
    else if (state == 1) // flash on
    {
        M5Cardputer.Display.fillRect(boxX, boxY, dm.cursor.width, dm.cursor.height, TFT_WHITE);
    }
    else if (state == 2) // flash off
    {
        M5Cardputer.Display.fillRect(boxX - kickWidth, boxY, dm.cursor.width + kickWidth, dm.cursor.height, charColor);
        M5Cardputer.Display.drawRect(boxX, boxY, dm.cursor.width, dm.cursor.height, TFT_WHITE);
        M5Cardputer.Display.drawString(s, drawX, drawY - dm.cursor.height / 2);
    }
}

void drawCursor(DrumMachine& dm, int state)
{
    
    // "underdraw" next step so that overlaps when kicking forward render correctly
    if (dm.cursor.step < nSteps - 1)
    {
        dm.cursor.step++;
        renderCursor(dm, 0);
        dm.cursor.step--;
    }

    renderCursor(dm, state);
}

const int16_t statusColor = RGB565(5, 5, 5);

void drawTopLine(DrumMachine &dm)
{
    int topHeight = 12; 
    char statusLine[256];
    const char *patternNames = "0123456789ABCDEFGHIJ";

    // Clear the status bar area
    M5Cardputer.Display.fillRect(0, 0, M5Cardputer.Display.width(), topHeight, TFT_BLACK);    
    snprintf(statusLine, 255, "%c", patternNames[dm.pattern]);
    M5Cardputer.Display.drawString(statusLine, 10, 3);
}



// render the running line at the top showing the current play position
void renderBeatLine(DrumMachine &dm)
{
    int16_t x, x2, y;
    static const int beatColor = RGB565(0, 50, 0);    
    uint32_t ms = millis();
        
    int32_t sampleTime;
    // from change in milliseconds work out how many samples have elapsed
    sampleTime = (ms - dm.syncMillis) * samplerate / 1000;
    //sampleTime -= dm.stepSamples; // adjust for sloppy timing of change
    int16_t step = sampleTime / dm.stepSamples;
    if(step<0) step += nSteps;
    
    


    if(step != dm.beatTime)
    {
        getCursorPixelPos(dm, dm.beatTime, 0, x, y);        
        // Clear the old beat line
        M5Cardputer.Display.fillRect(x, y-1, dm.cursor.width, 2, TFT_BLACK);
        // draw the new one
        getCursorPixelPos(dm, step, 0, x, y);                
        M5Cardputer.Display.fillRect(x, y-1, dm.cursor.width, 2, beatColor);        
        dm.beatTime = step;
    }

}


void drawStatus(DrumMachine& dm)
{
    int statusHeight = 16;
    char statusLine[256];

    // Set font for the display
    M5Cardputer.Display.setFont(&fonts::Font0);
      // Set text color to green for the status line
    M5Cardputer.Display.setTextColor(GREEN);

    drawTopLine(dm);
    // Clear the status bar area
    M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - statusHeight - 4, M5Cardputer.Display.width(), statusHeight + 4, TFT_BLACK);

  

    // Create the status line with BPM, Swing, Pattern, and Kit information
    snprintf(statusLine, 255, "BPM %03d VOL %02d SW %02d KT %02d", dm.bpm, dm.volume, dm.swing,  dm.kit);
    M5Cardputer.Display.drawString(statusLine, 10, M5Cardputer.Display.height() - statusHeight);

    // Recalculate channels to determine if any are soloed
    int solos = recalcChannels(dm);

    int32_t drawX = 180;
    for (int i = 0; i < nChans; i++)
    {
        snprintf(statusLine, 255, "%01d", i + 1);

        // Check if the channel is enabled
        if (dm.channels[i]._enabled)
        {
            // Set text color based on whether there are soloed channels
            if (solos != 0) // Show red color if soloed channels exist
            {
                M5Cardputer.Display.setTextColor(RED);
            }
            else // No solos, show all channels in green
            {
                M5Cardputer.Display.setTextColor(GREEN);
            }
        }
        else // Gray out disabled channels
        {
            M5Cardputer.Display.setTextColor(TFT_DARKGREY);
        }

        // Draw the channel number
        M5Cardputer.Display.drawString(statusLine, drawX, M5Cardputer.Display.height() - statusHeight);
        drawX += 6; // Move to the next position for the next channel
    }
    // Reset text color to white and font to Font2
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setFont(&fonts::Font2);
}

void setGraphicsModePattern() {
    M5Cardputer.Display.clearDisplay(TFT_BLACK);
    M5Cardputer.Display.setFont(&fonts::Font2);
    M5Cardputer.Display.setTextDatum(top_left);
    M5Cardputer.Display.setTextColor(WHITE);
}

void redrawPattern(DrumMachine& dm) {
    int oldStep = dm.cursor.step;
    int oldChan = dm.cursor.chan;

    for (int i = 0; i < nChans; i++) {
        for (int j = 0; j < nSteps; j++) {
            dm.cursor.step = j;
            dm.cursor.chan = i;
            dm.cursor.dirty = 1;
            drawCursor(dm, 0);
        }
    }
    dm.cursor.step = oldStep;
    dm.cursor.chan = oldChan;
}

// normal pattern mode
void patternModeUpdate(DrumMachine &dm)
{
  updateCursor(dm);
  renderBeatLine(dm);
  patternModeKeys(dm);
  feedPatternBuffers(dm);
  
}

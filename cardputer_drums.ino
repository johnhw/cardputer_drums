
/**
 * @file cardputer_drums
 * @author John H. Williamson
 * @brief M5Cardputer Drum machine
 *
 *
 * @Hardwares: M5Cardputer
 * @Platform Version: Arduino M5Stack Board Manager v2.0.7
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 */

// TODO:

// switch about fn+opt keys
// kit editor
// Loop samples
// Sample tuning
// Channel filters? (e.g. channel sidebar?)

// maybe:
// sd card reading?
// persistent patterns (in Flash with SPIFFS?)


#include <M5Cardputer.h>
#include "datatypes.h"
#include "config.h"
#include "audio.h"
#include "ui.h"
#include "patternui.h"
#include "previewui.h"

// Function Declarations
void initCardputer();
void setup(void);
void loop(void);

static struct DrumMachine machine; // global drum machine state

// Initialize the Cardputer
void initCardputer()
{
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);
  M5Cardputer.Display.startWrite();
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Speaker.setVolume(255);
  M5Cardputer.Speaker.begin();
}

void splash()
{
  M5Cardputer.Display.clearDisplay(TFT_DARKGREY);
  M5Cardputer.Display.setFont(&fonts::FreeMonoBold18pt7b);
  M5Cardputer.Display.setTextDatum(top_center);
  M5Cardputer.Display.setTextColor(TFT_BLACK);
  M5Cardputer.Display.drawString("BonnetHead", M5Cardputer.Display.width() / 2 + 1, M5Cardputer.Display.height() / 2 - 50 + 1);

  M5Cardputer.Display.setTextColor(TFT_LIGHTGREY);
  M5Cardputer.Display.drawString("BonnetHead", M5Cardputer.Display.width() / 2 , M5Cardputer.Display.height() / 2 - 50);

  // draw text in small font below
  M5Cardputer.Display.setFont(&fonts::Font0);
  M5Cardputer.Display.setTextColor(TFT_GREEN);
  M5Cardputer.Display.drawString("Williamson Industries", M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() / 2 + 50);
  


}

void setup(void)
{  
  initCardputer();
  splash();  
  resetState(machine);
}

void loop(void)
{
  M5Cardputer.update();
  if (machine.playMode == 0)
    patternModeUpdate(machine);
  if (machine.playMode == 1)
    previewModeUpdate(machine);
  delay(1);
}
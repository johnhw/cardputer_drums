
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


// live mode (inc. kick timing)
// demo song
// fix pattern sequence mode one pattern lag
// file UI: filtered file list, pattern cursor
// kit editor
// chords in kits
// Loop samples
// Sample tuning


// maybe:
// headphone USB-C audio


#include <M5Cardputer.h>
#include "src/kits.h"
#include "src/datatypes.h"
#include "src/config.h"
#include "src/audio.h"
#include "src/ui.h"
#include "src/patternui.h"
#include "src/previewui.h"
#include "src/flash.h"

// Function Declarations
void initCardputer();
void setup(void);
void loop(void);

static struct DrumMachine machine; // global drum machine state

// set up the Flash filesystem
void initFS()
{
  initLittleFS();
  createDirIfNotExists(basePathRoot);
  createDirIfNotExists(basePathPattern); // make sure we can write to the chosen dir
  createDirIfNotExists(basePathKits); 
  createDirIfNotExists(basePathSamples); 
  
}

// Initialize the Cardputer
void initCardputer()
{
  auto cfg = M5.config();  
  M5Cardputer.begin(cfg);
  M5Cardputer.Display.startWrite();
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Speaker.setVolume(255);
  M5Cardputer.Speaker.begin();
  initFS();
}

void splash()
{

  //M5Cardputer.Display.drawPng(splashPNG, sizeof(splashPNG)/sizeof(splashPNG[0]), 0, 0);

  //LGFX_Sprite splashSprite = LGFX_Sprite(&M5.Lcd);
  // LGFX_Sprite splashSprite = LGFX_Sprite(&M5Cardputer.Display);
  // splashSprite.setColorDepth(16);
  // splashSprite.createSprite(M5Cardputer.Display.width(), M5Cardputer.Display.height());

  auto splashSprite = M5Cardputer.Display; 
  splashSprite.clearDisplay(TFT_DARKGREY); 

  splashSprite.setFont(&fonts::FreeMonoBold18pt7b);
  splashSprite.setTextDatum(top_center);
  splashSprite.setTextColor(TFT_BLACK);
  splashSprite.drawString("BonnetHead", M5Cardputer.Display.width() / 2 + 1, M5Cardputer.Display.height() / 2 - 50 + 1);

  splashSprite.setTextColor(TFT_LIGHTGREY);
  splashSprite.drawString("BonnetHead", M5Cardputer.Display.width() / 2 , M5Cardputer.Display.height() / 2 - 50);

  
  splashSprite.setFont(&fonts::Font0);
  splashSprite.setTextColor(TFT_BLACK);
  splashSprite.drawString(VERSION, M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() / 2 - 20 );

  // draw a rectangle for the lower text
  splashSprite.fillRect(0, M5Cardputer.Display.height() / 2 + 45, M5Cardputer.Display.width(), 55, RGB565(5,5,5));
  // draw text in small font below
  splashSprite.setFont(&fonts::Font0);
  splashSprite.setTextColor(TFT_BLACK);
  splashSprite.drawString("Williamson Industries", M5Cardputer.Display.width() / 2 + 1, M5Cardputer.Display.height() / 2 + 50 + 1);
  splashSprite.setTextColor(TFT_GREEN);
  splashSprite.drawString("Williamson Industries", M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() / 2 + 50);
  
  //splashSprite.pushSprite(0, 0);


}

void setup(void)
{  
  initCardputer();
  splash();    
  machine.kit = -1; // set to -1 so we always set the kit  
  resetState(machine);
  String fname = "startup";
  loadDrumMachine(machine, fname);
  if(machine.kit==-1)
    setKit(machine, 0); // set the default kit if one didn't get loaded
}

void loop(void)
{
  M5Cardputer.update();

  updateUI(machine);
  
  delay(1);
}
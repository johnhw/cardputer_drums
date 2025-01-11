#include <SD.h>
#include <sd_defines.h>
#include <sd_diskio.h>

#include <SD.h>
#include <sd_defines.h>
#include <sd_diskio.h>


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

// chiptunes kits
// CHESNEY HAWKES MODE

// beat slicer? // audio recording
// fix pattern switch visual sync (off by 1/2 bar)
// better error messages etc. for kit load/save
// Loop samples: unify preview. Add envelopes. Better loop boolean (add to adjustments)
// flams: double flam, triple flam, roll, reverse. Key, UI, audio rendering
// UI for step and channel tuning
// allow blank samples to change volume/pitch
// add stop note sample/step ('#') (trigger release)
// add glide for pitch?


// maybe:

// live recording
// bluetooth audio (see: https://github.com/pschatzmann/ESP32-A2DP)
// chords in kits
// kit editor
// basic undo
// beat slicer
// polyrhytm patterns (different channel lengths)
// probability
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
#include "data/routed_7.h"

// Function Declarations
void initCardputer();
void setup(void);
void loop(void);

static struct DrumMachine machine; // global drum machine state

// set up the Flash filesystem
void initFS()
{
  initLittleFS();  
  initSD();
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


  auto splashSprite = M5Cardputer.Display; 
  splashSprite.clearDisplay(TFT_WHITE); 

  splashSprite.setFont(&fonts::Orbitron_Light_24);
  splashSprite.setTextDatum(top_center);
  splashSprite.setTextColor(TFT_BLACK);
  // for(int i=-1;i<2;i++)
  //   for(int j=-1;j<2;j++)
  //   splashSprite.drawString("BONNETHEAD", M5Cardputer.Display.width() / 2 + i, M5Cardputer.Display.height() / 2 - 50 + j);
  
  // splashSprite.setTextColor(TFT_LIGHTGREY);
  splashSprite.drawString("BONNETHEAD", M5Cardputer.Display.width() / 2 , M5Cardputer.Display.height() / 2 - 50);
  
  
  splashSprite.loadFont(font_vlw_routed_7);
  splashSprite.setTextColor(TFT_BLACK);
  
  splashSprite.drawString(VERSION, M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() / 2 - 20 );
  splashSprite.setFont(&fonts::Font0);
  // draw a rectangle for the lower text
  splashSprite.fillRect(0, M5Cardputer.Display.height() / 2 + 45, M5Cardputer.Display.width(), 55, RGB565(0,0,0));
  // draw text in small font below  
  splashSprite.setTextColor(TFT_WHITE);
  splashSprite.drawString("W I L L I A M S O N  I N D U S T R I E S", M5Cardputer.Display.width() / 2 , M5Cardputer.Display.height() / 2 + 52 );
  

}


void setup(void)
{  
  
  initCardputer();
  Serial.println("\n\n\nBonnetHead "+VERSION+"\n\n\n");
  initState(machine);
  splash();    
  
  resetState(machine);
  String fname = "/startup.drm";
  bool success = loadDrumMachine(machine, fname);  
  if(!success)
  {
    machine.kit = -1;
    resetState(machine);
  }

  if(machine.kit==-1)
    setKit(machine, 1); // set the default kit if one didn't get loaded.
  // clear the splash and redraw
  machine.splashFlag = false;
  resetMix(machine);
  updatePattern(machine);
}

void loop(void)
{
  M5Cardputer.update();
  updateUI(machine);
  delay(1);
}
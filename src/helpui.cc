#include <M5Cardputer.h>
#include "helpui.h"
#include "ui.h" 

constexpr const char* helpStrings[] = {
    "^Pattern",
    "a-z sample",
    "0-9 velocity",
    "DEL delete",
    "[  shift back",
    "]  shift fwd",
    "Sh] chan. fwd",
    "Sh[ chan. back",
    
    "^Global",
    "fn-; inc. BPM",
    "fn-, dec. BPM",
    "fn-. dec. swing",
    "fn-/ inc. swing",    
    "fn-[ dec. vol",
    "fn-] inc. vol",
    "ESC Show help",
    
    "^Kit edit",
    "BtA: kit edit.",

    "^Files",
    "fn-s Save",
    "fn-a Save as",
    "fn-o Open ",
    "fn-n New",

    "^Sequencer",
    "op-0-9 pattern ",
    "op-ESC toggle seq.",
    "op-← pat. left",
    "opt-→ pat. right",
    "opt-ENTER ins. pat",
    "opt-DEL del.  pat",

    "^Channel",
    "fn-0-9 mute chan.",
    "sh-0-9 solo chan.",
    "fn+- dec. cutoff",
    "fn+=: inc. cutoff",
    "fn+\\ dec. vol",
    "fn+' inc. vol",

    "^Kits",
    "alt-0-9: load kit",

    "^Clipboard",
    "fn-d delete",
    "fn-c copy",
    "fn-x cut",
    "fn-v paste",

};

// exit on any key
void helpModeKeys(DrumMachine &dm) {
  if (M5Cardputer.Keyboard.isChange())
  {
    if (M5Cardputer.Keyboard.isPressed())
    {
      setPlayMode(dm, dm.lastMode);
    }
  }
}

void helpModeUpdate(DrumMachine &dm)
{
    helpModeKeys(dm);
}

void displayHelpText() {
    int initialX = 6;
    int initialY = 6;
    int x = initialX;
    int y = initialY;
    int size = sizeof(helpStrings) / sizeof(helpStrings[0]);
    int columnOffset = 70; // Width of each column
    int lineHeight = 7;    // Height of each line
    int maxHeight = M5Cardputer.Display.height() - 12;
    M5Cardputer.Display.setFont(&fonts::TomThumb);  
    auto display = M5Cardputer.Display;
    for (size_t i = 0; i < size; ++i) {
        const char* text = helpStrings[i];

        if (text[0] == '^') { // Heading, marked by caret
            display.setTextColor(WHITE);
            display.setTextDatum(top_center);
            display.drawString(text + 1, x + columnOffset / 2, y); // Center heading
        } else { // Regular text
            display.setTextColor(GREEN);
            display.setTextDatum(top_left);
            display.drawString(text, x, y); // Left-justified text
        }

        y += lineHeight; // Move to the next line

        // Check if we've reached the bottom of the screen
        if (y + lineHeight > maxHeight) {
            x += columnOffset; // Start a new column to the right
            y = initialY;      // Reset y position to the top
        }
    }
}

void setGraphicsModeHelp(DrumMachine &dm)
{
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setFont(&fonts::Font0);
  M5Cardputer.Display.setTextColor(GREEN);
  int x = 12;
  int y = 12;
  
  displayHelpText();

}
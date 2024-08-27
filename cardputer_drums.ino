
/**
 * @file mic.ino
 * @author SeanKwok (shaoxiang@m5stack.com)
 * @brief M5Cardputer Microphone Record Test
 * @version 0.1
 * @date 2023-10-09
 *
 *
 * @Hardwares: M5Cardputer
 * @Platform Version: Arduino M5Stack Board Manager v2.0.7
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 */

// TODO:
// fix pattern
// Kits, opt 1-10
// better drums
// sd card reading?
// persistent patterns
// Attack for drums
// Filter for drums
// Loop samples
// Sample tuning
// Channel filters?

#include <M5Cardputer.h>

// one channel
typedef struct channel_t
{
  int16_t volume;
  int8_t mute;
  int8_t solo;
  int8_t _enabled; // NB: not directly modified; updated to reflect mute/solo status of all channels

} channel_t;

// a sample, with PCM data, length and a frequency
typedef struct sample_t
{
  int16_t *samples = 0;
  int32_t len = 0;
  int32_t freqIncrement = 0; // 32768 = 1.0
} sample_t;

// one step of a channel
typedef struct chanData_t
{
  int16_t type;
  int16_t velocity;
  int32_t kickDelay; // additional delay, in kickSubdiv units
} chanData_t;

// the cursor location/flash state
typedef struct cursor_t
{
  int8_t step;
  int8_t chan;
  int8_t width;
  int8_t height;
  int16_t flash;
  int8_t on;
} cursor_t;

// data for mixing one channel into the final mix
typedef struct mixData_t
{
  sample_t *currentSample;
  int32_t sampleIndex;
  int16_t stepIndex;
  int16_t nextIndex;
  int32_t kickDelay;
  int16_t currentVelocity;
} mixData_t;

float iirAlpha(int sr, float freq)
{
  return 1.0 / (1.0 + (sr / (2 * M_PI * freq)));
}

float halfLifeTime(int sr, float t)
{
  return exp(-log(2.0) / ((float)t * (float)sr + 0.0f));
}

// times in milliseconds, frequencies in hZ
void createDrum(sample_t *sample, int32_t len, int32_t samplerate, float delay, float startFreq, float ampAttack, float freqDecay, float ampDecay, float noise, float overdrive)
{
  int32_t j;
  float freq = startFreq;
  float amp = 1.0;
  float ph = 0.0;
  float out;
  int32_t delaySamples;
  delaySamples = delay * samplerate / 1000;
  float ampD = halfLifeTime(samplerate, ampDecay / 1000);
  float freqD = halfLifeTime(samplerate, freqDecay / 1000);
  float ampAttackInc = 1.0 / (samplerate * (ampAttack / 1000));
  float t = 0;

  for (j = delay; j < sample->len; j++)
  {
    ph += (freq * 2 * M_PI) / samplerate;
    out = cos(ph);
    out += noise * random(-32767, 32767) / 32767.0;
    out *= amp;
    out *= overdrive;
    if (out > 1.0)
      out = 1.0;
    if (out < -1.0)
      out = -1.0;
    out += sample->samples[j] / 32767.0; // accumulate on top of existing sample
    sample->samples[j] = tanh(out) * 32767.0f;

    freq *= freqD;
    amp *= ampD;
    // if(t>ampAttack)
    //   amp *= ampD; // exp decay
    // else
    //  amp += ampAttackInc; // linear attack

    t += 1000 / samplerate;
  }
}

// times in milliseconds, frequencies in hZ
void createFM(sample_t *sample, int32_t len, int32_t samplerate, float freq, float ampAttack, float modRatio, float modStart, float modDecay, float ampDecay)
{
  int32_t j;

  float amp = 1.0;
  float ph = 0.0;
  float modPh = 0.0;
  float out;

  float mod = modStart;
  float modFreq = freq * modRatio;
  float ampD = halfLifeTime(samplerate, ampDecay / 1000);
  float modD = halfLifeTime(samplerate, modDecay / 1000);
  float ampAttackInc = 1.0 / (samplerate * (ampAttack / 1000));
  float t = 0;

  for (j = 0; j < sample->len; j++)
  {
    ph += (freq * 2 * M_PI) / samplerate;
    modPh += (modFreq * 2 * M_PI) / samplerate;
    out = cos(ph + cos(modPh) * mod);
    out *= amp;
    out += sample->samples[j] / 32767.0; // accumulate on top of existing sample
    sample->samples[j] = tanh(out) * 32767.0f;

    mod = mod * modD;
    // if(t>ampAttack)
    amp *= ampD; // exp decay
    // else
    // amp += ampAttackInc; // linear attack

    t += 1000 / samplerate;
  }
}

#define RGB565(r, g, b) (((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F))

static constexpr const size_t samplerate = 16000;
static constexpr const int16_t nSteps = 16;
static constexpr const int16_t nChans = 8;
static constexpr const int16_t maxPatterns = 10;
static constexpr const int16_t kickSubdiv = 12; // subdivisions per unit time
static constexpr const int16_t minBPM = 60;     // minimum bpm
static constexpr const int16_t cursorFlashTime = 200;

static sample_t drumSamples[26];
static int16_t *audioBuffers[4] = {NULL, NULL, NULL, NULL}; // 4 buffers
static int8_t waveBufferIndex = 0;
static int32_t patternSamples; // number of samples in a whole pattern
int16_t waveBufferLen;         // samples in one buffer (1/4 pattern)
static int8_t playMode = 0;    // 0=normal; 1=sample preview
int16_t syncMix = 0;           // flag to indicate to remix at the next pattern loop
int16_t stepSamples;          // length of one step in samples (usually 1/16th of a pattern)
int32_t oneKickTime; // time in samples to kick forward by one kickSubdiv of a step (usually 1/12 of a step)

int16_t bpm = 120;
int16_t swing = 0;
int16_t pattern = 0;
int16_t kit = 0;


static cursor_t cursor;
static channel_t channels[nChans];
static chanData_t *currentPattern;
static chanData_t allPatterns[nSteps * nChans * maxPatterns];
static chanData_t clipboard[nSteps * nChans];

// update the enabled flag across channels
// using the solo and mute information
// returns the number of solo'd channels
int recalcChannels()
{
  int solo = 0;
  for (int i = 0; i < nChans; i++)
  {
    solo += channels[i].solo;
  }
  if (solo == 0) // no solo, enable all
  {
    for (int i = 0; i < nChans; i++)
    {
      channels[i]._enabled = 1;
    }
  }
  else
  {
    // select only solo'd channels
    for (int i = 0; i < nChans; i++)
    {
      channels[i]._enabled = channels[i].solo;
    }
  }

  // now apply mutes
  for (int i = 0; i < nChans; i++)
  {
    if (channels[i].mute)
      channels[i]._enabled = 0;
  }
  return solo;
}

// set the solo status for a channel
void toggleSolo(int ix)
{
  if (ix < 0 || ix >= nChans)
    return;
  channels[ix].solo = !channels[ix].solo;
  recalcChannels();
  drawStatus();
  updateMix(0, 0);
}

// set the mute status
void toggleMute(int ix)
{
  if (ix < 0 || ix >= nChans)
    return;
  channels[ix].mute = !channels[ix].mute;
  recalcChannels();
  drawStatus();
  updateMix(0, 0); // TODO: fix if needed (e.g. remove params to updateMix)
}

// create a new sample, with a given length
// and allocate the buffer for it
void allocSample(int8_t index, int32_t len)
{
  drumSamples[index].samples = (int16_t *)allocBuffer(drumSamples[index].samples, len, sizeof(int16_t));
  drumSamples[index].len = len;
  drumSamples[index].freqIncrement = 32768; // for fixed point frequency shifting
}


// allocate a buffer of a given length and size
// and zero it out. Free the existing buffer if
// it is non-NULL
void *allocBuffer(void *buffer, int16_t len, size_t size)
{
  if (buffer != NULL)
    free(buffer);
  buffer = malloc(len * size);
  memset(buffer, 0, len * size);
  return buffer;
}

// allocate the mix buffer for the longest
// possible time (minBPM speed)
void allocateMix()
{
  int32_t bufSamples;
  int32_t maxPatSamples;
  maxPatSamples = nSteps * samplerate * 60 / (minBPM * 4);
  bufSamples = maxPatSamples / 4;
  for (int i = 0; i < 4; i++)
  {
    audioBuffers[i] = (int16_t *)allocBuffer(audioBuffers[i], bufSamples, sizeof(int16_t));
  }
  waveBufferIndex = 0;
}

// recompute the sample sizes for a new BPM
void recalcBPM()
{
  stepSamples = samplerate * 60 / (bpm * 4);
  oneKickTime = stepSamples / kickSubdiv;
  patternSamples = stepSamples * nSteps;
  waveBufferLen = patternSamples / 4;
}

// get pixel coordinates from (step,chan) coordinates
void getCursor(int16_t step, int16_t chan, int16_t &x, int16_t &y)
{
  x = (step + (step / 4)) * 12 + 12;
  y = chan * 12 + 12;
}

// clear the current pattern
void clearPattern()
{
  int oldStep, oldChan;
  oldStep = cursor.step;
  oldChan = cursor.chan;
  for (int i = 0; i < nChans; i++)
  {
    for (int j = 0; j < nSteps; j++)
    {
      setCursorChar(0);
      setCursorVel('6');
      setCursorKick(0);
      cursor.step = j;
      cursor.chan = i;
    }
  }
  cursor.step = oldStep;
  cursor.chan = oldChan;
}

// redraw the whole pattern
// by moving the cursor over each element
// and drawing it!
void redrawPattern()
{
  int oldStep, oldChan;
  oldStep = cursor.step;
  oldChan = cursor.chan;

  int16_t x, y;
  for (int i = 0; i < nChans; i++)
  {
    for (int j = 0; j < nSteps; j++)
    {
      cursor.step = j;
      cursor.chan = i;

      drawCursor(0);
    }
  };
  cursor.step = oldStep;
  cursor.chan = oldChan;
}

// set the pattern, but don't redraw/mix
void _setPattern(int ix)
{
  if (ix < 0 || ix >= maxPatterns)
    return;
  pattern = ix;
  currentPattern = &allPatterns[nSteps * nChans * ix];
}

// set the display mode for rendering the pattern
void patternGraphicsInit()
{
  M5Cardputer.Display.clearDisplay(TFT_BLACK);
  M5Cardputer.Display.setFont(&fonts::Font2);
  M5Cardputer.Display.setTextDatum(top_left);
  M5Cardputer.Display.setTextColor(WHITE);
}

// redraw the whole pattern and mix
void updatePattern()
{
  patternGraphicsInit();
  redrawPattern();
  drawStatus();
  requestMix();
}

// set the pattern element (note) to the default state
void resetPatternElement(struct chanData_t *elt)
{
  elt->type = 0;
  elt->velocity = 6;
  elt->kickDelay = 0;
}

// set the pattern and mix/redraw
void setPattern(int ix)
{
  _setPattern(ix);
  updatePattern();
}

// copy the current pattern to the clipboard
// but only copy the enabled channels
void copyPattern()
{
  int i, j, k;
  k = 0;
  for (j = 0; j < nChans; j++)
  {

    for (i = 0; i < nSteps; i++)
    {
      if (channels[j]._enabled) // only copy enabled channels
        clipboard[k] = currentPattern[k];
      k++;
    }
  }
  updatePattern();
}

// paste the clipboard into the current pattern
// but only writing to enabled channels
void pastePattern()
{
  int i, j, k;
  k = 0;
  for (j = 0; j < nChans; j++)
  {
    for (i = 0; i < nSteps; i++)
    {
      if (channels[j]._enabled) // only paste enabled channels
        currentPattern[k] = clipboard[k];
      k++;
    }
  }
  updatePattern();
}

// clear enabled channels in the current pattern
void selectiveClearPattern()
{
  int i, j, k;
  k = 0;
  for (j = 0; j < nChans; j++)
  {

    for (i = 0; i < nSteps; i++)
    {
      if (channels[j]._enabled) // only clear enabled channels
        resetPatternElement(&currentPattern[k]);

      k++;
    }
  }
  updatePattern();
}

void createSamples()
{
  for (int i = 0; i < 19; i++)
  {
    allocSample(i, 2200); // 0=silence
  }

  // 8 semitones from middle-c
  int16_t noteFreqs[] = {261, 294, 330, 350, 392, 440, 493, 523};

  for (int i = 1; i < 9; i++)
  {
    createFM(&drumSamples[i], 2200, samplerate, noteFreqs[i - 1] * 0.25, 30, 2.0, 8.0, 50, 80);
    createFM(&drumSamples[i], 2200, samplerate, noteFreqs[i - 1] * 0.251, 30, 1.0, 0.0, 20, 100);
  }
  createDrum(&drumSamples[9], 2200, samplerate, 0.0, 450, 0.1, 15, 50, 0.0, 2.5);    // kick i
  createDrum(&drumSamples[10], 2200, samplerate, 0.0, 0, 0.1, 0, 8, 1.0, 0.5);       // hihat j
  createDrum(&drumSamples[11], 2200, samplerate, 0.0, 300, 0.1, 100, 18, 1.0, 3.6);  // snare k
  createDrum(&drumSamples[12], 2200, samplerate, 0.0, 900, 0.05, 1000, 3, 0.0, 1.0); // click l
  createDrum(&drumSamples[13], 2200, samplerate, 0.0, 400, 2, 1000, 15, 0.0, 1.0);   // tom m
  createDrum(&drumSamples[14], 2200, samplerate, 0.0, 2000, 1, 200, 200, 0.2, 0.25); // sweep
  createDrum(&drumSamples[15], 2200, samplerate, 0.0, 1000, 0.1, 200, 3, 0.0, 1.7);  // teek
  createDrum(&drumSamples[16], 2200, samplerate, 0.0, 0.0, 20, 0.0, 3000, 1.0, 0.1); // open hat

  // clap
  createDrum(&drumSamples[17], 2200, samplerate, 0.0, 0, 0.1, 100, 8, 1.0, 8.6);
  createDrum(&drumSamples[17], 2200, samplerate, 25, 0, 0.1, 100, 9, 1.0, 8.9);
  createDrum(&drumSamples[17], 2200, samplerate, 55, 0, 0.1, 100, 7, 1.0, 8.4);
  createDrum(&drumSamples[17], 2200, samplerate, 60, 0, 0.1, 100, 19, 1.0, 2.4);

  createDrum(&drumSamples[18], 2200, samplerate, 0.0, 190, 0.1, 100, 60, 0.0, 1.5); // low kick

}

void initCardputer()
{
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);
  M5Cardputer.Display.startWrite();
  M5Cardputer.Display.setRotation(1);

  M5Cardputer.Speaker.setVolume(255);
  M5Cardputer.Speaker.begin();
}

void setup(void)
{
  
  initCardputer();
  cursor.width = 12;
  cursor.height = 12;
  createSamples();
  resetState();

}

void resetState()
{
  int chan;
  // set global bpm/swing
  bpm = 120;
  swing = 0;
  // reset all channels
  for(chan=0;chan<nChans;chan++)
  {
    channels[chan].volume = 100;
    channels[chan].mute = 0;
    channels[chan].solo = 0;
    channels[chan]._enabled = 1;
  }

  allocateMix();
  recalcBPM();
  recalcChannels();
  createSamples();

  // clear all patterns
  for (int i = 0; i < 10; i++)
  {
    _setPattern(i);
    clearPattern();
  }
  setPattern(1);
  cursor.step = 0;
  cursor.chan = 0;
  cursor.flash = 0;
  cursor.on = 0;
  drawCursor(1);
  updatePattern();
}

void drawStatus()
{
  int statusHeight = 16;
  char statusLine[256];
  M5Cardputer.Display.setFont(&fonts::Font0);
  M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - statusHeight, M5Cardputer.Display.width(), statusHeight, TFT_BLACK);
  M5Cardputer.Display.setTextColor(GREEN);
  snprintf(statusLine, 255, "BPM %03d SW %03d PAT %02d KT %02d", bpm, swing, pattern, kit);
  M5Cardputer.Display.drawString(statusLine, 10, M5Cardputer.Display.height() - statusHeight);

  int solos = recalcChannels();

  int32_t drawX = 180;
  for (int i = 0; i < nChans; i++)
  {
    snprintf(statusLine, 255, "%01d", i + 1);
    if (channels[i]._enabled)
    {
      if (solos != 0) // show red color if solo'd channels
      {
        M5Cardputer.Display.setTextColor(RED);
      }
      else // no solos, show all channels
      {
        M5Cardputer.Display.setTextColor(GREEN);
      }
    }
    else // gray out disabled channels
    {
      M5Cardputer.Display.setTextColor(TFT_DARKGREY);
    }
    M5Cardputer.Display.drawString(statusLine, drawX, M5Cardputer.Display.height() - statusHeight);
    drawX += 6;
  }
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setFont(&fonts::Font2);
}

void getCursorChar(char &ch, int16_t &velocity)
{
  int index = cursor.step + nSteps * cursor.chan;
  int step = currentPattern[index].type;
  if (step < 32)
    ch = '.';
  else
    ch = (char)step;
  velocity = currentPattern[index].velocity;
}

void setCursorKick(int32_t kick)
{
  int index = cursor.step + nSteps * cursor.chan;
  currentPattern[index].kickDelay = kick;
  updateMix(cursor.step, cursor.chan);
}

void setCursorChar(char c)
{
  int index = cursor.step + nSteps * cursor.chan;
  currentPattern[index].type = c;
  updateMix(cursor.step, cursor.chan);
}

void setCursorVel(char c)
{
  int index = cursor.step + nSteps * cursor.chan;
  currentPattern[index].velocity = c - '0';
  updateMix(cursor.step, cursor.chan);
}

int32_t getKickDelay(int16_t step, int16_t chan)
{
  int index = step + nSteps * chan;
  return currentPattern[index].kickDelay;
}

sample_t *getSample(int16_t sampleIndex)
{
  int ix;
  ix = sampleIndex - '`';
  if (ix <= 0 || ix >= 19)
    return NULL;
  return &drumSamples[ix];
}

void getStep(int16_t step, int16_t chan, sample_t *&sample, int16_t &velocity)
{

  int index = step + nSteps * chan;
  sample = getSample(currentPattern[index].type);
  velocity = currentPattern[index].velocity;
}

void updateMix(int16_t step, int16_t chan)
{
  // for now, just mix the whole pattern
  // mix();
  syncMix = 1; // flag to update on next loop
}

void requestMix()
{
  syncMix = 1; // flag to update on next loop
}

void renderCursor(int state)
{
  char *s = "X";
  int16_t drawX, drawY, kickDelay, kickWidth;

  getCursor(cursor.step, cursor.chan, drawX, drawY);
  kickDelay = getKickDelay(cursor.step, cursor.chan);
  kickWidth = 12 * kickDelay / kickSubdiv;
  drawY += cursor.height / 2;
  drawX += kickWidth; // shift left for delayed onset

  char ch;
  int16_t vel;
  getCursorChar(ch, vel);
  s[0] = ch;

  int16_t charColor = RGB565(0, vel * 3, 0);
  int boxOffX = -4;
  int boxOffY = -2;
  int boxX = drawX + boxOffX;
  int boxY = drawY + boxOffY;

  if (state == 0) // no flash
  {
    M5Cardputer.Display.fillRect(boxX - kickWidth, boxY, cursor.width + kickWidth, cursor.height, charColor);
    M5Cardputer.Display.drawString(s, drawX, drawY - cursor.height / 2);
  }
  if (state == 1) // flash on
  {
    M5Cardputer.Display.fillRect(boxX, boxY, cursor.width, cursor.height, TFT_WHITE);
  }
  if (state == 2) // flash off
  {
    M5Cardputer.Display.fillRect(boxX - kickWidth, boxY, cursor.width + kickWidth, cursor.height, charColor);
    M5Cardputer.Display.drawRect(boxX, boxY, cursor.width, cursor.height, TFT_WHITE);
    M5Cardputer.Display.drawString(s, drawX, drawY - cursor.height / 2);
  }
}

void drawCursor(int state)
{
  // "underdraw" next step so that overlaps when kicking forward render correctly
  if (cursor.step < nSteps - 1)
  {
    cursor.step++;
    renderCursor(0);
    cursor.step--;
  }

  renderCursor(state);
}

void moveCursor(int x, int y)
{
  drawCursor(0);
  cursor.step += x;
  cursor.chan += y;

  if (cursor.step < 0)
    cursor.step = 0;
  if (cursor.chan < 0)
    cursor.chan = 0;
  if (cursor.step > nSteps)
    cursor.step = nSteps - 1;
  if (cursor.chan >= nChans)
    cursor.chan = nChans - 1;
}

void mix()
{

  int chan, i, j, k;
  mixData_t mixData[nChans];
  sample_t *newSample;
  int32_t out;
  int32_t gainDrop = 64;
  int32_t bufferIndex;
  int16_t *buffer;
  int16_t newIndex;

  // reset the mix data
  for (chan = 0; chan < nChans; chan++)
  {
    mixData[chan].currentSample = NULL;
    mixData[chan].sampleIndex = 0;
    mixData[chan].stepIndex = -1;
    mixData[chan].nextIndex = -1;
    mixData[chan].kickDelay = 0;
    mixData[chan].currentVelocity = 0;
  }
  k = 0;
  // for each buffer
  for (i = 0; i < 4; i++)
  {
    buffer = audioBuffers[i];

    for (j = 0; j < waveBufferLen; j++)
    {
      out = 0;
      for (chan = 0; chan < nChans; chan++)
      {
        if (!channels[chan]._enabled)
          continue; // skip disabled channels
        newIndex = k / stepSamples;
        if (newIndex != mixData[chan].nextIndex)
        {
          mixData[chan].nextIndex = newIndex;
          if (newIndex % 2 == 0)
            mixData[chan].kickDelay = 0; // no swing on downbeats
          else
            mixData[chan].kickDelay = stepSamples * swing / 200;                 // apply swing (max=50% delay)
          mixData[chan].kickDelay += getKickDelay(newIndex, chan) * oneKickTime; // add on forced kick delay in the channel
        }

        // did it change yet? if so, we need a new sample
        if (mixData[chan].kickDelay == 0)
        {
          // returns NULL if there's no sample there at all
          mixData[chan].stepIndex = mixData[chan].nextIndex;
          getStep(mixData[chan].stepIndex, chan, newSample, mixData[chan].currentVelocity);
          if (newSample != NULL) // cutoff if there's a new sample to start (do nothing otherwise)
          {
            mixData[chan].sampleIndex = 0;
            mixData[chan].currentSample = newSample;
          }
        }
        mixData[chan].kickDelay--;

        // copy in the sample, if there's more to copy
        if (mixData[chan].currentSample)
        {
          out += mixData[chan].currentVelocity * mixData[chan].currentSample->samples[mixData[chan].sampleIndex++];
          // overran the sample, so stop
          if (mixData[chan].sampleIndex >= mixData[chan].currentSample->len)
          {
            mixData[chan].currentSample = NULL;
          }
        }
      }
      // shift to a 16 bit result, clip and write
      out = out / gainDrop;
      if (out > 32767)
        out = 32767;
      if (out < -32767)
        out = -32767;
      buffer[j] = out;
      k++;
    }
  }
}

void adjSwing(int adj)
{
  swing += adj;
  if (swing < 0)
    swing = 0;
  if (swing > 100)
    swing = 100;
  requestMix();
  drawStatus();
}

void adjBpm(int adj)
{
  bpm += adj;
  if (bpm < minBPM)
    bpm = minBPM;
  if (bpm > 320)
    bpm = 320;
  recalcBPM();
  mix();
  drawStatus();
}

// increment/decrement the kickDelay on the current
// cursor element
void adjCursorKick(int adj)
{
  int index = cursor.step + nSteps * cursor.chan;
  int kick = currentPattern[index].kickDelay + adj;
  if (kick < 0)
    kick = 0;
  if (kick >= kickSubdiv)
    kick = kickSubdiv - 1;
  currentPattern[index].kickDelay = kick;
  requestMix();
}

void channelKey(Keyboard_Class::KeysState status)
{
  // solo or mute channels
  if (status.alt)
  {
    if (M5Cardputer.Keyboard.isKeyPressed('1'))
      toggleSolo(0);
    if (M5Cardputer.Keyboard.isKeyPressed('2'))
      toggleSolo(1);
    if (M5Cardputer.Keyboard.isKeyPressed('3'))
      toggleSolo(2);
    if (M5Cardputer.Keyboard.isKeyPressed('4'))
      toggleSolo(3);
    if (M5Cardputer.Keyboard.isKeyPressed('5'))
      toggleSolo(4);
    if (M5Cardputer.Keyboard.isKeyPressed('6'))
      toggleSolo(5);
    if (M5Cardputer.Keyboard.isKeyPressed('7'))
      toggleSolo(6);
    if (M5Cardputer.Keyboard.isKeyPressed('8'))
      toggleSolo(7);
  }
  else
  {
    if (M5Cardputer.Keyboard.isKeyPressed('1'))
      toggleMute(0);
    if (M5Cardputer.Keyboard.isKeyPressed('2'))
      toggleMute(1);
    if (M5Cardputer.Keyboard.isKeyPressed('3'))
      toggleMute(2);
    if (M5Cardputer.Keyboard.isKeyPressed('4'))
      toggleMute(3);
    if (M5Cardputer.Keyboard.isKeyPressed('5'))
      toggleMute(4);
    if (M5Cardputer.Keyboard.isKeyPressed('6'))
      toggleMute(5);
    if (M5Cardputer.Keyboard.isKeyPressed('7'))
      toggleMute(6);
    if (M5Cardputer.Keyboard.isKeyPressed('8'))
      toggleMute(7);
  }
}

void fnKey(Keyboard_Class::KeysState status)
{
  // bpm and swing
  if (M5Cardputer.Keyboard.isKeyPressed(','))
    adjSwing(-1);
  if (M5Cardputer.Keyboard.isKeyPressed('/'))
    adjSwing(1);
  if (M5Cardputer.Keyboard.isKeyPressed(';'))
    adjBpm(1);
  if (M5Cardputer.Keyboard.isKeyPressed('.'))
    adjBpm(-1);

  if (M5Cardputer.Keyboard.isKeyPressed('c'))
    copyPattern();
  if (M5Cardputer.Keyboard.isKeyPressed('v'))
    pastePattern();
  if (M5Cardputer.Keyboard.isKeyPressed('x'))
  {
    copyPattern();
    selectiveClearPattern();
    updatePattern();
  }
  if (M5Cardputer.Keyboard.isKeyPressed('d'))
  {
    selectiveClearPattern();
    updatePattern();
  }
  if (M5Cardputer.Keyboard.isKeyPressed('`'))
  {
    setPlayMode(1);
  }

  // this seems silly...
  if (M5Cardputer.Keyboard.isKeyPressed('0'))
    setPattern(0);
  if (M5Cardputer.Keyboard.isKeyPressed('1'))
    setPattern(1);
  if (M5Cardputer.Keyboard.isKeyPressed('2'))
    setPattern(2);
  if (M5Cardputer.Keyboard.isKeyPressed('3'))
    setPattern(3);
  if (M5Cardputer.Keyboard.isKeyPressed('4'))
    setPattern(4);
  if (M5Cardputer.Keyboard.isKeyPressed('5'))
    setPattern(5);
  if (M5Cardputer.Keyboard.isKeyPressed('6'))
    setPattern(6);
  if (M5Cardputer.Keyboard.isKeyPressed('7'))
    setPattern(7);
  if (M5Cardputer.Keyboard.isKeyPressed('8'))
    setPattern(8);
  if (M5Cardputer.Keyboard.isKeyPressed('9'))
    setPattern(9);
}

void updatePreview()
{
  M5Cardputer.Display.clearDisplay(TFT_DARKGREY);
  M5Cardputer.Display.setFont(&fonts::FreeMonoBold24pt7b);
  M5Cardputer.Display.setTextDatum(top_center);
  M5Cardputer.Display.setTextColor(WHITE);
}

void setPlayMode(int mode)
{
  playMode = mode;
  if (mode == 0)
  {
    updatePattern();
  }
  if (mode == 1)
  {
    updatePreview();
  }
}

void updateCursor()
{
  cursor.flash += 1;
  // draw/flash the cursor
  if (cursor.flash >= cursorFlashTime)
  {
    cursor.flash = 0;
    cursor.on = !cursor.on;
  }
  drawCursor(0); // draw the NEXT character to avoid overlap when kicking forward
  // flash current cursor
  if (cursor.on)
    drawCursor(2);
  else
    drawCursor(1);
}

void patternModeKeys()
{
  if (M5Cardputer.Keyboard.isChange())
  {
    if (M5Cardputer.Keyboard.isPressed())
    {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

      if (status.opt)
      {
        channelKey(status);
      }
      else
      {
        // function mode
        if (status.fn)
        {
          fnKey(status);
        }
        else
        {
          // set characters for sample entry
          if (status.del)
          {
            if (status.ctrl)
            {
              // wipe the whole channel line
              int cursorPos = cursor.step;
              for (int i = 0; i < nSteps; i++)
              {
                moveCursor(i, cursor.chan);
                setCursorChar(0);
              }
              cursor.step = cursorPos;
            }
            setCursorChar(0); // clear
          }
          if (status.word.size() > 0)
          {
            if (isalpha(status.word[0]))
              setCursorChar(status.word[0]);
            if (isdigit(status.word[0]))
              setCursorVel(status.word[0]);
          }
          // cursor
          if (M5Cardputer.Keyboard.isKeyPressed(','))
            moveCursor(-1, 0);
          if (M5Cardputer.Keyboard.isKeyPressed('/'))
            moveCursor(1, 0);
          if (M5Cardputer.Keyboard.isKeyPressed(';'))
            moveCursor(0, -1);
          if (M5Cardputer.Keyboard.isKeyPressed('.'))
            moveCursor(0, 1);
          if (M5Cardputer.Keyboard.isKeyPressed(']'))
            adjCursorKick(1);
          if (M5Cardputer.Keyboard.isKeyPressed('['))
            adjCursorKick(-1);
        }
      }
    }
  }
}

void feedPatternBuffers()
{
  // room in the queue? stuff the buffers
  while (M5Cardputer.Speaker.isPlaying(0) != 2)
  {
    M5Cardputer.Speaker.playRaw(audioBuffers[waveBufferIndex++], waveBufferLen, samplerate, false, 1, 0);
    if (waveBufferIndex > 3)
    {
      if (syncMix) // mix in time for the next buffer change
      {
        syncMix = 0;
        mix();
      }
      waveBufferIndex = 0;
    }
    M5Cardputer.update();
  }
}

// normal pattern mode
void patternModeUpdate()
{
  updateCursor();
  patternModeKeys();
  feedPatternBuffers();
}

void previewSample(char sample)
{
  char *preview = "X";
  sample_t *samplePtr;
  M5Cardputer.Display.clearDisplay(TFT_DARKGREY);
  preview[0] = sample;
  M5Cardputer.Display.drawString(preview, M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() / 2);
  samplePtr = getSample(sample);
  if (samplePtr)
    M5Cardputer.Speaker.playRaw(samplePtr->samples, samplePtr->len, samplerate, false, 1, 0);
}

// in preview mode, just play samples immediately
void previewModeUpdate()
{
  if (M5Cardputer.Keyboard.isChange())
  {
    if (M5Cardputer.Keyboard.isPressed())
    {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
      if (status.fn)
      {
        if (M5Cardputer.Keyboard.isKeyPressed('`'))
          setPlayMode(0);
      }
      else
      {
        if (status.word.size() > 0)
        {
          if (isalpha(status.word[0]))
            previewSample(status.word[0]);
        }
      }
    }
  }
}

void loop(void)
{
  M5Cardputer.update();

  if (playMode == 0)
    patternModeUpdate();
  if (playMode == 1)
    previewModeUpdate();
  delay(1);
}
# Bonnethead: Cardputer drums
![cardputer_drums.png](cardputer_drums.png)

[**DEMO VIDEO**](https://www.youtube.com/watch?v=r_0vd7Hbe3E)

This is a simple and lofi drum machine for the [M5Cardputer](https://shop.m5stack.com/products/m5stack-cardputer-kit-w-m5stamps3?srsltid=AfmBOoqHMPXzj-fobFJN0uUYR1yChvbehNYaBXx4sJz9z3Eqjelnhx3n). 

**Now available on M5Burner as "Bonnethead"** 

## Features

* 16KHz sample rate
* 16 steps per pattern, 8 tracks
* 36 patterns
* Swing/shuffle
* Channel mute/solos
* Per-step velocity and sub-tick timing adjustment
* Pattern sequencer
* Per-channel filters
* Multiple drum kits
* Live keydrumming mode
* Render to SD card
* Sampled drum kits from SD card
* Pitching and tuning, including portamento
* Loops
* Tap tempo

## UI
The dot grid indicates the 16x8 steps for the current pattern. 

### Status line

#### Top
* [0] Pattern number
* If there's a pattern sequence, the patterns appear next to the pattern number at the top line (e.g. `12241223`)

### Bottom
* **BPM** current BPM (60-320)
* **SW** current swing (0-99; about 20-30 is useful)
* **VOL** volume (0-16)
* **KT** kit selected 
* At the right Current note: NNN VV KK FX P
    * NNN: note name, like `C#4`
    * VV: velocity (0-9)
    * KK: kick timing (0-11)
    * FX: effect name
    * P: portamento time (0-9) 0=off, 9=slowest

## Controls

### Pattern
* ` ` (space) tap tempo. Press four times to set the tempo.
* `a-z` enter sounds. See the drum map for details.
* `0-9` sets the velocity of the current step (0=silent, 9=max. 6=default)
* `del` deletes the current step
* `[` "kick left" shift step timing back by 1/12 step
* `]` "kick right" shift step timing forward by 1/12 step

* `fn+shift+←` rotate current channel left
* `fn+shift+→` rotate current channel right
* `shift+[` kick current channel one 1/12 step left
* `shift+]` kick current channel one 1/12 step right

* `ctrl+key` set note pitch. `q`=middle C. Piano keyboard layout: `q2we4r5ty7u8i9op` for the top octave, `zsxdcvgbhnjm` for the bottom octave.
* `ctrl+[` decrease pitch by 10 cents
* `ctrl+]` increase pitch by 10 cents
* `ctrl+↑` increase pitch by 1 octave
* `ctrl+↓` decrease pitch by 1 octave
* `ctrl+shift+<n>` set portamento time to 0-9 (0=off, 9=slowest). Default is 0
* `tab` cycle through FX

### Global
* `fn+↑` increase BPM
* `fn+↓` decrease BPM

* `fn+←` decrease swing
* `fn+→` increase swing

* `fn+[` decrease volume
* `fn+]` increase volume

* `Button A` enter instant preview mode -- press a key to hear the sample preview. `Button A` switches back to pattern mode.

### Files

* `fn+s` Save the current pattern in the startup buffer (automatically reloaded on startup) 
* `fn+a <x>` Save the file as bank <x> (0-9a-z)
* `fn+o <x>` Open bank <x> (0-9a-z)
* `fn+n` New file, reset everything 
* `fn+r` Render the current pattern/pattern sequence to a WAV file on the SD card. The file will be named `/bonnethead/renders/render-<n>.wav`

### Sequencer

* `opt+<x>` where `n` is 0-9a-z selects pattern `n`
* `opt+shift+<x>` fill pattern `n` after this pattern, then resume 
* `opt+ESC` toggle pattern sequence mode
* `opt+←` pattern cursor left
* `opt+→` pattern cursor right
* `opt+ENTER` insert current pattern at cursor
* `opt+DEL` delete pattern at cursor

### channel
* `fn+k` enter keydrumming mode. Press keys to play the drum sounds. Press `fn+k` to exit keydrumming mode. (note: red circle at bottom indicates live keydrumming mode). `Fn-shift-K` enables **quantized** keydrumming mode.
* `fn+<n>` toggle mute channel `n` (1-8)
* `fn+shift+<n>` toggle solo channel `n` (multiple channels can be solo'd)
* `fn+enter` toggle solo the current channel (where the cursor is)
* `fn+-` decrease filter cutoff on active channels (16 steps of filter)
* `fn+=` increase filter cutoff on active channels
* `fn+\` decrease channel volume on active channels
* `fn+'` inccrease channel volume on active channels

### Kits
* `alt+<x>` select drum kit `x` (note: this will cause a delay as the kit is synthesised/loaded)
    * Only kits 1-4 are available w/o an SD card.

Note: you can create custom sampled kits by placing WAV files in `/bonnethead/kits/base-<n>.kit` on the SD card. (e.g. `/bonnethead/kits/base-9.kit`). Each WAV should be named `a.wav` through `y.wav` (any can be omitted, and will just be skipped). The kit will be loaded as `9` when you press `alt+9` in this example. `5-9a-z` are available for custom kits (1-4 are synthesised). WAV files should be 16-bit, 16KHz mono.

### clipboard
* `fn+d` delete current pattern
* `fn+c` copy current pattern to clipboard
* `fn+x` cut current pattern to clipboard
* `fn+v` paste current pattern to clipboard
* NOTE: all clipboard functions operate only on currently enabled channels. So e.g. solo'ing one channel allows you to selectively delete/copy it (or muting all but three channels, etc.)


## Drumkit

* `a` -> `h` bass, major scale
* `i` kick
* `j` closed hat
* `k` snare 
* `l` click/rimshot
* `m` tom
* `n` sweep
* `o` beep
* `p` open hat
* `q` clap
* `r` low bass
* `s` cowbell
* `t` damped bell
* `u` crash
* `z` Note off (appears as `#`)
* `,` Note continue (appears as '). This can be used to slide notes or change velocity without retriggering the sample.

## Preview mode

In preview mode, samples can be played with `a-z`. The current sample is displayed at the top of the screen. Press `Button A` to return to pattern mode.

Parameters can be edited:
* `detune` (cents)
* `cutoff` (0-1000)
* `volume` (-5000-5000) centibels
* `start` start point in samples
* `end` end point in samples
* `lp start` loop start point in samples (note: enabled if loop start!=loop end)
* `lp end` loop end point in samples
* `attack` attack time in ms
* `decay` decay time in ms
* `sustain` sustain level in centibels
* `release` release time in ms

### Keys
* `fn+s` <n> save kit (i.e. parameters of the current kit)
* `fn+o` <n> load kit (i.e. parameters of the current kit)
* `fn+n` reset kit
* `fn+r` enter live sampling/recording mode.
* `opt` hold for fine adjust (+/- 1)
* `alt` hold for coarse adjust (+/- 100)
* `alt+opt` hold for very coarse adjust (+/- 1000)
* `↑` increase value (+/- 10 by default)
* `↓` decrease value (+/- 10 by default)
* ` ` reset parameter to 0
* `←` previous parameter
* `→` next parameter 
* `tab` cycle loop mode (off, forward, stretch, pingpong)


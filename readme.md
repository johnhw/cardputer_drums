# Cardputer drums

This is a simple and lofi drum machine for the [M5Cardputer](https://shop.m5stack.com/products/m5stack-cardputer-kit-w-m5stamps3?srsltid=AfmBOoqHMPXzj-fobFJN0uUYR1yChvbehNYaBXx4sJz9z3Eqjelnhx3n). 

## Features

* 16KHz sample rate
* 16 steps, 8 tracks
* 10 patterns
* Swing/shuffle
* Channel mute/solos
* Per-step velocity and sub-tick timing adjustment

## UI
The dot grid indicates the 16x8 steps for the current pattern. 

### Status line

* **BPM** current BPM
* **SWI** current swing (0-100; about 20-30 is useful)
* **PAT** current pattern number
* **KIT** kit selected (currently only one kit)
* **12345678** channel enabled status. Mute'd channels are grayed. Solo'd channels are red. 

## Controls

* Letter keys `a-z` enter sounds. See the drum map for details.
* `0-9` sets the velocity of the current step (0=silent, 9=max. 6=default)
* `del` deletes the current step
* `[` "kick left" shift step timing back by 1/12 step
* `]` "kick right" shift step timing forward by 1/12 step
* `fn+<n>` where `n` is 0-9 selects pattern `n`

* `fn+↑` increase BPM
* `fn+↓` decrease BPM

* `fn+←` decrease swing
* `fn+→` increase swing

* `fn+esc` enter instant preview mode -- press a key to hear the sample preview. `fn+esc` switches back to pattern mode.

### channel
* `opt+<n>` mute channel `n`
* `opt+alt+<n>` solo channel `n`

### clipboard
* `fn+d` delete current pattern
* `fn+c` copy current pattern to clipboard
* `fn+c` cut current pattern to clipboard
* `fn+v` paste current pattern to clipboard
* NOTE: all clipboard functions operate only on currently enabled channels. So e.g. solo'ing one channel allows you to selectively delete/copy it (or muting all but three channels, etc.)
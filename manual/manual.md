**BONNETHEAD DRUM MACHINE MANUAL**  

Welcome to the **Bonnethead Drum Machine** for the **M5Cardputer**! This manual will guide you through all the features, provide a step-by-step tutorial to help you get started, and list every command reference so you can get the most out of your new lofi groove box. Whether you’re a seasoned beat maker or a newcomer to drum programming, Bonnethead is designed to offer a streamlined workflow and creative flexibility in a compact package.

---

## 1. FEATURES

Bonnethead is small, but it’s loaded with features to help you craft punchy and interesting drum patterns on the go. Below is an overview of what this drum machine can do:

1. **16kHz Sample Rate**  
   Enjoy a pleasantly crunchy, lofi sound with an internal sample rate of 16kHz that gives your beats a nostalgic flavor.

2. **16 Steps, 8 Tracks**  
   Each pattern contains 16 discrete steps across 8 possible drum or instrument tracks, offering enough complexity for rich grooves while keeping the workflow simple.

3. **20 Patterns**  
   Craft up to 20 individual patterns. Whether you’re composing an entire song or simply jamming around, you have plenty of room to experiment with different rhythms.

4. **Swing/Shuffle**  
   Add rhythmic feel and shuffle to your patterns, ranging from a subtle groove to an aggressive swing that can completely change the vibe of your loop.

5. **Channel Mute/Solos**  
   Quickly isolate or silence specific channels during playback. Mute multiple channels at once, or solo multiple channels for surgical control over your mix.

6. **Per-Step Velocity and Sub-Tick Timing**  
   Fine-tune each note’s dynamics with velocity, and shift its timing by up to 1/12 of a step either forward or backward for an organically human feel.

7. **Pattern Sequencer**  
   String patterns together in a sequence to create a longer arrangement or a full track. Transition between patterns seamlessly.

8. **Per-Channel Filters**  
   Sculpt the tone of each channel via adjustable filter cutoff settings. Apply gentle roll-offs or radical filtering to create new textures.

9. **Multiple Drum Kits**  
   Swap drum kits on the fly to drastically change the sonic character of your patterns. Each kit has a different sound palette.

10. **Live Keydrumming Mode**  
    Use the keyboard to trigger and record drum hits in real time. Choose between immediate playback or quantized step recording.

11. **Render to SD Card**  
    Export your masterpiece as a WAV file and save it directly to your SD card. Perfect for archiving your creations or sharing them online.

12. **Sampled Drum Kits**  
    Sampled percussion sounds are loaded into memory and triggered at 16kHz. Each kit merges convenience with the authenticity of real recordings.

---

## 2. TUTORIAL

In this section, we’ll walk you through some common tasks and demonstrate how to leverage Bonnethead’s features. By the end of these steps, you’ll be comfortable creating patterns, editing them, sequencing, and saving your work.

### 2.1 Creating Your First Pattern

1. **Powering On and Understanding the UI**  
   Once Bonnethead is running on your M5Cardputer, you’ll see an 8×16 dot grid in the main display area. This grid represents 8 channels (vertical axis) and 16 steps (horizontal axis). At the top, you’ll see the **pattern number** in brackets (e.g., `[0]`), and at the bottom, you’ll see the BPM, Swing (SW), Volume (VOL), Kit (KT), and the status of each channel (`1 2 3 4 5 6 7 8`).

2. **Selecting an Empty Pattern**  
   By default, Pattern 0 is selected. If you’d like to move to another pattern (say Pattern 1) before you start, press `opt+1`. The status line at the top will update to `[1]` indicating you’re now working on Pattern 1.

3. **Entering Step Data**  
   Let’s say you want to add a kick drum on Channel 1 (the top row). Move your cursor to the desired step (for instance, step 1 in the top row). Press the letter `i` on the keyboard (the default mapping for a kick drum). You’ll see the step light up on the display grid, indicating a note is placed there. Repeat this for steps 5, 9, and 13 to get a basic four-on-the-floor pattern.

4. **Adjusting Velocity**  
   While on a given step, you can set its velocity from 0 to 9 (0 = silent, 9 = maximum, 6 = default). For example, if you’d like the first kick to be louder, move the cursor to that step and press `9`. If you’d like the second kick to be softer, press `4`, and so on.

5. **Shifting Timing (Sub-Ticks)**  
   To add a humanized feel, you can shift each step slightly ahead or behind the main grid. While on the step you’d like to shift, press `[` to move it backward by 1/12 of a step, or `]` to move it forward. This will nudge the timing for just that single hit.

6. **Playback**  
   Bonnethead will automatically loop your pattern. If not, ensure your sequencer or pattern mode is active by tapping `opt+ESC` to toggle. You should now hear your pattern play repeatedly.

### 2.2 Adding More Elements

1. **Hi-Hats and Snare**  
   Move your cursor to another channel (say Channel 2). Add a hi-hat by pressing `j` for a closed hat or `p` for an open hat. Place them on steps 3, 7, 11, and 15, for instance, to add a syncopated pattern. Next, move to Channel 3 for a snare drum by pressing `k`. Typical snare placement is on steps 5 and 13 if you want a classic backbeat.

2. **Using Swing/Shuffle**  
   Adjust the swing with the function keys. For instance, press `fn+→` to increase the swing or `fn+←` to decrease it. A swing setting of around 20–30 is often pleasing for a classic hip-hop or R&B shuffle. Listen as your pattern gains a new rhythmic feel.

3. **Channel Mute and Solo**  
   If you want to temporarily silence the hi-hat channel, press `fn+2`. This toggles the mute on Channel 2. You’ll notice the channel’s number turn gray in the bottom status line. If you want to hear only the snare, press `fn+shift+3` to solo Channel 3. Channel 3’s number will turn red, indicating a soloed channel. You can solo multiple channels at once.

### 2.3 Building a Pattern Sequence

1. **Inserting Patterns in the Sequence**  
   Press `opt+ENTER` to insert the current pattern (in this case, Pattern 1) at the sequence cursor.

2. **Switching Patterns for the Sequence**  
   Switch to Pattern 2 (`opt+2`), create or load a new pattern, and then insert it into the sequence by pressing `opt+ENTER` again at the next position. Continue this process to build a chain of patterns.

3. **Editing the sequence**
    You can use `opt+←` and `opt+→` to move the sequence cursor left or right. If you want to remove a pattern from the sequence, press `opt+DEL` to delete it.

3. **Playing the Sequence**  
   Once you have multiple patterns in the sequence, press `opt+ESC` again to begin playback in sequence mode. Bonnethead will automatically progress through each pattern in the order you inserted them.

### 2.4 Saving and Loading

1. **Saving Your Project**  
   Press `fn+s` to save your current pattern to the temporary scratch buffer. To give it a proper file name, press `fn+a`. You’ll be prompted to enter a file name—type something like `mybeat1` and press ENTER.

2. **Loading a File**  
   Press `fn+o` to open the file selection dialog. Type to filter file names, use the up/down arrows to highlight your file, and press ENTER to load it.

3. **Creating a New Project**  
   If you want to start a fresh project, press `fn+n`. You’ll be asked for confirmation (y/n). Doing so resets all patterns and sequence data.

### 2.5 Rendering to WAV

If you’ve created a beat you love, you can export it to an SD card. Simply press `fn+r`. Bonnethead will create a WAV file in `/bonnethead/renders/` named `render-<n>.wav`, where `<n>` increments each time you render.

---

## 3. REFERENCE

This section lists all the key commands and functionality in detail. Refer back to it anytime you need a refresher on how something works.

---

### 3.1 Pattern Editing Commands

1. **Placing a Sound**  
   - **Keys `a–z`**: Enter drum sounds at the current cursor location.  
   - Mapping:  
     - `i`: Kick  
     - `j`: Closed hat  
     - `k`: Snare  
     - `l`: Click/Rimshot  
     - `m`: Tom  
     - `n`: Sweep  
     - `o`: Beep  
     - `p`: Open hat  
     - `q`: Clap  
     - `r`: Low bass  
     - `s`: Cowbell  
     - `t`: Damped bell  
     - `u`: Crash  
     - `a–h`: Additional melodic bass or major scale notes  
   - Each character triggers a specific sample from the current drum kit.

2. **Velocity**  
   - **Keys `0–9`**: Sets the velocity (volume intensity) of the placed sound.  
     - `0` = silent  
     - `9` = maximum volume  
     - `6` = default medium volume  

3. **Deleting a Step**  
   - **`del`**: Erases the note at the current step (if any).

4. **Nudging Step Timing**  
   - **`[`**: Shifts the current step’s timing backward by 1/12 of a step.  
   - **`]`**: Shifts the current step’s timing forward by 1/12 of a step.

---

### 3.2 Transport and Tempo Controls

1. **BPM Adjustment**  
   - **`fn+↑`**: Increases BPM by 1.  
   - **`fn+↓`**: Decreases BPM by 1.  
   - You can hold the key to adjust rapidly.

2. **Swing/Shuffle**  
   - **`fn+←`**: Decrease swing by 1.  
   - **`fn+→`**: Increase swing by 1.  
   - Swing is displayed on the bottom line as `SW`. Values range from 0 (no swing) to 99 (extreme shuffle).

3. **Volume (Master Volume)**  
   - **`fn+[`**: Decrease master volume by 1 (0–16 range).  
   - **`fn+]`**: Increase master volume by 1.

---

### 3.3 Preview and Pattern/Live Modes

1. **Instant Preview Mode**  
   - **`Button A`**: While held, the machine enters a preview mode. Press any sound key (`i`, `j`, `k`, etc.) to audition the drum sound in isolation. Release `Button A` to return to pattern editing mode.

2. **Keydrumming Mode**  
   - **`fn+k`**: Toggle live keydrumming mode. In this mode, pressing the sound keys (`a–z`) will trigger the sounds immediately and write it into the pattern. 
    - In this mode, the recording is *not* quantized and you can create freeform rhythms.
   - **fn+Shift+K**: Enables step-quantized keydrumming mode, meaning each live press is automatically placed on the closest step.  
   - Look for the red circle at the bottom to indicate that you’re in keydrumming mode.

---

### 3.4 File Management

1. **Save to Scratch Buffer**  
   - **`fn+s`**: Saves the current pattern data to a temporary buffer. If you haven’t yet assigned a name, it remains an untitled scratch.

2. **Save As**  
   - **`fn+a`**: Prompts for a filename to save the pattern permanently. Press ENTER to confirm.

3. **Open File**  
   - **`fn+o`**: Opens a file browser. Type in the filter text or use up/down to find your file, then press ENTER to load.

4. **New File**  
   - **`fn+n`**: Clears all pattern and sequence data. Requires `y/n` confirmation.

5. **Render to WAV**  
   - **`fn+r`**: Renders the current pattern (or sequence if the sequencer is running) to a WAV file in `/bonnethead/renders/render-<n>.wav`.

---

### 3.5 Sequencer Controls

The sequencer allows you to chain patterns together to create longer arrangements or full tracks.

1. **Selecting a Pattern Directly**  
   - **`opt+<n>`** where `<n>` is `0–9`: Switches immediately to Pattern `<n>`.

2. **Toggle Pattern Sequence Mode**  
   - **`opt+ESC`**: Switches between editing a single pattern vs. editing/playing a sequence of patterns.

3. **Navigation in Sequence**  
   - **`opt+←` / `opt+→`**: Moves the sequencer cursor left/right through the sequence slots.

4. **Insert Pattern into Sequence**  
   - **`opt+ENTER`**: Inserts the current pattern at the cursor location in the sequence.

5. **Delete Pattern from Sequence**  
   - **`opt+DEL`**: Removes the pattern at the cursor location from the sequence.

---

### 3.6 Channel Controls

Channel controls affect one channel (one horizontal row in the pattern grid) at a time. A channel is **enabled** if it’s soloed or if there's no solo and it’s not muted. Most operations act only on *enabled* channels, and do nothing to other channels. This is useful for selective editing and muting.


1. **Mute**  
   - **`fn+<n>`**: Toggles mute on channel `<n>`. Muted channels appear grayed out in the bottom status line. Multiple channels can be muted simultaneously.
   

2. **Solo**  
   - **`fn+shift+<n>`**: Toggles solo on channel `<n>`. Soloed channels are highlighted in red. Multiple channels can be soloed.  
   - **`fn+enter`**: Solo/unsolo the currently selected channel based on the cursor’s vertical position in the pattern grid.
   
3. **Per-Channel Filter Cutoff**  
   - **`fn+-`**: Decrease filter cutoff on all *enabled* (i.e., not muted) channels by one step.  
   - **`fn+=`**: Increase filter cutoff on all *enabled* channels by one step.  
   - There are 16 levels of filter cutoff from fully closed (dark) to fully open (bright).

4. **Per-Channel Volume**  
   - **`fn+\`**: Decrease channel volume on all *enabled* channels by one step.  
   - **`fn+'`**: Increase channel volume on all *enabled* channels by one step.  
   - Each channel’s volume can be adjusted independently from the master volume.

---

### 3.7 Kits Management

1. **Selecting a Drum Kit**  
   - **`alt+<n>`**: Switch to drum kit `<n>`. Kits are loaded from memory or dynamically generated, so allow a brief moment for loading. Each kit offers a different sound set, which may vary in timbre and pitch.

2. **Kit Indicator**  
   - The currently selected kit is shown as `KT` on the bottom line. For example, `KT=2` means you’re using kit number 2.

---

### 3.8 Clipboard and Pattern Operations

1. **Delete Current Pattern**  
   - **`fn+d`**: Erases the entire current pattern from memory (all 16×8 steps). Only channels that are *enabled* (not muted, or specifically soloed) are affected. For example, if you have one channel soloed, only that channel’s data is deleted.

2. **Copy/Cut Pattern**  
   - **`fn+c`** (pressed once): Copies the current pattern’s data for the *enabled* channels.  
   - **`fn+x`** Cuts the currents pattern data for the *enabled* channels.
   - After copying or cutting, the data remains in the clipboard until overwritten by another copy or cut operation.

3. **Paste Pattern**  
   - **`fn+v`**: Pastes the clipboard data onto the current pattern. Again, only channels that are *enabled* will receive the pasted data. This allows partial merges if you only want to paste the data for specific channels.

4. **Combining Mute/Solo with Clipboard**  
   - Use Mute or Solo states to isolate which channels you want to delete, copy, or paste. For instance, solo Channel 1 if you only want to copy that channel’s pattern. This selective approach helps quickly rearrange your beats.

---

### 3.9 Drum Kit Sound Map

Below is the default mapping of letters to drum/percussion voices. Note that different kits may alter the timbre, but the fundamental letter assignments remain consistent:

- **`a` →** Bass note (lowest pitch or start of a major scale for melodic kits)  
- **`b` →** Bass note (next in scale)  
- **`c` →** Bass note (continuing up the scale)  
- **`d` →** Bass note  
- **`e` →** Bass note  
- **`f` →** Bass note  
- **`g` →** Bass note  
- **`h` →** Bass note (highest pitch or end of scale)  

- **`i` →** Kick drum  
- **`j` →** Closed hat  
- **`k` →** Snare  
- **`l` →** Click / Rimshot  
- **`m` →** Tom  
- **`n` →** Sweep (a short noise sweep)  
- **`o` →** Beep (synth bleep)  
- **`p` →** Open hat  
- **`q` →** Clap  
- **`r` →** Low bass (an 808-style sub, if the kit supports it)  
- **`s` →** Cowbell  
- **`t` →** Damped bell / short chime  
- **`u` →** Crash cymbal  

---

## Tips and Best Practices

- **Using Velocity for Groove**: Even small differences in velocity can make a pattern sound more alive. For instance, on a hi-hat line, try alternating velocities between 6 and 5 (like 6,5,6,5…).  
- **Sub-Tick Offsets**: Subtle offset changes (`[` or `]` once or twice) can give a human feel. Overdoing it, however, might cause the beat to feel messy.  
- **Swing**: High swing settings (>50) can yield interesting polyrhythms, but often a moderate range (20–30) is perfect for typical funk or hip-hop vibes.  
- **Mutes and Solos**: In a live performance scenario, use mute and solo to quickly emphasize or remove specific channels to maintain interest.  
- **Keydrumming**: This is especially useful for capturing spontaneous ideas. Step-quantized mode (Shift+K) can keep your improvisations neat if you want them locked to the grid.  
- **Saving Frequently**: Because you can quickly experiment and drastically change your pattern, it’s good practice to save versions incrementally. Use descriptive file names (e.g., `beat_hiphop_v1`, `beat_hiphop_v2`, etc.).  
- **Rendering**: If you plan to edit your rendered audio further in a DAW or share it online, remember it’s saved at 16kHz. You may want to resample or use sample-rate converters if your final project is at 44.1kHz or higher.

---

## Conclusion

You’ve reached the end of the **Bonnethead Drum Machine** manual for the **M5Cardputer**! This compact groove box can handle punchy sequences, crunchy lofi textures, and dynamic drum programming. With 20 patterns to arrange, powerful swing options, and the ability to manipulate each channel’s filter and volume, Bonnethead is a versatile tool for both studio work and live performance. 

The **Features** section gave you a broad overview of Bonnethead’s capabilities—from per-step velocity to multiple kits and WAV rendering. The **Tutorial** section walked you through creating and editing your first pattern, exploring new features like channel mute, solo, and the pattern sequencer, as well as saving and loading your beats. Finally, the **Reference** section ensures every command is at your fingertips whenever you need it.

Armed with this knowledge, you can confidently start crafting rhythms and textures that push the boundaries of lofi drum programming. With practice, you’ll discover new sonic tricks—combining filter sweeps, sub-tick nudges, and creative velocity manipulation to find your signature style. Enjoy your journey with Bonnethead, and happy beat making!
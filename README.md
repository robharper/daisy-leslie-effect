# Leslie Speaker Effect

This is a basic interpretation of a [Leslie rotating speaker](https://en.wikipedia.org/wiki/Leslie_speaker) effect for the Electro-Smith Daisy, specifically the PedalPCB Terrarium build. The algorithm is:
 1. Split the incoming signal into two parts using low-pass and high-pass filters
 2. Pitch-shift each part using a LFO (separate LFOs for high/low) to simulate the doppler effect
 3. Amplitude modulate each part using the same LFO to simulate amplitude drop when speaker is facing away from listener
 4. Mix the low and high back together for the output

Controls are as follows:
 * Knob 1: Controls the rotation speed of slow ("chorale") mode; 0 - 4 Hz
 * Knob 2: Controls the rotation speed of fast ("tremolo") mode; 4 - 10 Hz
 * Knob 3: Sets the fraction of the rotation speed of the drum (low) with respect to the horn. In real Leslie speakers the drum would often rotate at a slower rate due to its larger size; 0 - 1, multiplier of horn speed
 * Knob 4: Variable "top boost", controls the ratio between high and low when mixed back together. Allows for "boosting" either low or high signal components; 0.3 - 0.7 where 0.5 (middle) is equal mix
 * Knob 5: Controls the intensity of the tremolo (amplitude moddulation) effect; 0 - 1, at zero no modulation, at 1 on to full-off modulation
 * Knob 6: Controls the intensity of the vibrato (ptich shifting) due to the doppler effect. 0.3 - 0.7 ms, where 0.5ms (middle) is typical Leslie-sounding doppler

Footswitches:
 * Footswitch 1: Bypass
 * Footswitch 2: Toggles between fast ("tremolo") and slow ("chorale") rotation rates

Params in the code:
 * `acceleration` - controls the speed at which the rotation rate changes. This simulates the inertia of the rotating parts and gives that nice slide into a new speed
 * `brk_freq` - controls the break frequency of the low/high crossover. Leslie speakers typically use 800Hz
 * `q` - controls the resonance of the filters, affecting the "sharpness" of the cutoff.

## Building
Ensure that the two primary daisy libraries are relative to this project at `../libDaisy` and `../DaisySP` or edit `LIBDAISY_DIR` and `DAISYSP_DIR` in the Makefile. For Terrarium, ensure the header is here `../Terrarium`.

To build and upload:
```
make
make program-dfu
```
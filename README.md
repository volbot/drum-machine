# VB-909 Drum Machine
A sample-based Arduino drum machine that I built as a side-project about a year ago.

`165test` and `dfplayertest` are aptly-named test programs I built to understand what I was doing with new components I'd never used. `drum-machine` is my first attempt, which I did mostly just as a proof-of-concept.

This leaves `drum_machine_2`, which is my final product. Because I was on a bit of a crunch to enter it into a competition, it's a bit sloppy, but it gets the job done quite well.

The physical device uses:
- an Arduino Uno R3
- one DFPlayer to play MP3 files from a microSD card
- a simple two-pin speaker
- an LCD1602 to display information 
- two rotary encoders (with two buttons built-in)
- 16 buttons (not including those two)
- three 74HC595 shift registers

Because it only uses a single DFPlayer, it has no polyphony — it can only play one sound at a time, and playing a new sound will truncate the tail of any currently-playing sound. With a more robust workflow, I'd like to fix this, but considering my interests, I'd much sooner learn to synthesize the sounds myself, either on a hardware or software level, so that's neither here nor there.

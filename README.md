# PBC Bass

PBC Bass is a synthesiser plugin focused on making Reese-style basses.

It builds as:
- VST3
- AU
- Standalone

The synth is built around:
- two detuned saw oscillators
- a sine sub oscillator
- ADSR amplitude envelope
- resonant low-pass filter
- LFO filter movement
- drive/saturation
- simple parameter-based UI

## Sound Design Direction

PBC Bass is designed as a compact Reese bass instrument.

The current voice architecture is:

- Saw oscillator A
- Saw oscillator B with detune
- Sub sine oscillator one octave down
- Amp envelope
- Low-pass filter
- LFO modulation on filter cutoff
- Drive
- Output gain

This gives you a dark, moving, detuned bass sound that works as a solid Reese starting point.

## Controls

The UI currently exposes 8 controls:

- Output — final output level in dB
- Detune — pitch spread between the two saw oscillators
- Sub — sub oscillator level
- Cutoff — low-pass filter cutoff frequency
- Resonance — filter resonance
- Drive — saturation amount
- LFO Rate — speed of filter movement
- LFO Depth — strength of filter movement

## Build Requirements

- CMake 3.25+
- A C++23-capable compiler
- Git
- macOS development environment for AU/Standalone/VST3 builds

## Building

### Debug

```bash
cmake --preset debug
cmake --build --preset debug
```

### Release

```bash
cmake --preset release
cmake --build --preset release
```

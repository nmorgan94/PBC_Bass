# SampleRealm: Reese

A synthesiser plugin focused on making Reese-style basses.

It builds as:
- VST3
- AU
- Standalone
## Sound Design Direction

The synth is designed as a compact Reese bass instrument.

The current voice architecture is:

- Saw/Triangle/Square oscillator A
- Saw/Triangle/Square  oscillator B 
- Detune
- Sub sine oscillator one octave down
- Amp envelope
- Low-pass filter
- Tempo-syncable LFO modulation on filter cutoff
- Drive
- Output gain

This gives you a dark, moving, detuned bass sound that works as a solid Reese starting point.

## Controls

The UI currently exposes 9 controls:

- Output — final output level in dB
- Detune — pitch spread between the two saw oscillators
- Sub — sub oscillator level
- Cutoff — low-pass filter cutoff frequency
- Resonance — filter resonance
- Drive — saturation amount
- LFO Rate — speed of filter movement (Hz or tempo-synced note divisions)
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

## Debugging in Xcode

To debug the plugin in Xcode with an executable:

### 1. Generate Xcode Project

```bash
cmake -B build-xcode -G Xcode
open build-xcode/Reese.xcodeproj
```

### 2. Configure Debugging

1. Select your plugin target from the scheme dropdown
2. Go to **Product → Scheme → Edit Scheme** 
3. Click **Run** on the left sidebar
4. Under **Executable**, choose **Other** and navigate to executable.

### 3. Build and Run

1. Press **Cmd+B** to build the plugin
2. Press **Cmd+R** to run with AudioPluginHost
4. Load your plugin in AudioPluginHost
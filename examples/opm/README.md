# YM2151 OPM Examples

This directory contains sample code demonstrating the ymfm library's YM2151 (OPM) FM synthesis chip emulation. These examples may be helpful if you're interested in learning OPM programming concepts through practical audio generation.

## Overview

The YM2151 (OPM - FM Operator Type-M) is a 4-operator FM synthesis chip used in arcade boards, the Sharp X68000 computer, and various synthesizers. This collection shows one approach to configuring OPM parameters using the ymfm library to generate different sounds and musical effects.

## What's Included

These examples offer:

- **Audio Output**: Each demo generates WAV files you can listen to
- **Parameter Examples**: Sample register settings and their effects on sound
- **Progressive Examples**: From simple sine waves to multi-channel compositions
- **Reference Code**: Commented source code that may be useful for your own projects

## Demo Programs

The sample generator creates 8 individual WAV files, each demonstrating specific OPM concepts:

### Basic Sound Generation

**01_sine_wave_scale.wav** - Pure sine wave scale
- Basic OPM setup and note frequency calculation
- Simple sine wave timbre (Algorithm 7, single operator)
- KC/KF register programming for pitch control
- A starting point for understanding OPM basics

**02_piano_scale.wav** - Piano-like sound scale  
- FM synthesis with multiple operators
- Algorithm selection and its effect on timbre
- Envelope programming for percussive sounds
- Comparison with sine wave to show FM complexity

**03_twinkle_twinkle_chords.wav** - Multi-channel chord progression
- Four-channel arrangement: melody, bass, and two chord voices
- Different timbres for different musical roles
- Coordinating multiple channels
- Basic harmonic arrangement

### OPM Control Demonstrations

**04_lfo_effects.wav** - LFO (Low Frequency Oscillator) effects
- Comparison of clean sound vs. vibrato vs. tremolo
- Pitch modulation (vibrato) using PMS settings
- Amplitude modulation (tremolo) using AMS settings
- Adding expression to synthesized sounds

**05_fm_algorithms.wav** - FM algorithm comparison
- Side-by-side comparison of Algorithms 0, 1, 4, and 7
- Operator routing and its effect on harmonic content
- How algorithms create different sound characteristics
- FM synthesis fundamentals

**06_envelope_control.wav** - ADSR envelope variations
- Attack, Decay, Sustain, Release envelope stages
- Percussive vs. pad-like envelope settings
- How envelope timing affects musical expression
- Creating different instrument sounds

**07_detune_effects.wav** - Detune for sound thickness
- Progressive detune (DT1) effects
- How slight frequency offsets create chorus and thickness
- Difference between thin and rich sounds
- Creating fuller synthesizer sounds

### Advanced Techniques

**08_stereo_arpeggiator.wav** - Smooth stereo arpeggiator
- 16-second stereo composition with dynamic panning
- Four-channel arpeggiator with different chord progressions
- Smooth note transitions and envelope management
- Stereo positioning and musical arrangement
- **May be best experienced with headphones**

## Building the Examples

### Requirements

- C++17 compatible compiler (GCC, Clang, MSVC)
- ymfm library source files (included in `../../src/`)

### Build Options

#### Using Make

```bash
# From examples/opm directory
make                 # Build the sample generator
make run             # Build and run
make clean           # Remove generated files
```

#### Manual Compilation

```bash
# Compile all source files
g++ -std=c++17 -O2 -Wall -I../../src \
    opm_sample.cpp opm_common.cpp \
    01_sine_wave_scale.cpp 02_piano_scale.cpp 03_twinkle_twinkle_chords.cpp \
    04_lfo_effects.cpp 05_fm_algorithms.cpp 06_envelope_control.cpp \
    07_detune_effects.cpp 08_stereo_arpeggiator.cpp \
    ../../src/ymfm_opm.cpp \
    -o opm_sample
```

## Running the Examples

Execute the compiled program to generate all demo files:

```bash
./opm_sample
```

**Output**: The program generates 8 WAV files (44.1kHz, 16-bit stereo) in the current directory:

- `01_sine_wave_scale.wav` - Simple sine wave demonstration
- `02_piano_scale.wav` - Piano-like FM synthesis
- `03_twinkle_twinkle_chords.wav` - Multi-channel music arrangement
- `04_lfo_effects.wav` - Vibrato and tremolo effects
- `05_fm_algorithms.wav` - Algorithm comparison
- `06_envelope_control.wav` - ADSR envelope variations  
- `07_detune_effects.wav` - Detune and chorus effects
- `08_stereo_arpeggiator.wav` - Advanced stereo composition

**Listening Notes**:
- Headphones may enhance the stereo effect in `08_stereo_arpeggiator.wav`
- Listening in sequence may help understand the progressive complexity
- Comparing similar demos (01 vs 02, different algorithms in 05) can highlight differences

## Code Structure

### Core Files

- **`opm_sample.cpp`** - Main program that orchestrates all demonstrations
- **`opm_common.h/.cpp`** - Shared utilities and timbre definitions
  - Audio utilities (WAV writing, sample rate conversion)
  - Musical utilities (note-to-frequency conversion, OPM frequency calculation)
  - Timbre presets (sine, piano, bass, chord, synth sounds)
  - YM2151 interface implementation

### Individual Demo Files

Each `0X_*.cpp` file contains a focused demonstration:
- Self-contained functions with specific educational goals
- Detailed comments explaining OPM register settings
- Progressive complexity from basic to advanced concepts

### Key Programming Concepts Demonstrated

1. **Register Programming**: Direct YM2151 register manipulation
2. **Timbre Design**: How operator settings create different sounds
3. **Frequency Control**: KC/KF register calculation and usage
4. **Envelope Programming**: ADSR envelope configuration
5. **Multi-channel Coordination**: Managing multiple voices simultaneously
6. **Real-time Parameter Changes**: Dynamic sound modification


## Technical Notes

- **Sample Rate**: YM2151 runs at ~3.579MHz, output is resampled to 44.1kHz
- **Bit Depth**: 16-bit signed stereo output
- **Latency**: Zero-latency direct synthesis (no audio drivers)
- **Compatibility**: Pure C++ implementation, cross-platform compatible

## Related Resources

- **ymfm Documentation**: See `../../README.md` for library overview
- **YM2151 Datasheet**: Official Yamaha documentation for register details
- **FM Synthesis Theory**: Academic resources on frequency modulation
- **Audio Examples**: Listen to generated WAV files for practical understanding

These examples may serve as a reference for understanding the ymfm library and FM synthesis principles using the classic YM2151 sound chip.
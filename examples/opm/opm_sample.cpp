#include "opm_common.h"


int main() {
    // YM2151 clock and sampling rate
    const double YM2151_CLOCK = 3579545.0;  // 3.58MHz
    const double YM2151_OUTPUT_RATE = YM2151_CLOCK / 64.0;  // Actual output rate
    const int SAMPLE_RATE = 44100;  // WAV file sampling rate
    
    // Create OPM instance
    opm_interface intf;
    ymfm::ym2151& chip = intf.m_chip;
    
    std::cout << "=== YM2151 OPM Sample Generator ===" << std::endl;
    std::cout << "Generating individual WAV files for each demonstration...\n" << std::endl;
    
    // First stop sound on all channels
    for (int i = 0; i < 8; i++) {
        chip.write_address(0x08);
        chip.write_data(i);  // Key off all operators
    }
    
    std::vector<int16_t> audioDataRaw;
    
    // Demo 01: Sine wave Do-Re-Mi-Fa-So-La-Ti-Do
    std::cout << "Demo 01: Sine wave scale" << std::endl;
    audioDataRaw.clear();
    playSineWaveScale(chip, audioDataRaw, YM2151_OUTPUT_RATE);
    auto audioData01 = convertSampleRate(audioDataRaw, YM2151_OUTPUT_RATE, SAMPLE_RATE);
    writeWAVFile("01_sine_wave_scale.wav", audioData01);
    
    // Demo 02: Piano sound Do-Re-Mi-Fa-So-La-Ti-Do
    std::cout << "\nDemo 02: Piano sound scale" << std::endl;
    audioDataRaw.clear();
    playPianoScale(chip, audioDataRaw, YM2151_OUTPUT_RATE);
    auto audioData02 = convertSampleRate(audioDataRaw, YM2151_OUTPUT_RATE, SAMPLE_RATE);
    writeWAVFile("02_piano_scale.wav", audioData02);
    
    // Demo 03: Twinkle Twinkle Little Star (with chords)
    std::cout << "\nDemo 03: Twinkle Twinkle Little Star with chords" << std::endl;
    audioDataRaw.clear();
    playTwinkleTwinkle(chip, audioDataRaw, YM2151_OUTPUT_RATE);
    auto audioData03 = convertSampleRate(audioDataRaw, YM2151_OUTPUT_RATE, SAMPLE_RATE);
    writeWAVFile("03_twinkle_twinkle_chords.wav", audioData03);
    
    // Demonstrations for OPM beginners
    std::cout << "\n=== OPM Control Demonstrations ===" << std::endl;
    
    // Demo 04: LFO effects demonstration
    std::cout << "\nDemo 04: LFO effects (vibrato and tremolo)" << std::endl;
    audioDataRaw.clear();
    demonstrateLFO(chip, audioDataRaw, YM2151_OUTPUT_RATE);
    auto audioData04 = convertSampleRate(audioDataRaw, YM2151_OUTPUT_RATE, SAMPLE_RATE);
    writeWAVFile("04_lfo_effects.wav", audioData04);
    
    // Demo 05: FM Algorithm comparison
    std::cout << "\nDemo 05: FM Algorithm comparison" << std::endl;
    audioDataRaw.clear();
    demonstrateAlgorithms(chip, audioDataRaw, YM2151_OUTPUT_RATE);
    auto audioData05 = convertSampleRate(audioDataRaw, YM2151_OUTPUT_RATE, SAMPLE_RATE);
    writeWAVFile("05_fm_algorithms.wav", audioData05);
    
    // Demo 06: Envelope control (ADSR) examples
    std::cout << "\nDemo 06: Envelope control (ADSR)" << std::endl;
    audioDataRaw.clear();
    demonstrateEnvelopeControl(chip, audioDataRaw, YM2151_OUTPUT_RATE);
    auto audioData06 = convertSampleRate(audioDataRaw, YM2151_OUTPUT_RATE, SAMPLE_RATE);
    writeWAVFile("06_envelope_control.wav", audioData06);
    
    // Demo 07: Detune effects for sound thickness
    std::cout << "\nDemo 07: Detune effects" << std::endl;
    audioDataRaw.clear();
    demonstrateDetune(chip, audioDataRaw, YM2151_OUTPUT_RATE);
    auto audioData07 = convertSampleRate(audioDataRaw, YM2151_OUTPUT_RATE, SAMPLE_RATE);
    writeWAVFile("07_detune_effects.wav", audioData07);
    
    // Demo 08: Smooth stereo arpeggiator with dynamic panning
    std::cout << "\nDemo 08: Smooth stereo arpeggiator with dynamic panning" << std::endl;
    audioDataRaw.clear();
    demonstrateStereoArpeggiator(chip, audioDataRaw, YM2151_OUTPUT_RATE);
    auto audioData08 = convertSampleRate(audioDataRaw, YM2151_OUTPUT_RATE, SAMPLE_RATE);
    writeWAVFile("08_stereo_arpeggiator.wav", audioData08);
    
    std::cout << "\n=== All demonstrations completed! ===" << std::endl;
    
    std::cout << "\nGenerated files:" << std::endl;
    std::cout << "  01_sine_wave_scale.wav       - Basic sine wave scale" << std::endl;
    std::cout << "  02_piano_scale.wav           - Piano-like FM synthesis" << std::endl;
    std::cout << "  03_twinkle_twinkle_chords.wav - Multi-channel music with chords" << std::endl;
    std::cout << "  04_lfo_effects.wav           - LFO vibrato and tremolo" << std::endl;
    std::cout << "  05_fm_algorithms.wav         - Comparison of algorithms 0,1,4,7" << std::endl;
    std::cout << "  06_envelope_control.wav      - ADSR envelope variations" << std::endl;
    std::cout << "  07_detune_effects.wav        - Detune for sound thickness" << std::endl;
    std::cout << "  08_stereo_arpeggiator.wav    - Smooth stereo panning arpeggiator" << std::endl;
    std::cout << "\nEach file demonstrates specific OPM concepts for learning!" << std::endl;
    std::cout << "Perfect for understanding YM2151 programming step by step." << std::endl;
    std::cout << "Listen to #8 with headphones for the best stereo panning effect!" << std::endl;
    
    return 0;
}
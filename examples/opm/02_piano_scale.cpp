#include "opm_common.h"

void playPianoScale(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE) {
    std::cout << "\nSwitching to piano-like sound..." << std::endl;
    
    // MIDI note numbers for Do-Re-Mi-Fa-So-La-Ti-Do (from C4)
    const int notes[] = {60, 62, 64, 65, 67, 69, 71, 72};
    const int NOTE_COUNT = 8;
    const double NOTE_DURATION = 60.0 / 120.0 / 2.0;  // Eighth note duration
    const int ch = 0;
    
    setupPianoTimbre(chip, ch);
    
    for (int i = 0; i < NOTE_COUNT; i++) {
        float freq = noteToFreq(notes[i]);
        uint8_t kc, kf;
        freqToOPM(freq, kc, kf);
        
        chip.write_address(0x28 + ch);
        chip.write_data(kc);
        chip.write_address(0x30 + ch);
        chip.write_data(kf << 2);
        
        std::cout << "Piano Note " << i << ": KC=0x" << std::hex << (int)kc << ", KF=0x" << (int)(kf << 2) << std::dec << std::endl;
        
        // Key On
        chip.write_address(0x08);
        chip.write_data(0x78 | ch);
        
        // Generate audio
        int samples_raw = (int)(NOTE_DURATION * YM2151_OUTPUT_RATE);
        for (int s = 0; s < samples_raw; s++) {
            ymfm::ym2151::output_data output;
            chip.generate(&output, 1);
            audioDataRaw.push_back(output.data[0]);
            audioDataRaw.push_back(output.data[1]);
        }
        
        // Key Off
        chip.write_address(0x08);
        chip.write_data(0x00 | ch);
        
        // Short silence period
        int silenceSamples = (int)(0.05 * YM2151_OUTPUT_RATE);
        for (int s = 0; s < silenceSamples; s++) {
            ymfm::ym2151::output_data output;
            chip.generate(&output, 1);
            audioDataRaw.push_back(output.data[0]);
            audioDataRaw.push_back(output.data[1]);
        }
    }
}
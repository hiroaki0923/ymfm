#include "opm_common.h"

void demonstrateLFO(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE) {
    std::cout << "\nDemonstrating LFO (Low Frequency Oscillator) effects..." << std::endl;
    
    const int ch = 0;
    const double DEMO_DURATION = 4.0;  // 4 seconds per demo for clear effect
    const int note = 60;  // C4
    
    // Use setupSineWaveTimbre for consistent behavior
    setupSineWaveTimbre(chip, ch);
    
    // Ensure sustained envelope for all operators
    for (int op = 0; op < 4; op++) {
        int base_addr = op * 8 + ch;
        
        chip.write_address(0xA0 + base_addr);
        chip.write_data(0x00);  // D1R=0 (no decay for sustained sound)
        
        chip.write_address(0xC0 + base_addr);
        chip.write_data(0x00);  // D2R=0
        
        chip.write_address(0xE0 + base_addr);
        chip.write_data(0xF0);  // D1L=15, RR=0 (sustained)
    }
    
    // Set note frequency
    float freq = noteToFreq(note);
    uint8_t kc, kf;
    freqToOPM(freq, kc, kf);
    chip.write_address(0x28 + ch);
    chip.write_data(kc);
    chip.write_address(0x30 + ch);
    chip.write_data(kf << 2);
    
    // Demo 1: No LFO (reference)
    std::cout << "  1. No LFO (reference sound)" << std::endl;
    chip.write_address(0x19);  // LFO Control
    chip.write_data(0x00);     // LFO OFF
    
    chip.write_address(0x08);
    chip.write_data(0x78 | ch);
    
    int samples = (int)(DEMO_DURATION * YM2151_OUTPUT_RATE);
    for (int s = 0; s < samples; s++) {
        ymfm::ym2151::output_data output;
        chip.generate(&output, 1);
        audioDataRaw.push_back(output.data[0]);
        audioDataRaw.push_back(output.data[1]);
    }
    
    chip.write_address(0x08);
    chip.write_data(0x00 | ch);
    
    // Longer gap between demos for clarity
    int gap_samples = (int)(0.5 * YM2151_OUTPUT_RATE);
    for (int s = 0; s < gap_samples; s++) {
        ymfm::ym2151::output_data output;
        chip.generate(&output, 1);
        audioDataRaw.push_back(output.data[0]);
        audioDataRaw.push_back(output.data[1]);
    }
    
    // Demo 2: Vibrato (pitch modulation)
    std::cout << "  2. LFO Vibrato (pitch modulation)" << std::endl;
    
    // Wait briefly for envelope reset
    for (int w = 0; w < 1000; w++) {
        ymfm::ym2151::output_data dummy;
        chip.generate(&dummy, 1);
    }
    
    // Set frequency again (in case it was affected)
    chip.write_address(0x28 + ch);
    chip.write_data(kc);
    chip.write_address(0x30 + ch);
    chip.write_data(kf << 2);
    
    chip.write_address(0x19);  // LFO Control
    chip.write_data(0x02);     // LFO ON, frequency = 2
    
    chip.write_address(0x1A);  // PMD (Pitch Modulation Depth)
    chip.write_data(0x50);     // Medium depth
    
    chip.write_address(0x1B);  // AMD (Amplitude Modulation Depth)
    chip.write_data(0x00);     // No amplitude modulation
    
    // Enable pitch modulation for channel 0
    chip.write_address(0x38 + ch);  // PMS/AMS
    chip.write_data(0x30);          // PMS=3, AMS=0
    
    chip.write_address(0x08);
    chip.write_data(0x78 | ch);
    
    for (int s = 0; s < samples; s++) {
        ymfm::ym2151::output_data output;
        chip.generate(&output, 1);
        audioDataRaw.push_back(output.data[0]);
        audioDataRaw.push_back(output.data[1]);
    }
    
    chip.write_address(0x08);
    chip.write_data(0x00 | ch);
    
    // Gap between demos
    for (int s = 0; s < gap_samples; s++) {
        ymfm::ym2151::output_data output;
        chip.generate(&output, 1);
        audioDataRaw.push_back(output.data[0]);
        audioDataRaw.push_back(output.data[1]);
    }
    
    // Demo 3: Tremolo (amplitude modulation)
    std::cout << "  3. LFO Tremolo (amplitude modulation)" << std::endl;
    
    // Wait briefly for envelope reset
    for (int w = 0; w < 1000; w++) {
        ymfm::ym2151::output_data dummy;
        chip.generate(&dummy, 1);
    }
    
    // Set frequency again
    chip.write_address(0x28 + ch);
    chip.write_data(kc);
    chip.write_address(0x30 + ch);
    chip.write_data(kf << 2);
    
    chip.write_address(0x1A);  // PMD
    chip.write_data(0x00);     // No pitch modulation
    
    chip.write_address(0x1B);  // AMD
    chip.write_data(0x50);     // Medium amplitude modulation
    
    chip.write_address(0x38 + ch);  // PMS/AMS
    chip.write_data(0x02);          // PMS=0, AMS=2
    
    chip.write_address(0x08);
    chip.write_data(0x78 | ch);
    
    for (int s = 0; s < samples; s++) {
        ymfm::ym2151::output_data output;
        chip.generate(&output, 1);
        audioDataRaw.push_back(output.data[0]);
        audioDataRaw.push_back(output.data[1]);
    }
    
    chip.write_address(0x08);
    chip.write_data(0x00 | ch);
    
    // Reset LFO
    chip.write_address(0x19);
    chip.write_data(0x00);
    chip.write_address(0x38 + ch);
    chip.write_data(0x00);
    
    // Final gap for clear separation
    for (int s = 0; s < gap_samples; s++) {
        ymfm::ym2151::output_data output;
        chip.generate(&output, 1);
        audioDataRaw.push_back(output.data[0]);
        audioDataRaw.push_back(output.data[1]);
    }
}
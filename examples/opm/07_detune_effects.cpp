#include "opm_common.h"

void demonstrateDetune(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE) {
    std::cout << "\nDemonstrating detune effects (DT1)..." << std::endl;
    
    const int ch = 0;
    const double DEMO_DURATION = 3.5;  // Longer for clearer effect
    const int note = 60;
    
    // Set algorithm 7 for all operators in parallel
    chip.write_address(0x20 + ch);
    chip.write_data(0xC7);
    
    float freq = noteToFreq(note);
    uint8_t kc, kf;
    freqToOPM(freq, kc, kf);
    chip.write_address(0x28 + ch);
    chip.write_data(kc);
    chip.write_address(0x30 + ch);
    chip.write_data(kf << 2);
    
    // Demo different detune settings with more extreme values
    struct DetuneDemo {
        const char* name;
        uint8_t dt1_values[4];  // DT1 for each operator
        const char* description;
    } demos[] = {
        {"No detune (single tone)", {0, 7, 7, 7}, "Pure reference tone"},
        {"Slight chorus effect", {0, 1, 7, 6}, "Subtle thickness"},
        {"Medium chorus effect", {0, 2, 5, 3}, "Rich harmonic content"},
        {"Strong detuning", {0, 3, 4, 2}, "Very thick, metallic sound"},
        {"Extreme detuning", {1, 2, 3, 4}, "Maximum thickness effect"}
    };
    
    for (int dt_idx = 0; dt_idx < 5; dt_idx++) {
        std::cout << "  " << (dt_idx + 1) << ". " << demos[dt_idx].name 
                  << " (" << demos[dt_idx].description << ")" << std::endl;
        
        // Set detune and volume for each operator
        for (int op = 0; op < 4; op++) {
            int base_addr = op * 8 + ch;
            
            chip.write_address(0x40 + base_addr);
            chip.write_data((demos[dt_idx].dt1_values[op] << 4) | 0x01);  // DT1=detune, MUL=1
            
            chip.write_address(0x60 + base_addr);
            // Adjust volume based on whether operator is active
            if (demos[dt_idx].dt1_values[op] == 7) {
                chip.write_data(127);  // Silent (DT1=7 is off)
            } else {
                chip.write_data(30);   // Audible
            }
            
            chip.write_address(0x80 + base_addr);
            chip.write_data(0x1F);  // AR=31
            
            chip.write_address(0xA0 + base_addr);
            chip.write_data(0x00);  // D1R=0 (sustained)
            
            chip.write_address(0xC0 + base_addr);
            chip.write_data(0x00);  // D2R=0
            
            chip.write_address(0xE0 + base_addr);
            chip.write_data(0xF0);  // D1L=15, RR=0 (sustained)
        }
        
        // Set frequency again to ensure it's set correctly
        chip.write_address(0x28 + ch);
        chip.write_data(kc);
        chip.write_address(0x30 + ch);
        chip.write_data(kf << 2);
        
        // Play note
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
        
        // Longer gap for clearer separation
        int gap_samples = (int)(0.5 * YM2151_OUTPUT_RATE);
        for (int s = 0; s < gap_samples; s++) {
            ymfm::ym2151::output_data output;
            chip.generate(&output, 1);
            audioDataRaw.push_back(output.data[0]);
            audioDataRaw.push_back(output.data[1]);
        }
        
        // Wait for envelope reset
        for (int w = 0; w < 1000; w++) {
            ymfm::ym2151::output_data dummy;
            chip.generate(&dummy, 1);
        }
    }
}
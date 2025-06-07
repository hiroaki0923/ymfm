#include "opm_common.h"

void demonstrateEnvelopeControl(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE) {
    std::cout << "\nDemonstrating envelope control (ADSR)..." << std::endl;
    
    const int ch = 0;
    const double DEMO_DURATION = 3.0;
    const int note = 60;
    
    // Set basic algorithm and frequency
    chip.write_address(0x20 + ch);
    chip.write_data(0xC7);  // Algorithm 7
    
    float freq = noteToFreq(note);
    uint8_t kc, kf;
    freqToOPM(freq, kc, kf);
    chip.write_address(0x28 + ch);
    chip.write_data(kc);
    chip.write_address(0x30 + ch);
    chip.write_data(kf << 2);
    
    // Demo different envelope settings
    struct EnvelopeDemo {
        const char* name;
        uint8_t ar, d1r, d2r, rr, d1l;
        const char* description;
    } demos[] = {
        {"Fast Attack, Fast Decay", 31, 15, 10, 15, 5, "Percussive sound"},
        {"Slow Attack, Slow Decay", 5, 3, 2, 3, 10, "Soft pad sound"},
        {"Medium Attack, No Decay", 15, 0, 0, 7, 15, "Organ-like sustain"},
        {"Very Slow Attack", 1, 5, 3, 5, 8, "Fade-in effect"}
    };
    
    for (int demo_idx = 0; demo_idx < 4; demo_idx++) {
        std::cout << "  " << (demo_idx + 1) << ". " << demos[demo_idx].name 
                  << " (" << demos[demo_idx].description << ")" << std::endl;
        
        // Set envelope for all operators
        for (int op = 0; op < 4; op++) {
            int base_addr = op * 8 + ch;
            
            chip.write_address(0x40 + base_addr);
            chip.write_data(0x01);  // MUL=1
            
            chip.write_address(0x60 + base_addr);
            chip.write_data(30);  // TL=30
            
            chip.write_address(0x80 + base_addr);
            chip.write_data(demos[demo_idx].ar);  // AR
            
            chip.write_address(0xA0 + base_addr);
            chip.write_data(demos[demo_idx].d1r);  // D1R
            
            chip.write_address(0xC0 + base_addr);
            chip.write_data(demos[demo_idx].d2r);  // D2R
            
            chip.write_address(0xE0 + base_addr);
            chip.write_data((demos[demo_idx].d1l << 4) | demos[demo_idx].rr);  // D1L/RR
        }
        
        // Play note for demonstration duration
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
        
        // Wait for full release
        int release_samples = (int)(0.5 * YM2151_OUTPUT_RATE);
        for (int s = 0; s < release_samples; s++) {
            ymfm::ym2151::output_data output;
            chip.generate(&output, 1);
            audioDataRaw.push_back(output.data[0]);
            audioDataRaw.push_back(output.data[1]);
        }
    }
}
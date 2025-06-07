#include "opm_common.h"

void demonstrateAlgorithms(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE) {
    std::cout << "\nDemonstrating different FM Algorithms..." << std::endl;
    
    const int ch = 0;
    const double DEMO_DURATION = 1.5;
    const int note = 60;  // C4
    
    float freq = noteToFreq(note);
    uint8_t kc, kf;
    freqToOPM(freq, kc, kf);
    
    // Algorithm demonstrations
    int algorithms[] = {0, 1, 4, 7};
    const char* alg_names[] = {
        "Algorithm 0 (All operators in series - complex FM)",
        "Algorithm 1 (Mixed series/parallel - rich harmonics)",
        "Algorithm 4 (Two parallel FM pairs - piano-like)",
        "Algorithm 7 (All operators parallel - organ-like)"
    };
    
    for (int alg_idx = 0; alg_idx < 4; alg_idx++) {
        std::cout << "  " << (alg_idx + 1) << ". " << alg_names[alg_idx] << std::endl;
        
        // Set algorithm
        chip.write_address(0x20 + ch);
        chip.write_data(0xC0 | algorithms[alg_idx]);  // L/R both, FB=0, Algorithm
        
        // Configure operators based on algorithm
        for (int op = 0; op < 4; op++) {
            int base_addr = op * 8 + ch;
            
            chip.write_address(0x40 + base_addr);
            chip.write_data(0x01);  // MUL=1
            
            chip.write_address(0x60 + base_addr);
            // Adjust TL based on algorithm and operator role
            int tl;
            if (algorithms[alg_idx] == 0) {
                // Series: only output operator (OP1) should be audible
                tl = (op == 0) ? 20 : 40;
            } else if (algorithms[alg_idx] == 1) {
                tl = (op == 0 || op == 2) ? 25 : 45;
            } else if (algorithms[alg_idx] == 4) {
                tl = (op == 0 || op == 1) ? 25 : 40;
            } else { // Algorithm 7
                tl = 30;  // All operators contribute
            }
            chip.write_data(tl);
            
            chip.write_address(0x80 + base_addr);
            chip.write_data(0x1F);  // AR=31
            
            chip.write_address(0xA0 + base_addr);
            chip.write_data(0x05);  // D1R=5
            
            chip.write_address(0xC0 + base_addr);
            chip.write_data(0x05);  // D2R=5
            
            chip.write_address(0xE0 + base_addr);
            chip.write_data(0xF7);  // D1L=15, RR=7
        }
        
        // Set frequency
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
        
        // Gap between algorithms
        int gap_samples = (int)(0.2 * YM2151_OUTPUT_RATE);
        for (int s = 0; s < gap_samples; s++) {
            ymfm::ym2151::output_data output;
            chip.generate(&output, 1);
            audioDataRaw.push_back(output.data[0]);
            audioDataRaw.push_back(output.data[1]);
        }
    }
}
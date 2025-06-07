#include "opm_common.h"
#include <random>

void setupSmoothSynthTimbre(ymfm::ym2151& chip, int channel) {
    // Ultra-smooth synthesizer sound - minimal artifacts
    chip.write_address(0x20 + channel);
    chip.write_data(0xC1);  // Algorithm=1 (series/parallel mix - smoother than pure parallel)
    
    for (int op = 0; op < 4; op++) {
        int base_addr = op * 8 + channel;
        
        // Very conservative multipliers
        int mul;
        switch (op) {
            case 0: mul = 1; break;   // Output carrier - fundamental only
            case 1: mul = 1; break;   // Secondary carrier - fundamental
            case 2: mul = 1; break;   // Modulator 1 - fundamental
            case 3: mul = 1; break;   // Modulator 2 - fundamental (no high harmonics)
        }
        chip.write_address(0x40 + base_addr);
        chip.write_data(mul);
        
        // Louder but still smooth volume levels
        int tl;
        switch (op) {
            case 0: tl = 15; break;   // Output - loud
            case 1: tl = 20; break;   // Secondary - medium-loud
            case 2: tl = 40; break;   // Modulator 1 - medium
            case 3: tl = 45; break;   // Modulator 2 - medium-quiet
        }
        chip.write_address(0x60 + base_addr);
        chip.write_data(tl);
        
        // Gentle attack to eliminate pops
        chip.write_address(0x80 + base_addr);
        chip.write_data(0x15);  // AR=21 (medium-fast, not instant)
        
        // Smooth decay
        chip.write_address(0xA0 + base_addr);
        chip.write_data(0x05);  // D1R=5
        
        // Gentle sustain decay
        chip.write_address(0xC0 + base_addr);
        chip.write_data(0x03);  // D2R=3
        
        // Smooth release
        chip.write_address(0xE0 + base_addr);
        chip.write_data(0xA8);  // D1L=10, RR=8 (gentle release)
    }
}

void demonstrateStereoArpeggiator(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE) {
    std::cout << "\nDemonstrating smooth stereo arpeggiator with dynamic panning..." << std::endl;
    
    const double DEMO_DURATION = 16.0;  // 16 seconds
    const double NOTE_DURATION = 60.0 / 120.0 / 4.0;  // Sixteenth notes at 120 BPM
    
    // Random number generator for panning
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> pan_dist(0, 3);
    
    // Smoother arpeggiator patterns - staying in comfortable octaves
    const int arp_pattern1[] = {60, 64, 67, 72, 67, 64};  // C major
    const int arp_pattern2[] = {67, 71, 74, 79, 74, 71};  // G major (higher octave)
    const int arp_pattern3[] = {57, 60, 64, 69, 64, 60};  // A minor
    const int arp_pattern4[] = {62, 65, 69, 74, 69, 65};  // D minor
    
    const int* patterns[] = {arp_pattern1, arp_pattern2, arp_pattern3, arp_pattern4};
    const int pattern_lengths[] = {6, 6, 6, 6};
    
    // Setup smooth synth timbres
    for (int ch = 0; ch < 4; ch++) {
        setupSmoothSynthTimbre(chip, ch);
    }
    
    int total_notes = (int)(DEMO_DURATION / NOTE_DURATION);
    std::cout << "Playing " << total_notes << " notes with smooth transitions..." << std::endl;
    
    for (int note_idx = 0; note_idx < total_notes; note_idx++) {
        int ch = note_idx % 4;
        int pattern_idx = note_idx % pattern_lengths[ch];
        int note = patterns[ch][pattern_idx];
        
        // Gentle octave variations
        if ((note_idx / 24) % 2 == 1) note += 12;  // Less frequent octave changes
        if (note > 84) note -= 24;  // Keep in reasonable range
        if (note < 48) note += 12;
        
        // Random panning
        int pan_setting = pan_dist(gen);
        uint8_t rl_setting;
        switch (pan_setting) {
            case 0: rl_setting = 0x80; break;  // L only
            case 1: rl_setting = 0xC0; break;  // L+R (center)
            case 2: rl_setting = 0x40; break;  // R only  
            default: rl_setting = 0xC0; break; // L+R (center)
        }
        
        // Smooth key transitions - no abrupt key offs
        if (note_idx > 0) {
            // Gradual volume fade before changing note (updated for higher base volume)
            for (int fade = 0; fade < 4; fade++) {
                for (int op = 0; op < 4; op++) {
                    int base_addr = op * 8 + ch;
                    chip.write_address(0x60 + base_addr);
                    int current_tl;
                    switch (op) {
                        case 0: current_tl = 15 + fade * 8; break;
                        case 1: current_tl = 20 + fade * 8; break;
                        case 2: current_tl = 40 + fade * 4; break;
                        case 3: current_tl = 45 + fade * 4; break;
                    }
                    chip.write_data(std::min(127, current_tl));
                }
                
                // Generate a few samples during fade
                for (int f = 0; f < 20; f++) {
                    ymfm::ym2151::output_data dummy;
                    chip.generate(&dummy, 1);
                }
            }
            
            // Key off gently
            chip.write_address(0x08);
            chip.write_data(0x00 | ch);
            
            // Extended gap for envelope settling
            for (int w = 0; w < 300; w++) {
                ymfm::ym2151::output_data dummy;
                chip.generate(&dummy, 1);
            }
        }
        
        // Set panning and algorithm
        chip.write_address(0x20 + ch);
        chip.write_data(rl_setting | 0x01);  // Algorithm 1 with panning
        
        // Reset volume levels to normal (updated for higher volume)
        for (int op = 0; op < 4; op++) {
            int base_addr = op * 8 + ch;
            chip.write_address(0x60 + base_addr);
            int tl;
            switch (op) {
                case 0: tl = 15; break;
                case 1: tl = 20; break;
                case 2: tl = 40; break;
                case 3: tl = 45; break;
            }
            chip.write_data(tl);
        }
        
        // Set frequency
        float freq = noteToFreq(note);
        uint8_t kc, kf;
        freqToOPM(freq, kc, kf);
        chip.write_address(0x28 + ch);
        chip.write_data(kc);
        chip.write_address(0x30 + ch);
        chip.write_data(kf << 2);
        
        // Key on
        chip.write_address(0x08);
        chip.write_data(0x78 | ch);
        
        // Debug output
        if (note_idx % 16 == 0) {
            const char* pan_name[] = {"L", "Center", "R", "Center"};
            std::cout << "  Beat " << (note_idx/16 + 1) << ": Ch" << ch 
                      << " note=" << note << " pan=" << pan_name[pan_setting] << std::endl;
        }
        
        // Generate audio
        int samples_per_note = (int)(NOTE_DURATION * YM2151_OUTPUT_RATE);
        for (int s = 0; s < samples_per_note; s++) {
            ymfm::ym2151::output_data output;
            chip.generate(&output, 1);
            audioDataRaw.push_back(output.data[0]);
            audioDataRaw.push_back(output.data[1]);
        }
    }
    
    // Final fade out for all channels
    for (int fade_step = 0; fade_step < 10; fade_step++) {
        for (int ch = 0; ch < 4; ch++) {
            for (int op = 0; op < 4; op++) {
                int base_addr = op * 8 + ch;
                chip.write_address(0x60 + base_addr);
                chip.write_data(127);  // Fade to silence
            }
        }
        
        for (int f = 0; f < 100; f++) {
            ymfm::ym2151::output_data output;
            chip.generate(&output, 1);
            audioDataRaw.push_back(output.data[0]);
            audioDataRaw.push_back(output.data[1]);
        }
    }
    
    // Final key off
    for (int ch = 0; ch < 4; ch++) {
        chip.write_address(0x08);
        chip.write_data(0x00 | ch);
    }
    
    // Extended settling time
    int fade_samples = (int)(1.0 * YM2151_OUTPUT_RATE);
    for (int s = 0; s < fade_samples; s++) {
        ymfm::ym2151::output_data output;
        chip.generate(&output, 1);
        audioDataRaw.push_back(output.data[0]);
        audioDataRaw.push_back(output.data[1]);
    }
    
    std::cout << "Smooth stereo arpeggiator completed - listen with headphones for best effect!" << std::endl;
}
#include "opm_common.h"

// Calculate frequency from note number
float noteToFreq(int note) {
    // A4 = 440Hz, note 69
    return 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
}

// Calculate OPM key code and key fraction from frequency
void freqToOPM(float freq, uint8_t& kc, uint8_t& kf) {
    // Calculate MIDI note number
    float note = 12.0f * log2f(freq / 440.0f) + 69.0f;
    int noteInt = (int)round(note);
    
    // Separate octave and note number
    int octave = (noteInt / 12) - 1;
    int noteInOctave = noteInt % 12;
    
    // YM2151 key code calculation
    // KC = (octave << 4) | note_code
    // Note codes: C=0, C#=1, D=2, D#=4, E=5, F=6, F#=8, G=9, G#=10, A=11, A#=13, B=14
    const uint8_t noteCode[12] = {0, 1, 2, 4, 5, 6, 8, 9, 10, 11, 13, 14};
    kc = ((octave & 0x07) << 4) | noteCode[noteInOctave];
    
    // Key fraction for fine tuning (set to 0 for now)
    kf = 0;
}

// Helper function to write WAV file
void writeWAVFile(const std::string& filename, const std::vector<int16_t>& audioData) {
    WavHeader header;
    header.dataSize = audioData.size() * sizeof(int16_t);
    header.fileSize = header.dataSize + sizeof(WavHeader) - 8;
    
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create WAV file: " << filename << std::endl;
        return;
    }
    
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    file.write(reinterpret_cast<const char*>(audioData.data()), audioData.size() * sizeof(int16_t));
    file.close();
    
    std::cout << "  Generated: " << filename << std::endl;
}

// Helper function to convert raw audio data to target sample rate
std::vector<int16_t> convertSampleRate(const std::vector<int16_t>& audioDataRaw, double sourceRate, int targetRate) {
    std::vector<int16_t> audioData;
    double ratio = sourceRate / targetRate;
    int num_output_samples = (int)(audioDataRaw.size() / 2 / ratio);
    
    for (int i = 0; i < num_output_samples; i++) {
        int src_idx = (int)(i * ratio) * 2;
        if (src_idx + 1 < audioDataRaw.size()) {
            // Simple linear interpolation
            audioData.push_back(audioDataRaw[src_idx]);      // L
            audioData.push_back(audioDataRaw[src_idx + 1]);  // R
        }
    }
    
    return audioData;
}

// Timbre setup functions
void setupSineWaveTimbre(ymfm::ym2151& chip, int channel) {
    // Algorithm 7 (all operators in parallel)
    chip.write_address(0x20 + channel);
    chip.write_data(0xC7);  // L/R both output, FB=0, Algorithm=7
    
    for (int op = 0; op < 4; op++) {
        int base_addr = op * 8 + channel;
        
        chip.write_address(0x40 + base_addr);
        chip.write_data(0x01);  // DT1=0, MUL=1
        
        chip.write_address(0x60 + base_addr);
        chip.write_data(32);  // TL=32 (moderate volume)
        
        chip.write_address(0x80 + base_addr);
        chip.write_data(0x1F);  // KS=0, AR=31
        
        chip.write_address(0xA0 + base_addr);
        chip.write_data(0x00);  // AMS-EN=0, D1R=0
        
        chip.write_address(0xC0 + base_addr);
        chip.write_data(0x00);  // DT2=0, D2R=0
        
        chip.write_address(0xE0 + base_addr);
        chip.write_data(0xF7);  // D1L=15, RR=7
    }
}

void setupPianoTimbre(ymfm::ym2151& chip, int channel) {
    // Algorithm 4 (piano-like)
    chip.write_address(0x20 + channel);
    chip.write_data(0xC4 | (3 << 3));  // L/R both output, FB=3, Algorithm=4
    
    for (int op = 0; op < 4; op++) {
        int base_addr = op * 8 + channel;
        
        // DT1/MUL
        int mul = (op == 0 || op == 1) ? 1 : 2;  // Carriers x1, modulators x2
        chip.write_address(0x40 + base_addr);
        chip.write_data(mul);  // DT1=0, MUL=mul
        
        // TL - Volume adjustment for each operator
        int tl;
        if (op == 0) tl = 24;      // Carrier 1
        else if (op == 1) tl = 28; // Carrier 2
        else if (op == 2) tl = 40; // Modulator 1
        else tl = 45;              // Modulator 2
        chip.write_address(0x60 + base_addr);
        chip.write_data(tl);
        
        chip.write_address(0x80 + base_addr);
        chip.write_data(0x1F);  // KS=0, AR=31
        
        chip.write_address(0xA0 + base_addr);
        chip.write_data((op < 2) ? 0x05 : 0x0A);  // Fast decay for carriers
        
        chip.write_address(0xC0 + base_addr);
        chip.write_data((op < 2) ? 0x02 : 0x00);
        
        chip.write_address(0xE0 + base_addr);
        chip.write_data((op < 2) ? 0x28 : 0xF5);  // Fast release for carriers
    }
}

void setupBassTimbre(ymfm::ym2151& chip, int channel) {
    // Simple and stable bass sound
    chip.write_address(0x20 + channel);
    chip.write_data(0xC0);  // FB=0, Algorithm=0 (simple FM)
    
    for (int op = 0; op < 4; op++) {
        int base_addr = op * 8 + channel;
        chip.write_address(0x40 + base_addr);
        chip.write_data(0x01);  // MUL=1 (all base frequency)
        
        chip.write_address(0x60 + base_addr);
        chip.write_data((op == 0) ? 15 : 60);  // Carrier loud, others quiet
        
        chip.write_address(0x80 + base_addr);
        chip.write_data(0x1F);  // AR=31
        
        chip.write_address(0xA0 + base_addr);
        chip.write_data(0x05);  // D1R=5
        
        chip.write_address(0xC0 + base_addr);
        chip.write_data(0x05);  // D2R=5
        
        chip.write_address(0xE0 + base_addr);
        chip.write_data(0xF8);  // D1L=15, RR=8
    }
}

void setupChordTimbre(ymfm::ym2151& chip, int channel) {
    // Organ-style chords
    chip.write_address(0x20 + channel);
    chip.write_data(0xC7);  // Algorithm=7 (all operators parallel)
    
    for (int op = 0; op < 4; op++) {
        int base_addr = op * 8 + channel;
        chip.write_address(0x40 + base_addr);
        chip.write_data(0x01);  // MUL=1
        
        chip.write_address(0x60 + base_addr);
        chip.write_data(45);  // TL=45 (chords kept moderate)
        
        chip.write_address(0x80 + base_addr);
        chip.write_data(0x1A);  // AR=26
        
        chip.write_address(0xA0 + base_addr);
        chip.write_data(0x03);  // D1R=3
        
        chip.write_address(0xC0 + base_addr);
        chip.write_data(0x00);  // D2R=0
        
        chip.write_address(0xE0 + base_addr);
        chip.write_data(0xF3);  // D1L=15, RR=3
    }
}

void setupSynthTimbre(ymfm::ym2151& chip, int channel) {
    // Smooth synthesizer lead sound - reduced artifacts
    chip.write_address(0x20 + channel);
    chip.write_data(0xC4);  // Algorithm=4 (cleaner than 7, less harsh harmonics)
    
    for (int op = 0; op < 4; op++) {
        int base_addr = op * 8 + channel;
        
        // Conservative multipliers to avoid aliasing
        int mul;
        switch (op) {
            case 0: mul = 1; break;   // Carrier 1 - fundamental
            case 1: mul = 1; break;   // Carrier 2 - fundamental  
            case 2: mul = 1; break;   // Modulator 1 - fundamental
            case 3: mul = 2; break;   // Modulator 2 - octave only
        }
        chip.write_address(0x40 + base_addr);
        chip.write_data(mul);  // DT1=0, MUL=mul
        
        // Balanced volume levels - carriers audible, modulators moderate
        int tl;
        switch (op) {
            case 0: tl = 25; break;   // Carrier 1 - moderate
            case 1: tl = 30; break;   // Carrier 2 - slightly quieter
            case 2: tl = 50; break;   // Modulator 1 - quiet
            case 3: tl = 55; break;   // Modulator 2 - very quiet
        }
        chip.write_address(0x60 + base_addr);
        chip.write_data(tl);
        
        // Smoother attack to reduce pops
        chip.write_address(0x80 + base_addr);
        chip.write_data(0x1C);  // AR=28 (fast but not instant)
        
        // Gradual decay
        chip.write_address(0xA0 + base_addr);
        chip.write_data(0x06);  // D1R=6
        
        // Moderate sustain decay
        chip.write_address(0xC0 + base_addr);
        chip.write_data(0x04);  // D2R=4
        
        // Moderate release to avoid cut-off pops
        chip.write_address(0xE0 + base_addr);
        chip.write_data(0x8C);  // D1L=8, RR=12 (moderate release)
    }
}
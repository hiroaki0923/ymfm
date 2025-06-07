#include "opm_common.h"

void playTwinkleTwinkle(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE) {
    std::cout << "\nPlaying Twinkle Twinkle Little Star with chords..." << std::endl;
    
    // Twinkle Twinkle Little Star melody (MIDI note numbers)
    const int melody[] = {
        60, 60, 67, 67, 69, 69, 67, // Twinkle twinkle little star
        65, 65, 64, 64, 62, 62, 60, // How I wonder what you are
        67, 67, 65, 65, 64, 64, 62, // Up above the world so high
        67, 67, 65, 65, 64, 64, 62, // Like a diamond in the sky
        60, 60, 67, 67, 69, 69, 67, // Twinkle twinkle little star
        65, 65, 64, 64, 62, 62, 60  // How I wonder what you are
    };
    
    // Bass line (walking bass style)
    const int bass[] = {
        48, 48, 48, 50, 53, 53, 48, // C C C D F F C
        53, 53, 55, 55, 55, 55, 48, // F F G G G G C
        48, 52, 53, 53, 55, 55, 55, // C E F F G G G
        48, 52, 53, 53, 55, 55, 55, // C E F F G G G
        48, 48, 48, 50, 53, 53, 48, // C C C D F F C
        53, 53, 55, 55, 55, 50, 48  // F F G G G D C
    };
    
    // Chords (simple triads)
    const int chord1[] = {
        52, 52, 52, 52, 57, 57, 52, // E E E E A A E (3rd of C, 3rd of F, 3rd of C)
        57, 57, 59, 59, 59, 59, 52, // A A B B B B E (3rd of F, 3rd of G, 3rd of C)
        52, 52, 57, 57, 59, 59, 59, // E E A A B B B
        52, 52, 57, 57, 59, 59, 59, // E E A A B B B
        52, 52, 52, 52, 57, 57, 52, // E E E E A A E
        57, 57, 59, 59, 59, 59, 52  // A A B B B B E
    };
    
    const int chord2[] = {
        55, 55, 55, 55, 60, 60, 55, // G G G G C C G (5th of C, root of F, 5th of C)
        60, 60, 62, 62, 62, 62, 55, // C C D D D D G (root of F, 5th of G, 5th of C)
        55, 55, 60, 60, 62, 62, 62, // G G C C D D D
        55, 55, 60, 60, 62, 62, 62, // G G C C D D D
        55, 55, 55, 55, 60, 60, 55, // G G G G C C G
        60, 60, 62, 62, 62, 62, 55  // C C D D D D G
    };
    
    const int TWINKLE_NOTES = 42;
    const double QUARTER_NOTE = 60.0 / 120.0;  // Quarter note duration
    const double HALF_NOTE = QUARTER_NOTE * 2;  // Half note duration
    
    // Twinkle Twinkle note durations (quarter note=1, half note=2)
    const double durations[] = {
        1, 1, 1, 1, 1, 1, 2, // Twinkle twinkle little star (last note is half note)
        1, 1, 1, 1, 1, 1, 2, // How I wonder what you are
        1, 1, 1, 1, 1, 1, 2, // Up above the world so high
        1, 1, 1, 1, 1, 1, 2, // Like a diamond in the sky
        1, 1, 1, 1, 1, 1, 2, // Twinkle twinkle little star
        1, 1, 1, 1, 1, 1, 2  // How I wonder what you are
    };
    
    // Set up timbres for all channels
    setupPianoTimbre(chip, 0);  // Melody
    setupBassTimbre(chip, 1);   // Bass
    setupChordTimbre(chip, 2);  // Chord 1
    setupChordTimbre(chip, 3);  // Chord 2
    
    // Play Twinkle Twinkle Little Star
    for (int i = 0; i < TWINKLE_NOTES; i++) {
        // Set pitch for each channel
        int notes_to_play[4] = {melody[i], bass[i], chord1[i], chord2[i]};
        
        if (i % 7 == 0) {  // Log output at the beginning of each phrase
            std::cout << "Playing phrase starting at note " << i << ", duration=" << durations[i] << std::endl;
        }
        
        // Key off all channels (individually for each channel)
        for (int ch = 0; ch < 4; ch++) {
            chip.write_address(0x08);
            chip.write_data(0x00 | ch);  // All operators OFF
        }
        
        // Wait a bit to reset envelope
        for (int w = 0; w < 100; w++) {
            ymfm::ym2151::output_data dummy;
            chip.generate(&dummy, 1);
        }
        
        // Set pitch and key on for each channel
        for (int ch = 0; ch < 4; ch++) {
            if (notes_to_play[ch] > 0) {
                float freq = noteToFreq(notes_to_play[ch]);
                uint8_t kc, kf;
                freqToOPM(freq, kc, kf);
                
                // KC/KF setup
                chip.write_address(0x28 + ch);
                chip.write_data(kc);
                chip.write_address(0x30 + ch);
                chip.write_data(kf << 2);
                
                // Key on
                chip.write_address(0x08);
                chip.write_data(0x78 | ch);  // All operators ON
            }
        }
        
        // Generate for note duration (according to rhythm)
        double note_duration = (durations[i] == 2) ? HALF_NOTE : QUARTER_NOTE;
        int samples_raw = (int)(note_duration * YM2151_OUTPUT_RATE);
        for (int s = 0; s < samples_raw; s++) {
            ymfm::ym2151::output_data output;
            chip.generate(&output, 1);
            audioDataRaw.push_back(output.data[0]);
            audioDataRaw.push_back(output.data[1]);
        }
        
        // Short gap
        int gap_samples = (int)(0.02 * YM2151_OUTPUT_RATE);
        for (int s = 0; s < gap_samples; s++) {
            ymfm::ym2151::output_data output;
            chip.generate(&output, 1);
            audioDataRaw.push_back(output.data[0]);
            audioDataRaw.push_back(output.data[1]);
        }
    }
    
    // Key off all channels (final)
    for (int ch = 0; ch < 4; ch++) {
        chip.write_address(0x08);
        chip.write_data(0x00 | ch);
    }
}
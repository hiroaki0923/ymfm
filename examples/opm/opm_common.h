#ifndef OPM_COMMON_H
#define OPM_COMMON_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdint>

// ymfm includes
#include "ymfm_opm.h"

// WAV file header structure
struct WavHeader {
    char riff[4] = {'R', 'I', 'F', 'F'};
    uint32_t fileSize;
    char wave[4] = {'W', 'A', 'V', 'E'};
    char fmt[4] = {'f', 'm', 't', ' '};
    uint32_t fmtSize = 16;
    uint16_t format = 1;
    uint16_t channels = 2;
    uint32_t sampleRate = 44100;
    uint32_t byteRate = 44100 * 2 * 2;
    uint16_t blockAlign = 4;
    uint16_t bitsPerSample = 16;
    char data[4] = {'d', 'a', 't', 'a'};
    uint32_t dataSize;
};

// OPM interface implementation
class opm_interface : public ymfm::ymfm_interface {
public:
    // Constructor
    opm_interface() : m_chip(*this) {
        m_chip.reset();
    }
    
    // Interface methods
    virtual void ymfm_sync_mode_write(uint8_t data) override {}
    virtual void ymfm_sync_check_interrupts() override {}
    virtual void ymfm_set_timer(uint32_t tnum, int32_t duration_in_clocks) override {}
    virtual void ymfm_set_busy_end(uint32_t clocks) override {}
    virtual void ymfm_update_irq(bool asserted) override {}
    
    // Chip instance
    ymfm::ym2151 m_chip;
};

// Function declarations
float noteToFreq(int note);
void freqToOPM(float freq, uint8_t& kc, uint8_t& kf);
void writeWAVFile(const std::string& filename, const std::vector<int16_t>& audioData);
std::vector<int16_t> convertSampleRate(const std::vector<int16_t>& audioDataRaw, double sourceRate, int targetRate);

// Timbre setup functions
void setupSineWaveTimbre(ymfm::ym2151& chip, int channel);
void setupPianoTimbre(ymfm::ym2151& chip, int channel);
void setupBassTimbre(ymfm::ym2151& chip, int channel);
void setupChordTimbre(ymfm::ym2151& chip, int channel);
void setupSynthTimbre(ymfm::ym2151& chip, int channel);

// Educational demonstration functions
void demonstrateLFO(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE);
void demonstrateAlgorithms(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE);
void demonstrateEnvelopeControl(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE);
void demonstrateDetune(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE);
void demonstrateStereoArpeggiator(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE);

// Music playback functions
void playSineWaveScale(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE);
void playPianoScale(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE);
void playTwinkleTwinkle(ymfm::ym2151& chip, std::vector<int16_t>& audioDataRaw, double YM2151_OUTPUT_RATE);

#endif // OPM_COMMON_H
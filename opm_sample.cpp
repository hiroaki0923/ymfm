#include <iostream>
#include <vector>
#include <cmath>
#include <fstream> // Required for file output
#include <chrono>  // Required for sleep
#include <thread>  // Required for sleep

// ymfm library (headers copied into ymfm_lib directory)
#include "ymfm_lib/ymfm.h"
#include "ymfm_lib/ymfm_opm.h"


// Define note frequencies (e.g., C4, D4, etc.)
// Frequencies for OPM (KC codes) need to be calculated based on a reference
// For YM2151, F-Number = Freq * 2^20 / (Clock / 64)
// Assuming a 3.579545 MHz clock for YM2151
const double YM2151_CLOCK = 3579545.0;

// Note definitions (using C4 as middle C for this example scale)
const double C4_FREQ = 261.63;
const double D4_FREQ = 293.66;
const double E4_FREQ = 329.63;
const double F4_FREQ = 349.23;
const double G4_FREQ = 392.00;
const double A4_FREQ = 440.00;
const double B4_FREQ = 493.88;
const double C5_FREQ = 523.25;


// Define BPM and note duration
const double BPM = 120.0;
const double EIGHTH_NOTE_DURATION = 60.0 / BPM / 2.0; // Duration in seconds
const int SAMPLE_RATE = 44100; // Standard sample rate for audio

struct Note {
    double frequency; // Hz
    double duration;  // seconds
    uint8_t opm_key_code; // KC value for OPM
    uint8_t opm_key_fraction; // KF value for OPM (upper 6 bits of F-Num's fractional part)
};

// Basic WAV file header structure
struct WavHeader {
    char riff_header[4] = {'R', 'I', 'F', 'F'};
    int wav_size = 0;
    char wave_header[4] = {'W', 'A', 'V', 'E'};
    char fmt_header[4] = {'f', 'm', 't', ' '};
    int fmt_chunk_size = 16;
    short audio_format = 1; // PCM
    short num_channels = 1; // Mono (ymfm outputs stereo, we'll mix or take one)
    int sample_rate = SAMPLE_RATE;
    int byte_rate = SAMPLE_RATE * sizeof(short) * 1; // For mono
    short block_align = sizeof(short) * 1; // For mono
    short bits_per_sample = 16;
    char data_header[4] = {'d', 'a', 't', 'a'};
    int data_chunk_size = 0;
};

// Function to write a WAV file
void write_wav_file(const std::string& filename, const std::vector<short>& audio_data, int num_channels_file, int sample_rate_file) {
    std::ofstream out_file(filename, std::ios::binary);
    if (!out_file) {
        std::cerr << "Error: Could not open WAV file for writing." << std::endl;
        return;
    }

    WavHeader header;
    header.num_channels = num_channels_file; // Use parameter
    header.sample_rate = sample_rate_file;   // Use parameter
    header.bits_per_sample = 16;
    header.byte_rate = sample_rate_file * header.num_channels * (header.bits_per_sample / 8);
    header.block_align = header.num_channels * (header.bits_per_sample / 8);
    header.data_chunk_size = audio_data.size() * sizeof(short);
    // Standard WAV header size is 36 bytes before data_chunk_size for PCM
    header.wav_size = 36 + header.data_chunk_size;


    out_file.write(reinterpret_cast<const char*>(&header), sizeof(WavHeader)); // sizeof(WavHeader) should be 44 for this struct.
                                                                              // Let's write only the parts up to data_chunk_size if there's padding.
                                                                              // More robust: write members individually or use a packed struct.
                                                                              // For now, assuming this struct is packed and size is 44.
    // Check actual size of header to be safe.
    // std::cout << "DEBUG: Sizeof WavHeader: " << sizeof(WavHeader) << std::endl; // Should be 44

    out_file.write(reinterpret_cast<const char*>(&header.riff_header), 4);
    out_file.write(reinterpret_cast<const char*>(&header.wav_size), 4);
    out_file.write(reinterpret_cast<const char*>(&header.wave_header), 4);
    out_file.write(reinterpret_cast<const char*>(&header.fmt_header), 4);
    out_file.write(reinterpret_cast<const char*>(&header.fmt_chunk_size), 4);
    out_file.write(reinterpret_cast<const char*>(&header.audio_format), 2);
    out_file.write(reinterpret_cast<const char*>(&header.num_channels), 2);
    out_file.write(reinterpret_cast<const char*>(&header.sample_rate), 4);
    out_file.write(reinterpret_cast<const char*>(&header.byte_rate), 4);
    out_file.write(reinterpret_cast<const char*>(&header.block_align), 2);
    out_file.write(reinterpret_cast<const char*>(&header.bits_per_sample), 2);
    out_file.write(reinterpret_cast<const char*>(&header.data_header), 4);
    out_file.write(reinterpret_cast<const char*>(&header.data_chunk_size), 4);

    out_file.write(reinterpret_cast<const char*>(audio_data.data()), header.data_chunk_size);

    std::cout << "WAV file generated: " << filename << std::endl;
}

// Dummy ymfm_interface implementation
class my_ymfm_interface : public ymfm::ymfm_interface
{
public:
    // constructor
    my_ymfm_interface() : m_chip(*this) { m_chip.reset(); }

    // ymfm_interface overrides
    virtual void ymfm_sync_mode_write(uint8_t data) override {} // For YM2149 primarily (or other chips needing sync mode)
    virtual void ymfm_set_timer(uint32_t tnum, int32_t duration_in_clocks) override {} // Corrected signature
    virtual void ymfm_set_busy_end(uint32_t clocks) override {}
    virtual void ymfm_update_irq(bool asserted) override {}


    // provide a reference to our chip an public member
    ymfm::ym2151 m_chip;
};


// Note: This function defines a very basic FM piano-like patch for the YM2151.
// It uses a 4-operator configuration with specific envelope and modulation settings.
// This patch is intended as a simple starting point and would require significant
// tuning (adjusting Total Level, Attack/Decay/Sustain/Release rates, Multipliers, etc.)
// and potentially different algorithms or feedback settings to achieve a more
// realistic or richer piano sound. FM synthesis offers vast sonic possibilities through these parameters.
void set_piano_patch(ymfm::ym2151& chip, uint8_t chan_offset) {
    // Register values for a basic piano sound (algorithm 5 (0x04), FB=7)
    // Pan Left & Right, Feedback 7, Algorithm 4
    chip.write(0x20 + chan_offset, 0xc0 | (7 << 3) | 4);

    // Operator parameters (using offsets for channel `chan_offset`)
    // Operator 1 (Modulator)
    chip.write(0x40 + chan_offset, 0x01); // DT1=0, MUL=1
    chip.write(0x60 + chan_offset, 40);   // TL (Total Level) = 40
    chip.write(0x80 + chan_offset, 0x1f); // KS=0, AR=31 (fast attack)
    chip.write(0xA0 + chan_offset, 0x0f); // AM=0, D1R=15 (fast decay)
    chip.write(0xC0 + chan_offset, 0x0f); // DT2=0, D2R=15
    chip.write(0xE0 + chan_offset, (5 << 4) | 5); // SL=5 , RR=5

    // Operator 2 (Modulator)
    chip.write(0x48 + chan_offset, 0x01);
    chip.write(0x68 + chan_offset, 30);
    chip.write(0x88 + chan_offset, 0x1f);
    chip.write(0xA8 + chan_offset, 0x0a);
    chip.write(0xC8 + chan_offset, 0x0f);
    chip.write(0xE8 + chan_offset, (3 << 4) | 7);

    // Operator 3 (Modulator)
    chip.write(0x50 + chan_offset, 0x01);
    chip.write(0x70 + chan_offset, 20);
    chip.write(0x90 + chan_offset, 0x1f);
    chip.write(0xB0 + chan_offset, 0x07);
    chip.write(0xD0 + chan_offset, 0x0f);
    chip.write(0xF0 + chan_offset, (2 << 4) | 7);

    // Operator 4 (Carrier)
    chip.write(0x58 + chan_offset, 0x00); // MUL=0 (fundamental)
    chip.write(0x78 + chan_offset, 0);    // TL=0 (loudest)
    chip.write(0x98 + chan_offset, 0x1f);
    chip.write(0xB8 + chan_offset, 0x05);
    chip.write(0xD8 + chan_offset, 0x0f);
    chip.write(0xF8 + chan_offset, (1 << 4) | 7);
}


int main() {
    std::cout << "OPM Sample Program - YM2151 Piano" << std::endl;

    // Note: The KC (Key Code) and KF (Key Fraction) values used below for the YM2151
    // are pre-defined approximations for a standard C-Major scale.
    // A more accurate and flexible method would involve direct calculation
    // from frequency using YM2151's specific formulas for F-Number and Block,
    // which can be found in its datasheet or technical manuals. This often involves
    // more complex logarithmic scaling or lookup tables for precise tuning.
    std::vector<Note> sequence_def = {
        {C4_FREQ, EIGHTH_NOTE_DURATION}, {D4_FREQ, EIGHTH_NOTE_DURATION},
        {E4_FREQ, EIGHTH_NOTE_DURATION}, {F4_FREQ, EIGHTH_NOTE_DURATION},
        {G4_FREQ, EIGHTH_NOTE_DURATION}, {A4_FREQ, EIGHTH_NOTE_DURATION},
        {B4_FREQ, EIGHTH_NOTE_DURATION}, {C5_FREQ, EIGHTH_NOTE_DURATION},
    };

    std::vector<Note> sequence;
    for (const auto& note_def : sequence_def) {
        Note n = note_def;
        // Pre-defined KC/KF for YM2151 C-Major Scale starting C4
        if (abs(n.frequency - C4_FREQ) < 1) { n.opm_key_code = 0x40; n.opm_key_fraction = 0x30; } // C4
        else if (abs(n.frequency - D4_FREQ) < 1) { n.opm_key_code = 0x42; n.opm_key_fraction = 0x34; } // D4
        else if (abs(n.frequency - E4_FREQ) < 1) { n.opm_key_code = 0x44; n.opm_key_fraction = 0x00; } // E4
        else if (abs(n.frequency - F4_FREQ) < 1) { n.opm_key_code = 0x45; n.opm_key_fraction = 0x2C; } // F4
        else if (abs(n.frequency - G4_FREQ) < 1) { n.opm_key_code = 0x47; n.opm_key_fraction = 0x28; } // G4
        else if (abs(n.frequency - A4_FREQ) < 1) { n.opm_key_code = 0x49; n.opm_key_fraction = 0x00; } // A4
        else if (abs(n.frequency - B4_FREQ) < 1) { n.opm_key_code = 0x4B; n.opm_key_fraction = 0x0C; } // B4
        else if (abs(n.frequency - C5_FREQ) < 1) { n.opm_key_code = 0x50; n.opm_key_fraction = 0x30; } // C5
        else {
            n.opm_key_code = 0x40; n.opm_key_fraction = 0x30; // Default to C4
            std::cerr << "Warning: Frequency " << n.frequency << " not mapped, using C4." << std::endl;
        }
        sequence.push_back(n);
    }

    my_ymfm_interface intf;
    ymfm::ym2151& opm_chip = intf.m_chip;

    opm_chip.reset(); // Ensure chip is in a known state

    const double YM2151_OUTPUT_RATE = YM2151_CLOCK / 64.0;
    std::vector<short> audio_buffer_raw;

    uint8_t current_channel_offset = 0; // Use channel 0 (actual channel registers 0-7)
    set_piano_patch(opm_chip, current_channel_offset);

    for (const auto& note : sequence) {
        std::cout << "Processing note: Freq " << note.frequency << " (KC:0x" << std::hex << (int)note.opm_key_code << ", KF:0x" << (int)note.opm_key_fraction << std::dec << ") for " << note.duration << "s" << std::endl;

        opm_chip.write(0x28 + current_channel_offset, note.opm_key_code);
        opm_chip.write(0x30 + current_channel_offset, note.opm_key_fraction);

        // Key On for channel (channel index 0-7 for YM2151)
        // Register 0x08: Key On/Off. Bits 2-0 select channel. Bits 6-3 select operators (D1-D4).
        // KON_D4_D3_D2_D1_CH2_CH1_CH0. For all 4 ops on, use 1111b = 0xF.
        // So, (0x0F << 3) | channel_index
        opm_chip.write(0x08, (0x0F << 3) | current_channel_offset);

        int samples_for_note_raw = static_cast<int>(note.duration * YM2151_OUTPUT_RATE);
        for (int i = 0; i < samples_for_note_raw; ++i) {
            ymfm::ym2151::output_data out;
            opm_chip.generate(&out, 1);
            // Scale factor: ymfm output for FM chips is often in a range of +/- 8000 to +/- 16000.
            // Max output of ym2151::output_data is an int32_t that is effectively 14-bit data.
            // Let's assume a max of around 16383. Scaling by 2 gives approx 32766.
            audio_buffer_raw.push_back(static_cast<short>(out.data[0] * 2));
            audio_buffer_raw.push_back(static_cast<short>(out.data[1] * 2));
        }

        opm_chip.write(0x08, (0x00 << 3) | current_channel_offset); // Key Off for current_channel_offset
    }

    std::vector<short> audio_buffer_final;
    if (YM2151_OUTPUT_RATE == 0) {
        std::cerr << "Error: YM2151_OUTPUT_RATE is zero, cannot resample." << std::endl;
        write_wav_file("output.wav", audio_buffer_final, 1, SAMPLE_RATE); // Write empty or error file
        return 1;
    }
    double ratio = YM2151_OUTPUT_RATE / SAMPLE_RATE;

    if (ratio == 0) {
         std::cerr << "Error: Resampling ratio is zero." << std::endl;
        write_wav_file("output.wav", audio_buffer_final, 1, SAMPLE_RATE);
        return 1;
    }

    int num_output_samples = static_cast<int>(static_cast<double>(audio_buffer_raw.size() / 2) / ratio);
    audio_buffer_final.reserve(num_output_samples);

    for (int i = 0; i < num_output_samples; ++i) {
        double raw_sample_index_double = static_cast<double>(i) * ratio;
        int raw_sample_index_floor = static_cast<int>(raw_sample_index_double);

        size_t actual_raw_idx_left = static_cast<size_t>(raw_sample_index_floor * 2);
        size_t actual_raw_idx_right = actual_raw_idx_left + 1;

        if (actual_raw_idx_right < audio_buffer_raw.size()) {
            short left = audio_buffer_raw[actual_raw_idx_left];
            short right = audio_buffer_raw[actual_raw_idx_right];
            audio_buffer_final.push_back(static_cast<short>((static_cast<double>(left) + static_cast<double>(right)) / 2.0));
        } else if (actual_raw_idx_left < audio_buffer_raw.size()) { // Should not happen with stereo pairs
             audio_buffer_final.push_back(static_cast<short>(static_cast<double>(audio_buffer_raw[actual_raw_idx_left]) / 2.0));
        } else {
            if (i < num_output_samples) {
                 audio_buffer_final.push_back(0);
            }
        }
    }

    if (audio_buffer_final.empty() && !audio_buffer_raw.empty()) {
        std::cerr << "Warning: Final audio buffer is empty after resampling. Raw buffer had " << audio_buffer_raw.size() << " samples." << std::endl;
        for(size_t i = 0; (i*2 + 1) < audio_buffer_raw.size() && i < (size_t)SAMPLE_RATE * 5; ++i) {
            audio_buffer_final.push_back((audio_buffer_raw[i*2] + audio_buffer_raw[i*2+1])/2);
        }
    }

    write_wav_file("output.wav", audio_buffer_final, 1, SAMPLE_RATE);

    return 0;
}

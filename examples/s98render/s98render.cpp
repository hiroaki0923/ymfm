
//   clang++ --std=c++17 -liconv -I../../src s98render.cpp s98file.cpp ../../src/ymfm_misc.cpp ../../src/ymfm_opl.cpp ../../src/ymfm_opm.cpp ../../src/ymfm_opn.cpp ../../src/ymfm_adpcm.cpp ../../src/ymfm_pcm.cpp ../../src/ymfm_ssg.cpp -o s98render.exe


#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <list>
#include <string>

#include "ymfm_misc.h"
#include "ymfm_opl.h"
#include "ymfm_opm.h"
#include "ymfm_opn.h"

#include "s98file.hpp"

#define LOG_WRITES (0)

// enable this to capture each chip at its native rate as well
#define CAPTURE_NATIVE (0 || RUN_NUKED_OPN2)



//*********************************************************
//  GLOBAL TYPES
//*********************************************************

// we use an int64_t as emulated time, as a 32.32 fixed point value
using emulated_time = int64_t;

// enumeration of the different types of chips we support
enum chip_type
{
	CHIP_YM2149,
	CHIP_YM2151,
	CHIP_YM2203,
	CHIP_YM2413,
	CHIP_YM2608,
	CHIP_YM2610,
	CHIP_YM2612,
	CHIP_YM3526,
	CHIP_Y8950,
	CHIP_YM3812,
	CHIP_YMF262,
	CHIP_YMF278B,
	CHIP_TYPES
};



//*********************************************************
//  CLASSES
//*********************************************************

// ======================> vgm_chip_base

// abstract base class for a Yamaha chip; we keep a list of these for processing
// as new commands come in
class vgm_chip_base
{
public:
	// construction
	vgm_chip_base(uint32_t clock, chip_type type, char const *name) :
		m_type(type),
		m_name(name)
	{
	}

	// simple getters
	chip_type type() const { return m_type; }
	virtual uint32_t sample_rate() const = 0;

	// required methods for derived classes to implement
	virtual void write(uint32_t reg, uint8_t data) = 0;
	virtual void generate(emulated_time output_start, emulated_time output_step, int32_t *buffer) = 0;

	// write data to the ADPCM-A buffer
	void write_data(ymfm::access_class type, uint32_t base, uint32_t length, uint8_t const *src)
	{
		uint32_t end = base + length;
		if (end > m_data[type].size())
			m_data[type].resize(end);
		memcpy(&m_data[type][base], src, length);
	}

	// seek within the PCM stream
	void seek_pcm(uint32_t pos) { m_pcm_offset = pos; }
	uint8_t read_pcm() { auto &pcm = m_data[ymfm::ACCESS_PCM]; return (m_pcm_offset < pcm.size()) ? pcm[m_pcm_offset++] : 0; }
    
    void set_ssg_volume(double ratio){ssgvol = ratio;}

protected:
	// internal state
	chip_type m_type;
	std::string m_name;
	std::vector<uint8_t> m_data[ymfm::ACCESS_CLASSES];
	uint32_t m_pcm_offset;

    double ssgvol = 1;
};


// ======================> vgm_chip

// actual chip-specific implementation class; includes implementatino of the
// ymfm_interface as needed for vgmplay purposes
template<typename ChipType>
class vgm_chip : public vgm_chip_base, public ymfm::ymfm_interface
{
public:
	// construction
	vgm_chip(uint32_t clock, chip_type type, char const *name) :
		vgm_chip_base(clock, type, name),
		m_chip(*this),
		m_clock(clock),
		m_clocks(0),
		m_step(0x100000000ull / m_chip.sample_rate(clock)),
		m_pos(0)
	{
		m_chip.reset();
	}

	virtual uint32_t sample_rate() const override
	{
		return m_chip.sample_rate(m_clock);
	}

	// handle a register write: just queue for now
	virtual void write(uint32_t reg, uint8_t data) override
	{
		m_queue.push_back(std::make_pair(reg, data));
	}

    virtual void setSSGVolumeRatio(double ratio){
        ssgvol = ratio;
    }

	// generate one output sample of output
	virtual void generate(emulated_time output_start, emulated_time output_step, int32_t *buffer) override
	{
		uint32_t addr1 = 0xffff, addr2 = 0xffff;
		uint8_t data1 = 0, data2 = 0;

		// see if there is data to be written; if so, extract it and dequeue
		if (!m_queue.empty())
		{
			auto front = m_queue.front();
			addr1 = 0 + 2 * ((front.first >> 8) & 3);
			data1 = front.first & 0xff;
			addr2 = addr1 + ((m_type == CHIP_YM2149) ? 2 : 1);
			data2 = front.second;
			m_queue.erase(m_queue.begin());
		}

		// write to the chip
		if (addr1 != 0xffff)
		{
			if (LOG_WRITES)
				printf("%10.5f: %s %03X=%02X\n", double(m_clocks) / double(m_chip.sample_rate(m_clock)), m_name.c_str(), data1, data2);
			m_chip.write(addr1, data1);
			m_chip.write(addr2, data2);
		}

		// generate at the appropriate sample rate
		for ( ; m_pos <= output_start; m_pos += m_step)
		{
			m_chip.generate(&m_output);

		}

		// add the final result to the buffer
		if (m_type == CHIP_YM2203)
		{
			int32_t out0 = m_output.data[0];
			int32_t out1 = m_output.data[1 % ChipType::OUTPUTS];
			int32_t out2 = m_output.data[2 % ChipType::OUTPUTS];
			int32_t out3 = m_output.data[3 % ChipType::OUTPUTS];
			*buffer++ += out0 + out1 + out2 + out3;
			*buffer++ += out0 + out1 + out2 + out3;
		}
		else if (m_type == CHIP_YM2608 || m_type == CHIP_YM2610)
		{
			int32_t out0 = m_output.data[0];
			int32_t out1 = m_output.data[1 % ChipType::OUTPUTS];
			int32_t out2 = m_output.data[2 % ChipType::OUTPUTS];
			*buffer++ += out0 + out2 * ssgvol;
			*buffer++ += out1 + out2 * ssgvol;
		}
		else if (m_type == CHIP_YMF278B)
		{
			*buffer++ += m_output.data[4];
			*buffer++ += m_output.data[5];
		}
		else if (ChipType::OUTPUTS == 1)
		{
			*buffer++ += m_output.data[0];
			*buffer++ += m_output.data[0];
		}
		else
		{
			*buffer++ += m_output.data[0];
			*buffer++ += m_output.data[1 % ChipType::OUTPUTS];
		}
		m_clocks++;
	}

protected:
	// handle a read from the buffer
	virtual uint8_t ymfm_external_read(ymfm::access_class type, uint32_t offset) override
	{
		auto &data = m_data[type];
		return (offset < data.size()) ? data[offset] : 0;
	}

	// internal state
	ChipType m_chip;
	uint32_t m_clock;
	uint64_t m_clocks;
	typename ChipType::output_data m_output;
	emulated_time m_step;
	emulated_time m_pos;
	std::vector<std::pair<uint32_t, uint8_t>> m_queue;


};



//*********************************************************
//  GLOBAL HELPERS
//*********************************************************

// global vector of active chips
std::vector<vgm_chip_base *> active_chips;



//-------------------------------------------------
//  parse_uint32 - parse a little-endian uint32_t
//-------------------------------------------------

uint32_t parse_uint32(std::vector<uint8_t> &buffer, uint32_t &offset)
{
	uint32_t result = buffer[offset++];
	result |= buffer[offset++] << 8;
	result |= buffer[offset++] << 16;
	result |= buffer[offset++] << 24;
	return result;
}

//-------------------------------------------------
//  add_chips - add 1 or 2 instances of the given
//  supported chip type
//-------------------------------------------------

template<typename ChipType>
void add_chips(uint32_t clock, chip_type type, char const *chipname)
{
	uint32_t clockval = clock & 0x3fffffff;
	printf("Adding %s @ %dHz\n", chipname, clockval);

	char name[100];
	sprintf(name, "%s #%d", chipname, index);
	active_chips.push_back(new vgm_chip<ChipType>(clockval, type, chipname));

	if (type == CHIP_YM2608)
	{
		FILE *rom = fopen("ym2608_adpcm_rom.bin", "rb");
		if (rom == nullptr)
			fprintf(stderr, "Warning: YM2608 enabled but ym2608_adpcm_rom.bin not found\n");
		else
		{
			fseek(rom, 0, SEEK_END);
			uint32_t size = ftell(rom);
			fseek(rom, 0, SEEK_SET);
			std::vector<uint8_t> temp(size);
			fread(&temp[0], 1, size, rom);
			fclose(rom);
			for (auto chip : active_chips)
				if (chip->type() == type)
					chip->write_data(ymfm::ACCESS_ADPCM_A, 0, size, &temp[0]);
		}
	}
}



//-------------------------------------------------
//  find_chip - find the given chip and index
//-------------------------------------------------


vgm_chip_base *find_chip(chip_type type, uint8_t index)
{
	if (index < active_chips.size()){
		return active_chips[index];
	}
	return nullptr;
}


//-------------------------------------------------
//  write_chip - handle a write to the given chip
//  and index
//-------------------------------------------------

void write_chip(uint8_t index, uint32_t reg, uint8_t data)
{
	if (index < active_chips.size()){
		active_chips[index]->write(reg, data);
	}
}


//-------------------------------------------------
//  add_rom_data - add data to the given chip
//  type in the given access class
//-------------------------------------------------

void add_rom_data(chip_type type, ymfm::access_class access, std::vector<uint8_t> &buffer, uint32_t &localoffset, uint32_t size)
{
	uint32_t length = parse_uint32(buffer, localoffset);
	uint32_t start = parse_uint32(buffer, localoffset);
	for (int index = 0; index < 64; index++)
	{
		vgm_chip_base *chip = find_chip(type, index);
		if (chip != nullptr)
			chip->write_data(access, start, size, &buffer[localoffset]);
	}
}


//-------------------------------------------------
//  write_wav - write a WAV file from the provided
//  stereo data
//-------------------------------------------------

int write_wav(char const *filename, uint32_t output_rate, std::vector<int32_t> &wav_buffer_src)
{
	// determine normalization parameters
	int32_t max_scale = 0;
	for (int index = 0; index < wav_buffer_src.size(); index++)
	{
		int32_t absval = std::abs(wav_buffer_src[index]);
		max_scale = std::max(max_scale, absval);
	}
	// now convert
	std::vector<int16_t> wav_buffer(wav_buffer_src.size());
	if (max_scale != 0){
		for (int index = 0; index < wav_buffer_src.size(); index++){
			wav_buffer[index] = wav_buffer_src[index] * 26000 / max_scale;
		}
	}else{
		copy(wav_buffer_src.begin(), wav_buffer_src.end(), back_inserter(wav_buffer) );
	}

	// write the WAV file
	FILE *out = fopen(filename, "wb");
	if (out == nullptr)
	{
		fprintf(stderr, "Error creating output file '%s'\n", filename);
		return 6;
	}

	// write the 'RIFF' header
	if (fwrite("RIFF", 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the total size
	uint32_t total_size = 48 + wav_buffer.size() * 2 - 8;
	uint8_t wavdata[4];
	wavdata[0] = total_size >> 0;
	wavdata[1] = total_size >> 8;
	wavdata[2] = total_size >> 16;
	wavdata[3] = total_size >> 24;
	if (fwrite(wavdata, 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the 'WAVE' type
	if (fwrite("WAVE", 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the 'fmt ' tag
	if (fwrite("fmt ", 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the format length
	wavdata[0] = 16;
	wavdata[1] = 0;
	wavdata[2] = 0;
	wavdata[3] = 0;
	if (fwrite(wavdata, 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the format (PCM)
	wavdata[0] = 1;
	wavdata[1] = 0;
	if (fwrite(wavdata, 1, 2, out) != 2)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the channels
	wavdata[0] = 2;
	wavdata[1] = 0;
	if (fwrite(wavdata, 1, 2, out) != 2)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the sample rate
	wavdata[0] = output_rate >> 0;
	wavdata[1] = output_rate >> 8;
	wavdata[2] = output_rate >> 16;
	wavdata[3] = output_rate >> 24;
	if (fwrite(wavdata, 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the bytes/second
	uint32_t bps = output_rate * 2 * 2;
	wavdata[0] = bps >> 0;
	wavdata[1] = bps >> 8;
	wavdata[2] = bps >> 16;
	wavdata[3] = bps >> 24;
	if (fwrite(wavdata, 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the block align
	wavdata[0] = 4;
	wavdata[1] = 0;
	if (fwrite(wavdata, 1, 2, out) != 2)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the bits/sample
	wavdata[0] = 16;
	wavdata[1] = 0;
	if (fwrite(wavdata, 1, 2, out) != 2)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the 'data' tag
	if (fwrite("data", 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the data length
	uint32_t datalen = wav_buffer.size() * 2;
	wavdata[0] = datalen >> 0;
	wavdata[1] = datalen >> 8;
	wavdata[2] = datalen >> 16;
	wavdata[3] = datalen >> 24;
	if (fwrite(wavdata, 1, 4, out) != 4)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}

	// write the data
	if (fwrite(&wav_buffer[0], 1, datalen, out) != datalen)
	{
		fprintf(stderr, "Error writing to output file\n");
		return 7;
	}
	fclose(out);
	return 0;
}

int nextdata(S98File& file, unsigned int& pointer){
    unsigned char* p;
    int n, s, i;
    p = file.data + pointer;
    if(pointer >= file.filesize){
        //filesize error
        return -2;
    }

	if (p[0] < 0x7F){
		if (p[0] % 2 == 0)
			write_chip(p[0] / 2, p[1], p[2]);
		else
			write_chip(p[0] / 2, p[1] | 0x100, p[2]);

		pointer += 3;
		return 0;
	}else if (p[0] == 0xff){
		++pointer;
		return 1;
	}else if (p[0] == 0xfe){
		n = s = 0; i = 0;
		do{
			++i;
			n |= (p[i] & 0x7f) << s;
			s += 7;
		} while(p[i] & 0x80);
		n += 2;
		pointer += i + 1;
		
		return n;
	}else if (p[0] == 0xfd){
		++pointer;
		return -1;
	}

	return -1;
}

void generate_all(S98File& file, int loop, uint32_t output_rate, double ssgvol, std::vector<int32_t> &wav_buffer){
    emulated_time output_step = 0x100000000ull / output_rate;
	emulated_time output_pos = 0;
    int loopcount = 0;
    //create chip
    for (int i = 0; i < file.header->devicecount; i++){
        switch (file.header->deviceInfo[i].type)
        {
		case S98File::TYPE_PSG:
			add_chips<ymfm::ym2149>(file.header->deviceInfo[i].clock, CHIP_YM2149, "YM2149");
			break;
        case S98File::TYPE_OPNA:
			add_chips<ymfm::ym2608>(file.header->deviceInfo[i].clock, CHIP_YM2608, "YM2608");
			break;
        case S98File::TYPE_OPN:
            add_chips<ymfm::ym2203>(file.header->deviceInfo[i].clock, CHIP_YM2203, "YM2203");
            break;
        case S98File::TYPE_OPM:
            add_chips<ymfm::ym2151>(file.header->deviceInfo[i].clock, CHIP_YM2151, "YM2151");
            break;
        default:
            break;
        }
    }
    //for s98v1
    if (file.header->format == '1'){
        add_chips<ymfm::ym2608>(7987200, CHIP_YM2608, "YM2608");
    }

    for (auto chip : active_chips)
        if (chip->type() == CHIP_YM2608)
            chip->set_ssg_volume(ssgvol);
    
    unsigned int pointer = file.header->dataptr;
    while (loopcount <= (file.header->loopptr == 0 ? 0 : loop)){
        int ret = nextdata(file, pointer);
        switch (ret) {
            case 0:
                break;
            case -1:
                loopcount++;
                pointer = file.header->loopptr;
                break;
            default:
                int delay = ret * output_rate * file.header->timer1 / file.header->timer2;
                while (delay-- != 0)
                {
                    int32_t outputs[2] = { 0 };
                    for (auto chip : active_chips)
                        chip->generate(output_pos, output_step, outputs);
                    output_pos += output_step;
                    wav_buffer.push_back(outputs[0]);
                    wav_buffer.push_back(outputs[1]);
                }
                break;
        }
    }
}


//-------------------------------------------------
//  main - program entry point
//-------------------------------------------------

int main(int argc, char *argv[])
{
	char const *filename = nullptr;
	char const *outfilename = nullptr;
    int loop_count = 0;
	int output_rate = 44100;
    double ssg_vol = 1;

	// parse command line
	bool argerr = false;
	for (int arg = 1; arg < argc; arg++)
	{
		char const *curarg = argv[arg];
		if (*curarg == '-')
		{
			if (strcmp(curarg, "-o") == 0 || strcmp(curarg, "--output") == 0)
				outfilename = argv[++arg];
			else if (strcmp(curarg, "-r") == 0 || strcmp(curarg, "--samplerate") == 0)
				output_rate = atoi(argv[++arg]);
			else if (strcmp(curarg, "-l") == 0 || strcmp(curarg, "--loop") == 0)
                loop_count = atoi(argv[++arg]);
            else if (strcmp(curarg, "-v") == 0 || strcmp(curarg, "--ssgvolume") == 0)
                ssg_vol = atof(argv[++arg]);
            else
			{
				fprintf(stderr, "Unknown argument: %s\n", curarg);
				argerr = true;
			}
		}
		else
			filename = curarg;
	}

	// if invalid syntax, show usage
	if (argerr || filename == nullptr || outfilename == nullptr)
	{
		fprintf(stderr, "Usage: s98render <inputfile> -o <outputfile> [-v <ssg volume ratio>] [-l <loop count>] [-r <rate>]\n");
		return 1;
	}

    S98File* s98File = new S98File();
    bool canRead = s98File->setFilePath(filename);
    if (!canRead) return 1;

	// generate the output
	std::vector<int32_t> wav_buffer;
    generate_all(*s98File, loop_count, output_rate, ssg_vol, wav_buffer);

	int err = write_wav(outfilename, output_rate, wav_buffer);

	return err;
}


# Build settings
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
INCLUDES = -I../../src

# Parallel build support - auto-detect CPU cores
MAKEFLAGS += -j$(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Source files
SOURCES = opm_sample.cpp opm_common.cpp \
    01_sine_wave_scale.cpp \
    02_piano_scale.cpp \
    03_twinkle_twinkle_chords.cpp \
    04_lfo_effects.cpp \
    05_fm_algorithms.cpp \
    06_envelope_control.cpp \
    07_detune_effects.cpp \
    08_stereo_arpeggiator.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# ymfm source files
YMFM_SOURCES = ../../src/ymfm_opm.cpp
YMFM_OBJECTS = $(YMFM_SOURCES:.cpp=.o)

TARGET = opm_sample

all: $(TARGET)

$(TARGET): $(OBJECTS) $(YMFM_OBJECTS)
	$(CXX) $(OBJECTS) $(YMFM_OBJECTS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

../../src/%.o: ../../src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) *.o ../../src/*.o *.wav

run: $(TARGET)
	./$(TARGET)

# Parallel build targets
fast: clean
	@echo "Building with $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) parallel jobs..."
	$(MAKE) -j$(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) $(TARGET)

# Override parallel jobs manually if needed
parallel%:
	$(MAKE) -j$(subst parallel,,$@) $(TARGET)

.PHONY: all clean run fast
cmake_minimum_required(VERSION 3.10)
project(OPMSampleProject_Example CXX) # Changed project name for clarity

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define ymfm source files from the main src directory
set(YMFM_SOURCES
    ${CMAKE_SOURCE_DIR}/src/ymfm_adpcm.cpp
    ${CMAKE_SOURCE_DIR}/src/ymfm_misc.cpp
    ${CMAKE_SOURCE_DIR}/src/ymfm_opl.cpp
    ${CMAKE_SOURCE_DIR}/src/ymfm_opm.cpp
    ${CMAKE_SOURCE_DIR}/src/ymfm_opn.cpp
    ${CMAKE_SOURCE_DIR}/src/ymfm_opq.cpp
    ${CMAKE_SOURCE_DIR}/src/ymfm_opz.cpp
    ${CMAKE_SOURCE_DIR}/src/ymfm_pcm.cpp
    ${CMAKE_SOURCE_DIR}/src/ymfm_ssg.cpp
)

# Add executable
add_executable(opm_sample opm_sample.cpp ${YMFM_SOURCES})

# Include directory for ymfm headers (main src directory)
target_include_directories(opm_sample PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Link libraries (e.g., for std::chrono, std::thread, std::cmath)
if(CMAKE_THREAD_PREFER_PTHREADS)
    find_package(Threads REQUIRED)
    target_link_libraries(opm_sample PRIVATE Threads::Threads)
endif()
if(UNIX AND NOT APPLE AND NOT WIN32) # Added NOT WIN32 for broader compatibility
    target_link_libraries(opm_sample PRIVATE m) # For libm (math library for sin, pow, etc.)
endif()

# Output executable to the current build directory (e.g., examples/opm/build/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# --- Instructions for user ---
# This CMakeLists.txt is typically called from a root CMakeLists.txt.
# 1. To build (from project root):
#    mkdir build
#    cd build
#    cmake ..
#    make
#
# 2. The executable 'opm_sample' will be in 'build/examples/opm/'.
#    The 'output.wav' will be generated in the directory where opm_sample is run.
#    If run from 'build/examples/opm/', output.wav will be there.
# --- End Instructions ---

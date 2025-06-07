cmake_minimum_required(VERSION 3.10)
project(OPMSampleProject)

set(CMAKE_CXX_STANDARD 17) # Changed from 11 to 17
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Create a directory for ymfm library source files if it doesn't exist
# This is mainly for user clarity; the script assumes ymfm_lib will be populated.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ymfm_lib)

# Define ymfm source files
# These are the .cpp files from the ymfm library's 'src' folder
# that need to be compiled along with the main project.
set(YMFM_SOURCES
    ymfm_lib/ymfm_adpcm.cpp
    ymfm_lib/ymfm_misc.cpp
    ymfm_lib/ymfm_opl.cpp
    ymfm_lib/ymfm_opm.cpp
    ymfm_lib/ymfm_opn.cpp
    ymfm_lib/ymfm_opq.cpp
    ymfm_lib/ymfm_opz.cpp
    ymfm_lib/ymfm_pcm.cpp
    ymfm_lib/ymfm_ssg.cpp
)

# Add executable
add_executable(opm_sample opm_sample.cpp ${YMFM_SOURCES})

# Include directory for ymfm headers (e.g., ymfm.h, ymfm_opm.h)
# and for opm_sample.cpp to find #include "ymfm_lib/ymfm.h"
target_include_directories(opm_sample PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries
# For pthreads (std::thread)
if(CMAKE_THREAD_PREFER_PTHREADS)
    find_package(Threads REQUIRED)
    target_link_libraries(opm_sample PRIVATE Threads::Threads)
endif()

# For libm (math functions like sin, pow) on Unix-like systems
if(UNIX AND NOT APPLE)
    target_link_libraries(opm_sample PRIVATE m)
endif()

install(TARGETS opm_sample DESTINATION bin)

# --- Instructions for user ---
# 1. Make sure the ymfm library source files (.cpp and .h, including the 'detail'
#    subdirectory and others from ymfm's 'src') are copied into a
#    subdirectory named 'ymfm_lib' within your project's root directory
#    (where this CMakeLists.txt is located).
#
#    The required ymfm files are typically found in the 'src' directory of the ymfm library.
#    For example, after copying, you should have paths like:
#    your_project_root/ymfm_lib/ymfm.h
#    your_project_root/ymfm_lib/ymfm_opm.h
#    your_project_root/ymfm_lib/ymfm_opm.cpp
#    your_project_root/ymfm_lib/detail/fm_op_state.h
#    etc.
#
# 2. To build:
#    mkdir build
#    cd build
#    cmake ..
#    make
#
# 3. The executable 'opm_sample' will be in the 'build' directory (or 'build/bin' if 'make install' is used
#    and CMAKE_INSTALL_PREFIX is default or set appropriately).
# --- End Instructions ---

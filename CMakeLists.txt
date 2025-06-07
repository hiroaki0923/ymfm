cmake_minimum_required(VERSION 3.10)
project(YMFM_Examples_Project CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the examples/opm subdirectory
add_subdirectory(examples/opm)

# Potentially add other examples or library targets here if needed in the future

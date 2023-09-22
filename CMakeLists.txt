cmake_minimum_required(VERSION 3.26)
project(MargueriteDSP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)

include_directories(dsp)

add_subdirectory(dsp)
add_subdirectory(Tests)
add_subdirectory(externals/googletest)
add_library(MargueriteDSP STATIC
phaseshapers.cpp
sinc_resampler.cpp
)

set(CMAKE_BUILD_TYPE Debug)

target_include_directories(MargueriteDSP PUBLIC ${CMAKE_CURRENT_DIRECTORY})
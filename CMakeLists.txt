project(spectralizer)

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    add_definitions(-DLINUX=1)
    add_definitions(-DUNIX=1)
endif ()

set(spectralizer_SOURCES
        kiss_fft130/kiss_fft.c
        kiss_fft130/kiss_fftr.c
        src/spectralizer.cpp
        src/source/visualizer_source.cpp
        src/source/visualizer_source.hpp
        src/util/util.hpp
        src/util/audio/spectrum_visualizer.cpp
        src/util/audio/spectrum_visualizer.hpp
        src/util/audio/bar_visualizer.cpp
        src/util/audio/bar_visualizer.hpp
        src/util/audio/wire_visualizer.cpp
        src/util/audio/wire_visualizer.hpp
        src/util/audio/fifo.cpp
        src/util/audio/fifo.hpp
        src/util/audio/obs_internal_source.cpp
        src/util/audio/obs_internal_source.hpp
        src/util/audio/audio_visualizer.cpp
        src/util/audio/audio_visualizer.hpp
        src/util/audio/audio_source.hpp)

add_library(spectralizer MODULE
        ${spectralizer_SOURCES})
target_link_libraries(spectralizer
        libobs)

include_directories(kiss_fft130)
install_obs_plugin_with_data(spectralizer data)

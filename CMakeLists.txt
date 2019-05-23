project(spectralizer)

if (MSVC)
    set(spectralizer_PLATFORM_DEPS
            w32-pthreads)
endif ()

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    add_definitions(-DLINUX=1)
    add_definitions(-DUNIX=1)
    set(spectralizer_PLATFORM_DEPS
            mpdclient)
endif ()

set(spectralizer_SOURCES
        src/spectralizer.cpp
        src/source/visualizer_source.cpp
        src/util/util.hpp
        src/util/audio/spectrum_visualizer.cpp
        src/util/audio/spectrum_visualizer.hpp
        src/util/audio/fifo.cpp
        src/util/audio/fifo.hpp
        src/util/audio/audio_processor.hpp
        src/util/audio/audio_visualizer.hpp
        src/util/audio/audio_processor.cpp)

add_library(spectralizer MODULE
        ${spectralizer_SOURCES})
target_link_libraries(spectralizer
        libobs
        fftw3
        ${spectralizer_PLATFORM_DEPS})

install_obs_plugin_with_data(spectralizer data)

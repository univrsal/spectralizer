project(spectraliser)

if (MSVC)
    set(spectraliser_PLATFORM_DEPS
            w32-pthreads)
endif ()

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    add_definitions(-DLINUX=1)
    add_definitions(-DUNIX=1)
    set(spectraliser_PLATFORM_DEPS
            mpdclient)
endif ()

set(spectraliser_SOURCES
        src/spectraliser.cpp
        src/source/visualiser_source.cpp
        src/util/util.hpp
        src/util/audio/spectrum_visualiser.cpp
        src/util/audio/spectrum_visualiser.hpp
        src/util/audio/fifo.cpp
        src/util/audio/fifo.hpp
        src/util/audio/audio_processor.hpp
        src/util/audio/audio_visualiser.hpp
        src/util/audio/audio_processor.cpp)

add_library(spectraliser MODULE
        ${spectraliser_SOURCES})
target_link_libraries(spectraliser
        libobs
        fftw3
        ${spectraliser_PLATFORM_DEPS})

install_obs_plugin_with_data(spectraliser data)
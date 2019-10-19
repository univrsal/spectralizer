project(spectralizer)

if (MSVC)
message(STATUS "[spectralizer] On windows fftw3 is available in three versions. fftw3-3, fftw3f-3 and fftw3l-3. Going from least precision to most precesion. For this plugin least precision should be fine")
    set(spectralizer_PLATFORM_DEPS
            w32-pthreads)
endif ()

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    add_definitions(-DLINUX=1)
    add_definitions(-DUNIX=1)
endif ()

find_path(FFTW_INCLUDE_DIRS fftw3.h)
find_library(FFTW_LIBRARIES fftw3)

if (WIN32)
        set(FFTW_BINARY fftw3-3.dll)
	math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")
	add_custom_command(TARGET obs-browser POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"./fftw3/bin/${BITS}bit/${FFTW_BINARY}"
		"${CMAKE_BINARY_DIR}/rundir/$<CONFIGURATION>/obs-plugins/${BITS}bit/"
	)
endif()

set(spectralizer_SOURCES
        src/spectralizer.cpp
        src/source/visualizer_source.cpp
        src/source/visualizer_source.hpp
        src/util/util.hpp
        src/util/audio/spectrum_visualizer.cpp
        src/util/audio/spectrum_visualizer.hpp
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
        libobs
        ${FFTW_LIBRARIES}
        ${spectralizer_PLATFORM_DEPS})

include_directories(${FFTW_INCLUDE_DIRS})
install_obs_plugin_with_data(spectralizer data)

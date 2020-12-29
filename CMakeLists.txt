cmake_minimum_required(VERSION 3.5)
project(spectralizer VERSION 1.3.2)
set(PLUGIN_AUTHOR "univrsal")
set(PLUGIN_GIT spectralizer)
set(LINUX_MAINTAINER_EMAIL "uni@vrsal.cf")
set(MACOS_BUNDLEID "cf.vrsal.spectralizer")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_PREFIX_PATH "${QTDIR}")
string(TIMESTAMP TODAY "%Y.%m.%d %H:%M")
add_definitions(-DBUILD_TIME="${TODAY}")
add_definitions(-DSPECTRALIZER_VERSION="${PROJECT_VERSION}")


if (WIN32 OR APPLE)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
endif()

option(LOCAL_INSTALLATION "Whether to install for the current user (default: OFF)" OFF)
option(GLOBAL_INSTALLATION "Whether to install for all users (default: OFF)" OFF)
option(USE_CMAKE_LIBDIR "Whether to use install to the cmake defined library directory, which breaks on ubuntu. (default: OFF)" OFF)

if (MSVC)
    set(spectralizer_PLATFORM_DEPS
            ${W32_PTHREADS_LIB})
endif ()

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    add_definitions(-DLINUX=1)
    add_definitions(-DUNIX=1)
endif ()

find_package(LibObs REQUIRED)
find_path(FFTW_INCLUDE_DIRS fftw3.h)
find_library(FFTW_LIBRARIES fftw3)

configure_file(
    package/installer-macOS.pkgproj.in
    ${CMAKE_SOURCE_DIR}/package/installer-macOS.pkgproj
)
configure_file(
    package/installer-Windows.iss.in
    ${CMAKE_SOURCE_DIR}/package/installer-Windows.iss
)
configure_file(
    package/README.txt.in
    ${CMAKE_SOURCE_DIR}/package/README.txt
)

set(spectralizer_SOURCES
    src/spectralizer.cpp
    src/source/visualizer_source.cpp
    src/source/visualizer_source.hpp
    src/util/util.hpp
    src/util/util.cpp
    src/util/audio/spectrum_visualizer.cpp
    src/util/audio/spectrum_visualizer.hpp
    src/util/audio/bar_visualizer.cpp
    src/util/audio/bar_visualizer.hpp
    src/util/audio/circle_bar_visualizer.cpp
    src/util/audio/circle_bar_visualizer.hpp
    src/util/audio/wire_visualizer.cpp
    src/util/audio/wire_visualizer.hpp
    src/util/audio/fifo.cpp
    src/util/audio/fifo.hpp
    src/util/audio/obs_internal_source.cpp
    src/util/audio/obs_internal_source.hpp
    src/util/audio/audio_visualizer.cpp
    src/util/audio/audio_visualizer.hpp
    src/util/audio/audio_source.hpp)

if (APPLE)
    add_definitions(-DMACOS=1)
endif()

add_library(spectralizer MODULE
        ${spectralizer_SOURCES})

target_link_libraries(spectralizer
    ${LIBOBS_LIBRARIES}
    ${FFTW_LIBRARIES}
    ${OBS_FRONTEND_LIB}
    ${spectralizer_PLATFORM_DEPS})

include_directories(${FFTW_INCLUDE_DIRS}
    "${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
    ${LIBOBS_INCLUDE_DIR}
)

# Installation stuff

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH_NAME "64bit")
    set(OBS_BUILDDIR_ARCH "build64")
else()
    set(ARCH_NAME "32bit")
    set(OBS_BUILDDIR_ARCH "build32")
endif()

if(WIN32)
    # Enable Multicore Builds and disable FH4 (to not depend on VCRUNTIME140_1.DLL when building with VS2019)
    if (MSVC)
        add_definitions(/MP /d2FH4-)
    endif()

    include_directories(
        "${LIBOBS_INCLUDE_DIR}"
    )

    target_link_libraries(${CMAKE_PROJECT_NAME}
        "${OBS_FRONTEND_LIB}"
    )

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH_NAME "64bit")
        set(OBS_BUILDDIR_ARCH "build64")
    else()
        set(ARCH_NAME "32bit")
        set(OBS_BUILDDIR_ARCH "build32")
    endif()

    get_filename_component(LibOBS_DIR ${LIBOBS_LIB} DIRECTORY)

    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        # Copy to obs-studio dev environment for immediate testing
        COMMAND
            "${CMAKE_COMMAND}" -E copy
                "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
                "${LibOBS_DIR}/../../rundir/$<CONFIG>/obs-plugins/${ARCH_NAME}"

        COMMAND
            "${CMAKE_COMMAND}" -E copy
                "$<TARGET_PDB_FILE:${CMAKE_PROJECT_NAME}>"
                "${LibOBS_DIR}/../../rundir/$<CONFIG>/obs-plugins/${ARCH_NAME}"

        COMMAND
            "${CMAKE_COMMAND}" -E make_directory
                "${LibOBS_DIR}/../../rundir/$<CONFIG>/data/obs-plugins/${CMAKE_PROJECT_NAME}"

        COMMAND
            "${CMAKE_COMMAND}" -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/data"
                "${LibOBS_DIR}/../../rundir/$<CONFIG>/data/obs-plugins/${CMAKE_PROJECT_NAME}"
        COMMAND
            ${CMAKE_COMMAND} -E copy
                "${CMAKE_CURRENT_SOURCE_DIR}/fftw3/bin/${ARCH_NAME}/libfftw3-3.dll"
                "${LibOBS_DIR}/../../rundir/$<CONFIG>/obs-plugins/${ARCH_NAME}"
    )
elseif(UNIX AND NOT APPLE)
    include(GNUInstallDirs)

    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "")
    target_link_libraries(${CMAKE_PROJECT_NAME} obs-frontend-api)

    if (LOCAL_INSTALLATION)
        # Installs into home directory
        install(TARGETS ${CMAKE_PROJECT_NAME}
            LIBRARY DESTINATION "$ENV{HOME}/.config/obs-studio/plugins/${CMAKE_PROJECT_NAME}/bin/${ARCH_NAME}")

        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
            DESTINATION "$ENV{HOME}/.config/obs-studio/plugins/${CMAKE_PROJECT_NAME}")
    elseif(GLOBAL_INSTALLATION)
        # For *.deb installer
        if (USE_CMAKE_LIBDIR)
            install(TARGETS ${CMAKE_PROJECT_NAME}
                LIBRARY DESTINATION "lib/obs-plugins") # hardcoded, since ubuntu uses the wrong path by default
        else()
            install(TARGETS ${CMAKE_PROJECT_NAME}
                LIBRARY DESTINATION "/usr/lib/obs-plugins")
        endif()

        file(GLOB locale_files ${CMAKE_SOURCE_DIR}/data/locale/*.ini)
        file(GLOB other_files ${CMAKE_SOURCE_DIR}/data/*.*)

        install(FILES ${locale_files}
            DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/obs/obs-plugins/${CMAKE_PROJECT_NAME}/locale")
        install(FILES ${other_files}
            DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/obs/obs-plugins/${CMAKE_PROJECT_NAME}")
    else()
        # For *.zip binary
        install(TARGETS ${CMAKE_PROJECT_NAME}
            LIBRARY DESTINATION "${CMAKE_PROJECT_NAME}/bin/${ARCH_NAME}")

        install(DIRECTORY ${CMAKE_SOURCE_DIR}/data
            DESTINATION "${CMAKE_PROJECT_NAME}")

        install(FILES "${CMAKE_SOURCE_DIR}/package/README.txt"
            DESTINATION "${CMAKE_PROJECT_NAME}")
    endif()
elseif(APPLE)
    if (LOCAL_INSTALLATION)
        # Installs into home directory
        install(TARGETS ${CMAKE_PROJECT_NAME}
            LIBRARY DESTINATION "$ENV{HOME}/Library/Application Support/obs-studio/plugins/${CMAKE_PROJECT_NAME}/bin/")

        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
            DESTINATION "$ENV{HOME}/Library/Application Support/obs-studio/plugins/${CMAKE_PROJECT_NAME}")
    endif()
endif()

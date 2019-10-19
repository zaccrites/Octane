
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(synthetic VERSION 0.1.0
        DESCRIPTION "FPGA-based FM synthesizer"
        LANGUAGES CXX)


# TODO: Remove this dependency and just emit WAVE files directly.
find_package(SDL2 REQUIRED)


add_subdirectory(Vmodules)



add_executable(synth
    "src/main.cpp"
    "src/synth.cpp"
    "src/PatchConfig.cpp"
    "src/debug.cpp"

    # TODO: Add these somewhere else?
    "roms/sine_rom.hex"
    "roms/algorithm_rom.hex"
)


# Note that these should really be generated out-of-source, as they would
# need to be distributed with the executable.
#
# Although for generating an FPGA bitsteram they would need to be consumed
# in-source.
add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/roms/sine_rom.hex"
    COMMAND python "tools/sine_rom_generator.py" > "${CMAKE_SOURCE_DIR}/roms/sine_rom.hex"
    DEPENDS tools/sine_rom_generator.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating sine table ROM"
    VERBATIM
)

add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/roms/algorithm_rom.hex"
    COMMAND python "tools/algorithm_rom_generator.py" > "${CMAKE_SOURCE_DIR}/roms/algorithm_rom.hex"
    DEPENDS tools/algorithm_rom_generator.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating algorithm ROM"
    VERBATIM
)


target_link_libraries(synth
    ${SDL2_LIBRARIES}
    SDL2_ttf

    Vsynth
)

target_include_directories(synth PRIVATE SYSTEM
    "vendor/include"
)


# TODO: Prevent copy-paste of compiler settings to unit_test target
target_compile_options(synth PRIVATE
    -std=c++14
    -pedantic
    -Wall
    -Wextra
    # -Werror
    # -Weffc++
    -Wshadow
    -Wcast-qual
    -Wold-style-cast
    -Wfloat-equal

    -Werror=return-type
    -Werror=uninitialized
    # -Werror=sometimes-uninitialized

    -fdiagnostics-color

    # -O2

    -g
    -Og
    -rdynamic

)



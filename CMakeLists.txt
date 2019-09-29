
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(synthetic VERSION 0.1.0
        DESCRIPTION "FPGA-based FM synthesizer"
        LANGUAGES CXX)


find_package(SDL2 REQUIRED)


add_subdirectory(Vmodules)



add_executable(synth
    "src/main.cpp"
)

target_link_libraries(synth
    ${SDL2_LIBRARIES}
    SDL2_ttf

    Vsynth
)


# TODO: Prevent copy-paste of compiler settings to unit_test target
target_compile_options(synth PRIVATE
    -std=c++14
    -pedantic
    -Wall
    -Wextra
    # -Werror
    -Weffc++
    -Wshadow
    -Wcast-qual
    -Wold-style-cast
    -Wfloat-equal

    -Werror=return-type
    -Werror=sometimes-uninitialized

    -fdiagnostics-color

    -O2
    # -g

)



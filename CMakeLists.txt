
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(octane VERSION 0.1.0
        DESCRIPTION "An eight-operator FM synthesizer"
        LANGUAGES CXX)


# TODO: Remove this dependency and just emit WAVE files directly.
find_package(SDL2 REQUIRED)


add_subdirectory(Vmodules)
add_subdirectory(synthesis)



add_executable(simulator
    "src/main.cpp"
    "src/synth.cpp"
    "src/PatchConfig.cpp"
    "src/debug.cpp"
)

set_target_properties(simulator PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
)


target_link_libraries(simulator
    ${SDL2_LIBRARIES}
    SDL2_ttf

    Vsynth
)

target_include_directories(simulator PRIVATE SYSTEM
    "vendor/include"
)


# TODO: Prevent copy-paste of compiler settings to unit_test target
target_compile_options(simulator PRIVATE
    -pedantic
    -Wall
    -Wextra
    # -Werror
    # -Weffc++
    -Wshadow
    -Wcast-qual
    -Wold-style-cast
    # -Wfloat-equal

    -Werror=return-type
    -Werror=uninitialized
    # -Werror=sometimes-uninitialized

    -fdiagnostics-color

    # -O2

    -g
    -Og
    -rdynamic

)



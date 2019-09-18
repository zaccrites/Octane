
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(softsynth VERSION 0.1.0
        DESCRIPTION "Software FM synthesizer"
        LANGUAGES CXX)


find_package(SDL2 REQUIRED)



add_executable(softsynth
    "main.cpp"
)

target_link_libraries(softsynth
    ${SDL2_LIBRARIES}
    SDL2_ttf
)


# TODO: Prevent copy-paste of compiler settings to unit_test target
target_compile_options(softsynth PRIVATE
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

    -fdiagnostics-color

    -O2
    # -g

)




cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(octane VERSION 0.1.0
        DESCRIPTION "An eight-operator FM synthesizer"
        LANGUAGES C CXX)

set(COMMON_COMPILE_FLAGS
    -std=gnu++17
    -pedantic

    -Wall
    -Wextra
    -Weffc++
    -Wold-style-cast
    -Wnull-dereference
    -Wdouble-promotion
    -Wlogical-op
    -Wuseless-cast
    -Wshadow
    -Werror=switch
    -Werror=return-type

    -Og
    -g3
    # -O2

    -fdiagnostics-color
)

add_subdirectory(simulator)
add_subdirectory(hdl)

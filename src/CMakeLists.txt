
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(octane VERSION 0.1.0
        DESCRIPTION "An eight-operator FM synthesizer"
        LANGUAGES C CXX)

set(COMMON_COMPILE_OPTIONS
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

    -Og
    -g3
    # -O2

    -fdiagnostics-color
)

add_subdirectory(simulator)
add_subdirectory(embedded)
add_subdirectory(shared)
add_subdirectory(vendor)
add_subdirectory(hdl)

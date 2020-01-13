
find_package(SDL2 REQUIRED)

add_executable(simulator
    main.cpp
    Synth.cpp
)

target_compile_options(simulator PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)

target_link_libraries(simulator
    ${SDL2_LIBRARIES}
    Vsynth
    shared
)

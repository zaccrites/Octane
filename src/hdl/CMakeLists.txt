
set(VERILATOR_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/verilator_output)
set(VERILATOR_INCLUDE_DIR "/usr/share/verilator/include")

set(VSYNTH_SOURCE_FILES
    "${VERILATOR_OUTPUT_DIR}/Vsynth.cpp"
    # "${VERILATOR_OUTPUT_DIR}/Vsynth__024unit.cpp"
    "${VERILATOR_OUTPUT_DIR}/Vsynth__Syms.cpp"
)

add_library(Vsynth
    ${VSYNTH_SOURCE_FILES}
    "${VERILATOR_INCLUDE_DIR}/verilated.cpp"
)

target_include_directories(Vsynth SYSTEM PUBLIC
    ${VERILATOR_OUTPUT_DIR}
    ${VERILATOR_INCLUDE_DIR}
)

target_compile_options(Vsynth PRIVATE
    -Os
    -fdiagnostics-color
)

set_target_properties(Vsynth PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
)

add_custom_command(
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${VERILATOR_OUTPUT_DIR}
    COMMAND
        verilator_bin -cc "${CMAKE_CURRENT_SOURCE_DIR}/synth.sv"
        --MMD --Mdir ${VERILATOR_OUTPUT_DIR}
        -I${CMAKE_CURRENT_SOURCE_DIR}
        -Wall -Werror-width -Werror-selrange -Werror-implicit -Werror-pinmissing

    OUTPUT
        ${VSYNTH_SOURCE_FILES}

    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/double_register.sv"
        "${CMAKE_CURRENT_SOURCE_DIR}/spi.sv"
        "${CMAKE_CURRENT_SOURCE_DIR}/stage_envelope_attenuator.sv"
        "${CMAKE_CURRENT_SOURCE_DIR}/stage_modulator.sv"
        "${CMAKE_CURRENT_SOURCE_DIR}/stage_phase_accumulator.sv"
        "${CMAKE_CURRENT_SOURCE_DIR}/stage_sample_generator.sv"
        "${CMAKE_CURRENT_SOURCE_DIR}/stage_waveform_generator.sv"
        "${CMAKE_CURRENT_SOURCE_DIR}/synth.sv"
        "${CMAKE_CURRENT_SOURCE_DIR}/synth.svh"

    VERBATIM
)

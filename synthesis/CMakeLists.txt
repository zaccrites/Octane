

add_custom_target(synthesis DEPENDS octane.json)
add_custom_command(
    # TODO: Use configure_file to create a Yosys script with the right paths
    # and execute it. Might even be able to make al ist of verilog sources
    # and use it to generate the Yosys script, set DEPENDS of this command,
    # and run Verilator without wrapping in a Python script.
    #
    # See docs for the COMMAND argument: https://cmake.org/cmake/help/latest/command/add_custom_command.html
    COMMAND yosys
        -p "read_verilog -sv \"${CMAKE_SOURCE_DIR}/rtl/double_register.sv\""
        -p "read_verilog -sv \"${CMAKE_SOURCE_DIR}/rtl/spi.sv\""
        -p "read_verilog -sv \"${CMAKE_SOURCE_DIR}/rtl/stage_envelope_attenuator.sv\""
        -p "read_verilog -sv \"${CMAKE_SOURCE_DIR}/rtl/stage_modulator.sv\""
        -p "read_verilog -sv \"${CMAKE_SOURCE_DIR}/rtl/stage_phase_accumulator.sv\""
        -p "read_verilog -sv \"${CMAKE_SOURCE_DIR}/rtl/stage_sample_generator.sv\""
        -p "read_verilog -sv \"${CMAKE_SOURCE_DIR}/rtl/stage_waveform_generator.sv\""
        -p "read_verilog -sv \"${CMAKE_SOURCE_DIR}/rtl/synth.sv\""
        -p "synth_ice40 -top synth -json octane.json"
        2>&1 | tee yosys.log

    OUTPUT octane.json
    DEPENDS
        "${CMAKE_SOURCE_DIR}/rtl/spi.sv"
        "${CMAKE_SOURCE_DIR}/rtl/stage_envelope_attenuator.sv"
        "${CMAKE_SOURCE_DIR}/rtl/stage_modulator.sv"
        "${CMAKE_SOURCE_DIR}/rtl/stage_phase_accumulator.sv"
        "${CMAKE_SOURCE_DIR}/rtl/stage_sample_generator.sv"
        "${CMAKE_SOURCE_DIR}/rtl/stage_waveform_generator.sv"
        "${CMAKE_SOURCE_DIR}/rtl/synth.sv"
        "${CMAKE_SOURCE_DIR}/rtl/synth.svh"

    COMMENT "Running FPGA design synthesis"
    USES_TERMINAL
)


add_custom_target(implementation DEPENDS octane.asc)
add_custom_command(
    # TODO: --freq option?
    COMMAND nextpnr-ice40
        --up5k --package sg48
        --pcf "${CMAKE_CURRENT_LIST_DIR}/octane.pcf" --json octane.json
        --asc octane.asc
        2>&1 | tee nextpnr.log

    OUTPUT octane.asc
    DEPENDS
        octane.json
        octane.pcf
    COMMENT "Running FPGA design implementation"
    USES_TERMINAL
)


# TODO: Better way?
add_custom_target(timing

    # Current timing target: 35 MHz (subject to change)
    # That would be enough for two pipeline stalls at 44.1 kHz output
    COMMAND icetime -mit octane.asc -c 35 | tee icetime.log

    DEPENDS octane.asc
    COMMENT "Running FPGA implementation timing"
    USES_TERMINAL
)


add_custom_target(bitstream DEPENDS octane.bin)
add_custom_command(
    COMMAND icepack octane.asc octane.bin | tee icepack.log

    OUTPUT octane.bin
    DEPENDS octane.asc

    COMMENT "Generating FPGA bitstream"
    USES_TERMINAL
)


add_custom_target(program
    COMMAND iceprog octane.bin
    DEPENDS bitstream
    COMMENT "Programming development board"
    USES_TERMINAL
)

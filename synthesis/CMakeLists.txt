

add_custom_target(fpga_synthesis DEPENDS octane.json)
add_custom_command(
    COMMAND yosys
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
)


add_custom_target(fpga_implementation DEPENDS octane.asc)
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
)


add_custom_target(fpga_bitstream DEPENDS octane.bin)
add_custom_command(
    COMMAND icepack octane.asc octane.bin

    OUTPUT octane.bin
    DEPENDS octane.asc

    COMMENT "Generating FPGA bitstream"
)


# set (rtl_sources

# )


# TODO: Move into own file

# TODO: Add Verilator as a first step to lint SystemVerilog source files

# TODO: Use configure_file to inject the paths to source files instead?

# TODO: Read stdout of all of these to find errors? Tee them to log files?
# Or are they already duplicated in the output?

# add_custom_target(create_radiant_project DEPENDS octane.rdf)
# add_custom_command(
#     COMMAND ${CMAKE_COMMAND} -E env "HDL_DIR=${CMAKE_SOURCE_DIR}/hdl" radiantc ${CMAKE_CURRENT_SOURCE_DIR}/create_project.tcl
#     DEPENDS create_project.tcl
#     OUTPUT octane.rdf
#     COMMENT "Creating Lattice Radiant project"
#     USES_TERMINAL
# )

# # TODO: Read and check .srf log file for errors
# add_custom_target(synthesize_design DEPENDS octane_impl_1_syn.udb)
# add_custom_command(
#     COMMAND radiantc ${CMAKE_CURRENT_SOURCE_DIR}/synthesize_design.tcl
#     DEPENDS octane.rdf
#     OUTPUT octane_impl_1_syn.udb octane_impl_1.srf
#     COMMENT "Running FPGA synthesis"
#     USES_TERMINAL
# )

# # TODO: Read and check .tw1 file for timing errors
# add_custom_target(map_design DEPENDS octane_impl_1_map.udb)
# add_custom_command(
#     COMMAND radiantc ${CMAKE_CURRENT_SOURCE_DIR}/map_design.tcl
#     DEPENDS octane_impl_1_syn.udb
#     OUTPUT octane_impl_1_map.udb octane_impl_1.tw1
#     COMMENT "Running FPGA map"
#     USES_TERMINAL
# )

# add_custom_target(place_and_route_design DEPENDS octane_impl_1_par.udb)
# add_custom_command(
#     COMMAND radiantc ${CMAKE_CURRENT_SOURCE_DIR}/place_and_route_design.tcl
#     DEPENDS octane_impl_1_map.udb
#     OUTPUT octane_impl_1_par.udb
#     COMMENT "Running FPGA place-and-route"
#     USES_TERMINAL
# )

# Why duplicate work that Radiant keeps track of itself?
add_custom_target(fpga_synthesis DEPENDS idk)
add_custom_command(
    COMMAND
        ${CMAKE_COMMAND}
        -E env "HDL_DIR=${CMAKE_SOURCE_DIR}/hdl" "SYNTHESIS_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
        radiantc ${CMAKE_CURRENT_SOURCE_DIR}/generate_bitstream.tcl

    # DEPENDS

    OUTPUT idk
    COMMENT "Generating FPGA design"
    USES_TERMINAL
)

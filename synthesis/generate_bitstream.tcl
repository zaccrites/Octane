
set HDL_DIR $::env(HDL_DIR)
set SYNTHESIS_DIR $::env(SYNTHESIS_DIR)

# puts ${HDL_DIR}
# puts ${SYNTHESIS_DIR}

# Create base project if it doesn't already exist
if { [catch {prj_open "octane.rdf"}] } {
    prj_create -name "octane" -impl "impl_1" -dev iCE40UP5K-SG48I -performance "High-Performance_1.2V" -synthesis "synplify"
}

prj_add_source ${HDL_DIR}/double_register.sv
prj_add_source ${HDL_DIR}/spi.sv
prj_add_source ${HDL_DIR}/stage_envelope_attenuator.sv
prj_add_source ${HDL_DIR}/stage_modulator.sv
prj_add_source ${HDL_DIR}/stage_phase_accumulator.sv
prj_add_source ${HDL_DIR}/stage_sample_generator.sv
prj_add_source ${HDL_DIR}/stage_waveform_generator.sv
prj_add_source ${HDL_DIR}/synth.sv
prj_add_source ${SYNTHESIS_DIR}/design_constraints.pdc
prj_save


# Synthesis
prj_run Synthesis -impl impl_1

puts "FINISHED SYNTHESIS"

# # Map
# prj_run Map -impl impl_1

# # Place and Route
# prj_run PAR -impl impl_1

# # prj_open "octane_test3.rdf"

# # Power calculation setup
# pwc_new_project "power_calculator_project.pcf"
# # pwc_open_project "power_calculator_project.pcf"
# pwc_set_processtype "Worst"
# pwc_set_thetaja -board "Small Board" -heatsink "No Heat Sink" -airflow "0 LFM"
# pwc_set_freq COMBINATORIAL 96
# pwc_set_freq i_Clock_c 96
# pwc_gen_report "pwc_report.txt"
# # pwc_save_project "power_calculator_project.pcf"
# # pwc_save_project

# # Generate bitstream
# prj_run Export -impl impl_1




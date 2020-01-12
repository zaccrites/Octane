
prj_create -name "octane" -impl "impl_1" -dev iCE40UP5K-SG48I -performance "High-Performance_1.2V" -synthesis "synplify"

set HDL_DIR $::env(HDL_DIR)

prj_add_source ${HDL_DIR}/double_register.sv
prj_add_source ${HDL_DIR}/spi.sv
prj_add_source ${HDL_DIR}/stage_envelope_attenuator.sv
prj_add_source ${HDL_DIR}/stage_modulator.sv
prj_add_source ${HDL_DIR}/stage_phase_accumulator.sv
prj_add_source ${HDL_DIR}/stage_sample_generator.sv
prj_add_source ${HDL_DIR}/stage_waveform_generator.sv
prj_add_source ${HDL_DIR}/synth.sv

prj_add_source "/home/zac/synth/OLD/synthesis/design_constraints.pdc"

# TODO: Set implementation's top module?

prj_save

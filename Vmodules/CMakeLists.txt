
# Verilog modules listed here will be "Verilated" into their own
# library target suitable for linking into an executable test target.

if (APPLE)
    # MacOS
    set(VERILATOR_INCLUDE_DIR "/usr/local/share/verilator/include")
elseif (UNIX)
    # Linux
    set(VERILATOR_INCLUDE_DIR "/usr/share/verilator/include")
else()
    # TODO: Try to find it using find_package or something?
    message(FATAL_ERROR "I don't know where to find the Verilator include files!")
endif()


function(verilate_cmake)
    cmake_parse_arguments(VERILATE_CMAKE "" "VERILOG_MODULE" "" ${ARGN})
    get_filename_component(VERILOG_MODULE_NAME "${VERILATE_CMAKE_VERILOG_MODULE}" NAME_WE)
    set(OUTPUT_CMAKE_FILE_PATH "${CMAKE_SOURCE_DIR}/Vmodules/V${VERILOG_MODULE_NAME}.cmake")
    if (NOT EXISTS ${OUTPUT_CMAKE_FILE_PATH})
        execute_process(
            COMMAND
                python "${CMAKE_CURRENT_LIST_DIR}/verilate_cmake.py"
                "${CMAKE_SOURCE_DIR}/rtl/${VERILATE_CMAKE_VERILOG_MODULE}"
                "-I${PROJECT_SOURCE_DIR}/rtl"
                "-I${PROJECT_SOURCE_DIR}/rtl/include"
                --verilator-output-dir "${CMAKE_CURRENT_BINARY_DIR}/verilator_output/V${VERILOG_MODULE_NAME}"
                --verilator-include-dir "${VERILATOR_INCLUDE_DIR}"
                --cmake "${OUTPUT_CMAKE_FILE_PATH}"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()
    include(${OUTPUT_CMAKE_FILE_PATH})
endfunction()


verilate_cmake(VERILOG_MODULE synth.sv)


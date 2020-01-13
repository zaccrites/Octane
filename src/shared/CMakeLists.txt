
add_library(shared
    test.cpp
)

target_compile_options(shared PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)

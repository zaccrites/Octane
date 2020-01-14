
add_library(printf
    "printf/printf.c"
)

target_compile_options(printf PRIVATE
    -Os
    ${TARGET_COMPILE_FLAGS}
)

target_include_directories(printf SYSTEM PUBLIC
    "printf"
)

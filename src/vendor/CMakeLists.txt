
add_library(printf
    "printf/printf.c"
)

# # TODO: Address this. Do I need some common options?
# target_compile_options(printf PRIVATE
#     -mcpu=cortex-m4
#     -mthumb
#     -mabi=aapcs
#     # -mfpu=fpv4-sp-d16       # TODO (these options don't seem to be right)
#     # -mfloat-abi=hard
#     -mlong-calls

#     -Og
#     -g3
#     # -O2
# )

target_compile_options(printf PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)

target_include_directories(printf SYSTEM PUBLIC
    "printf"
)

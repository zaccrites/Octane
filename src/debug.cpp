
#include <iostream>
#include <stdexcept>
#include <execinfo.h>

#include "debug.hpp"


// https://stackoverflow.com/a/3356421

void terminate_handler()
{
    // On using addr2line:
    // https://stackoverflow.com/a/51597989
    //
    // Example: addr2line -afC -e ../synth-build/synth +0x69af

    std::cerr << "Printing stack trace of uncaught exception: \n";
    std::exception_ptr eptr = std::current_exception();
    try
    {
        // assert(eptr);
        std::rethrow_exception(eptr);
    }
    catch (const std::exception& e)
    {
        std::cerr << "what(): " << e.what() << "\n";
    }
    std::cerr << "------------------------------------------------------------"
              << std::endl;

    void *trace_elems[20];
    int trace_elem_count = backtrace(trace_elems, 20);
    char **stack_syms = backtrace_symbols(trace_elems, trace_elem_count);
    for (int i = 0; i < trace_elem_count; i++)
    {
        std::cout << stack_syms[trace_elem_count - i - 1] << std::endl;
    }
    free(stack_syms);

    std::cerr << "------------------------------------------------------------"
              << std::endl;

    exit(1);
}


void setup_debug_handlers()
{
    std::set_terminate(terminate_handler);
}

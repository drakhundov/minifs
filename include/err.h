#pragma once

#include <stdarg.h>

// * Prints error message using `perror` and logs it.
void err_exit(const char* err_msg, ...);
void err_exit_with_arg(const char* err_msg, void* arg, ...);
void set_final_callback(void (*callback)(void*));

#pragma once

#include <stdarg.h>

// * Prints error message using `perror`
// * and logs it.
void err_exit(const char *err_msg, ...);

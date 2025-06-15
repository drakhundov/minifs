#ifndef ERR_H
#define ERR_H

#include <stdarg.h>

// * Prints error message using `perror`
// * and logs it.
void err_exit(const char *err_msg, ...);

#endif // ERR_H
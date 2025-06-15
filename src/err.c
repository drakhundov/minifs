#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "err.h"
#include "logging.h"

void err_exit(const char* err_msg, ...) {
	va_list args;
	va_start(args, err_msg);
	char tmp[1024];
	vsnprintf(tmp, sizeof(tmp), err_msg, args);
	va_end(args);
	perror(tmp);
	logMsg(ERROR_LOG, tmp);
	exit(1);
}

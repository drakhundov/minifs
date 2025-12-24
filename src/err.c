#include "err.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "logging.h"

// --------------- LOCAL ---------------

static void (*final_callback)(void*) = NULL;

static void err_exit_general(const char* err_msg, void* arg, va_list args) {
    char tmp[1024];
    vsnprintf(tmp, sizeof(tmp), err_msg, args);
    perror(tmp);
    logMsg(ERROR_LOG, tmp);

    if (final_callback != NULL) {
        final_callback(arg);
    }

    logMsg(INFO_LOG, "err_exit: exiting");
    exit(1);
}

// -------------------------------------

void err_exit(const char* err_msg, ...) {
    va_list args;
    va_start(args, err_msg);
    err_exit_general(err_msg, NULL, args);
    va_end(args);
}

void err_exit_with_arg(const char* err_msg, void* arg, ...) {
    va_list args;
    va_start(args, arg);
    err_exit_general(err_msg, arg, args);
    va_end(args);
}

void set_final_callback(void (*callback)(void*)) {
    if (final_callback != NULL) {
        logMsg(WARN_LOG, "set_final_callback: a final callback function has already been set");
        return;
    }
    final_callback = callback;
}

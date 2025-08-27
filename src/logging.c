#include "logging.h"

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Owner: rw
// Group & others: r
#define LOGPERMS 0644

// Supposed to be inaccessible
// anywhere else.
static int logfd;
static bool print_logs = true;  // Print logs to the stdout by default.

void set_print_logs(bool toggle) { print_logs = toggle; }

void init_logs(const char *filename, int mode) {
  logfd = open(filename, mode, LOGPERMS);
  if (logfd == -1) {
    perror("Failed to open the log file.");
    exit(1);
  }
  logMsg(INFO_LOG, "Started logging");
}

// ! This is a helper function.
// ! It is not supposed to be used
// ! outside of the logging implementation.
char *_get_timestamp() {
  time_t now;
  time(&now);
  struct tm *local = localtime(&now);
  char *timestamp = (char *)malloc(20);
  if (timestamp == NULL) {
    logMsg(ERROR_LOG, "Failed to allocate memory for timestamp");
    perror("Failed to allocate memory for timestamp.");
    exit(1);
  }
  sprintf(timestamp, "%02d-%02d-%04d %02d:%02d:%02d", local->tm_mday, local->tm_mon + 1,
          local->tm_year + 1900, local->tm_hour, local->tm_min, local->tm_sec);
  return timestamp;
}

void end_logs() {
  logMsg(INFO_LOG, "Finished logging");
  close(logfd);
}

void logMsg(Logtype log_type, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char tmp[1024];
  char *type_msg = (log_type == ERROR_LOG) ? "ERROR" : (log_type == WARN_LOG) ? "WARN" : "INFO";
  vsnprintf(tmp, sizeof(tmp), fmt, args);
  va_end(args);
  char msg[1024];
  sprintf(msg, "[%s] [%s] %s\n", _get_timestamp(), type_msg, tmp);
  write(logfd, msg, strlen(msg));
  if (print_logs) {
    puts(msg);
  }
  fsync(logfd);  // Ensure the log message is written to disk.
}

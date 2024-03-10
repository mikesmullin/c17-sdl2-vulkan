#include <stdarg.h>
#include <stdio.h>

static const char* LOG_FILE_PATH = "out.log";
static const char* FILE_TRUNCATE = "w";
static const char* FILE_APPEND = "a";

void logit(const char* line, ...) {
  static FILE* fh;
  if (NULL == fh) {
    // truncate file
    fopen_s(&fh, LOG_FILE_PATH, FILE_TRUNCATE);
    fclose(fh);
  }

  // append file
  fopen_s(&fh, LOG_FILE_PATH, FILE_APPEND);
  va_list myargs;
  va_start(myargs, line);
  vfprintf(fh, line, myargs);
  fclose(fh);

  vfprintf(stdout, line, myargs);
  va_end(myargs);
}
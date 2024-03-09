#include "Shader.h"

#include <stdio.h>

#include "Base.h"

static const char* READ = "rb";

u64 Shader__ReadFile(char* buf, const char* file) {
  LOG_INFOF("reading shader file: %s", file)

  FILE* fh;
  ASSERT(0 == fopen_s(&fh, file, READ))
  ASSERT(NULL != fh)

  // determine file length
  fseek(fh, 0, SEEK_END);
  u32 len = ftell(fh);
  rewind(fh);

  // read file into buffer
  u64 bytes_read = fread(buf, 1, len, fh);

  // security: guarantee null-termination
  buf[bytes_read] = '\0';

  fclose(fh);

  ASSERT_EQUAL(bytes_read, len, "Error reading file %s", file)

  return bytes_read;
}
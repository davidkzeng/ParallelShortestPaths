#include "util.h"

void outmsg(char* fmt, ...) {
  va_list ap;
  bool got_newline = fmt[strlen(fmt)-1] == '\n';
  va_start(ap,fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  if (!got_newline)
    fprintf(stderr, "\n");
}

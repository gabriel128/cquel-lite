#ifndef __cquel__
#define __cquel__

#include <stdlib.h>

#define size_of_attr(Struct, Attr) sizeof(((Struct*)0)->Attr)

#define TABLE_MAX_PAGES 100
#define PAGE_SIZE 4096

typedef struct {
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

#endif

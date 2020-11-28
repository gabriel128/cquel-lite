#ifndef __pager_header__
#define __pager_header__

#include <stdint.h>
#include "cquel.h"

typedef struct {
  uint32_t num_rows;
  void* rows;
} Page;

typedef struct {
  int fd;
  uint32_t file_length;
  void* pages[TABLE_MAX_PAGES];
} Pager;

Pager* pager_open(const char* filename);
void* get_page(Pager* pager, uint32_t page_num);
void flush_page(Pager* pager, uint32_t page_num);

#endif

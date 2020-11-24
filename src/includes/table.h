#ifndef __table_header__
#define __table_header__

#include <stdint.h>
#include <stdlib.h>
#include "cquel.h"

typedef struct {
  uint32_t num_rows;
  void* pages[TABLE_MAX_PAGES];
} Table;

Table* new_table();
void free_table(Table* table);

#endif

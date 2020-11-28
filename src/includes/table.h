#ifndef __table_header__
#define __table_header__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "cquel.h"
#include "pager.h"

typedef struct {
  uint32_t num_rows;
  Pager* pager;
} Table;

Table* db_open(const char* filename);
void db_close(Table* table);

#endif

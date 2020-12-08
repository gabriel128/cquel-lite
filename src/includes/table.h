#ifndef __table_header__
#define __table_header__

#include "cquel.h"
#include "pager.h"
#include "tuples.h"

typedef struct {
  uint32_t num_rows; // deprecated
  TableHeader header;
  Pager* pager;
} Table;

Table* db_open(char* filename);
void db_close(Table* table);

#endif

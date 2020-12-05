#ifndef __table_header__
#define __table_header__

#include "cquel.h"
#include "pager.h"

typedef struct {
  Tuple* tuple;
  size_t num_tuples;
} ResultSet;

typedef struct {
  uint32_t num_rows; // deprecated
  TableHeader header;
  Pager* pager;
} Table;

void print_row(Row* row);
void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);
void* insert_row_slot(Table* table, uint32_t row_num);
void* read_row_slot(Table* table, uint32_t row_num);
Table* db_open(char* filename);
void db_close(Table* table);

#endif

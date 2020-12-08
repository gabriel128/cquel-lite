#include "includes/table.h"
#include <unistd.h>

Table* db_open(char* filename) {
  Pager* pager = pager_open(filename);
  Table* table = calloc(1, sizeof(Table));

  table->pager = pager;
  table->num_rows = 0; // Deprecated

  // TODO read header from file
  TableHeader t_header;

  if (FIRST_PAGE_OFFSET > pager->file_length) {
    t_header.page_qty = 0;
  } else {
    t_header.page_qty = (pager->file_length - FIRST_PAGE_OFFSET) / PAGE_SIZE;
  }
  t_header.table_name = filename;

  table->header = t_header;

  printf("Table Name: %s\n", table->header.table_name);
  return table;
}

void db_close(Table* table) {
  Pager* pager = table->pager;

  // Flush pages

  int closed = fclose(pager->fp);
  if (closed == -1) {
    printf("Couldn't close table file\n");
    exit(EXIT_FAILURE);
  }
  free(pager);
  free(table);
}

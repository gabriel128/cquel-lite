#include "includes/table.h"
#include <unistd.h>

Table* db_open(const char* filename) {
  Pager* pager = pager_open(filename);
  Table* table = malloc(sizeof(Table));

  table->pager = pager;
  table->num_rows = pager->file_length / ROW_SIZE;

  printf("Initial Num of rows %u\n", table->num_rows);
  return table;
}

void db_close(Table* table) {
  Pager* pager = table->pager;

  // Ensure that when we close the db everything has been flushed
  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    if(pager->pages[i] != NULL) {
      flush_page(pager, i);
      free(pager->pages[i]);
      pager->pages[i] = NULL;
    }
  }

  int closed = close(pager->fd);
  if (closed == -1) {
    printf("Couldn't close table file\n");
    exit(EXIT_FAILURE);
  }
  free(table);
}

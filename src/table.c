#include "includes/table.h"

Table* new_table() {
  Table* table = malloc(sizeof(Table));
  table->num_rows = 0;

  for(uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    table->pages[i] = NULL;
  }
  return table;
}

void free_table(Table* table) {
  for (uint32_t i = 0; table->pages[i]; i++) {
    free(table->pages[i]);
  }
  free(table);
}

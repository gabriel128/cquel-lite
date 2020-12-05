#include "includes/table.h"
#include <unistd.h>

const size_t ID_SIZE = size_of_attr(Row,id);
const size_t USERNAME_SIZE = size_of_attr(Row, username);
const size_t EMAIL_SIZE = size_of_attr(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const size_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

Table* db_open(char* filename) {
  Pager* pager = pager_open(filename);
  Table* table = malloc(sizeof(Table));

  table->pager = pager;
  table->num_rows = pager->file_length / ROW_SIZE;

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

  uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;
  // Ensure that when we close the db everything has been flushed
  for (uint32_t i = 0; i < num_full_pages; i++) {
    if(pager->pages[i] != NULL) {
      flush_page_old(pager, i, PAGE_SIZE);
      free(pager->pages[i]);
      pager->pages[i] = NULL;
    }
  }

  uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;

  if (num_additional_rows > 0) {
    uint32_t page_num = num_full_pages;
    if (pager->pages[page_num] != NULL) {

      flush_page_old(pager, page_num, num_additional_rows * ROW_SIZE);

      free(pager->pages[page_num]);
      pager->pages[page_num] = NULL;
    }
  }

  int closed = fclose(pager->fp);
  if (closed == -1) {
    printf("Couldn't close table file\n");
    exit(EXIT_FAILURE);
  }
  free(pager);
  free(table);
}

void print_row(Row* row) {
  printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

void serialize_row(Row* source, void* destination) {
  memcpy((uint8_t*)destination + ID_OFFSET, &(source->id), ID_SIZE);
  memcpy((uint8_t*)destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
  memcpy((uint8_t*)destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination) {
  memcpy(&(destination->id), (uint8_t*)source + ID_OFFSET, ID_SIZE);
  memcpy(&(destination->username), (uint8_t*)source + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&(destination->email), (uint8_t*)source + EMAIL_OFFSET, EMAIL_SIZE);
}

void* insert_row_slot(Table* table, uint32_t row_num) {

  /* page->header.num_rows++; */
  /* uint32_t row_offset = row_num % ROWS_PER_PAGE; */
  /* uint32_t byte_offset = row_offset * ROW_SIZE; */
  /* return (uint8_t*)page + sizeof(PageHeader) + byte_offset; */
}

void* read_row_slot(Table* table, uint32_t row_num) {
  uint32_t page_num = row_num / ROWS_PER_PAGE;
  void* page = get_page(table->pager, page_num);
  uint32_t row_offset = row_num % ROWS_PER_PAGE;
  uint32_t byte_offset = row_offset * ROW_SIZE;
  /* printf("row slot address %p\n", (uint8_t*)page); */
  /* printf("Byte offset %u\n", byte_offset); */
  /* printf("Page + byte_offset %p\n", (uint8_t*)page + byte_offset); */
  return (uint8_t*)page + byte_offset;
}

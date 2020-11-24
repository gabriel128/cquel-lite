#include "includes/row.h"
#include "includes/table.h"

const size_t ID_SIZE = size_of_attr(Row,id);
const size_t USERNAME_SIZE = size_of_attr(Row, username);
const size_t EMAIL_SIZE = size_of_attr(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const size_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

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

void* row_slot(Table* table, uint32_t row_num) {
  uint32_t page_num = row_num / ROWS_PER_PAGE;
  void* page = table->pages[page_num];

  if (page == NULL) {
    table->pages[page_num] = calloc(1, PAGE_SIZE);

    assert(table->pages[page_num]);

    page = table->pages[page_num];
  }

  uint32_t row_offset = row_num % ROWS_PER_PAGE;
  uint32_t byte_offset = row_offset * ROW_SIZE;
  return (uint8_t*)page + byte_offset;
}

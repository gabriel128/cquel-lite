#include "includes/cquel.h"
#include "includes/pager.h"
#include "includes/table.h"
#include "includes/statements.h"

MetaCommandResult do_meta_command(InputBuffer* input_buffer, Table* table) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    db_close(table);
    exit(EXIT_SUCCESS);
  } else {
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}
InputBuffer* new_input_buffer() {
  InputBuffer* input_buffer = (InputBuffer*) malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;

  return input_buffer;
}

void print_prompt() {
  printf("cquel> ");
}

void read_input(InputBuffer* input_buffer) {
  ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

  if (bytes_read <= 0) {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }

  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(InputBuffer* ib) {
  free(ib->buffer);
  free(ib);
}

PageHeader new_page_header(Table table) {
  printf("Page qty was %d\n", table.header.page_qty);
  PageHeader page_header = {
    .page_id = table.header.page_qty,
    .dirty = false,
    .lower_limit = sizeof(PageHeader),
    .upper_limit = PAGE_SIZE,
  };

  return page_header;
}

void initialize_page(Page page) {
  for (size_t i = 0; i < PAGE_SIZE; i++) {
    page[i] = 0;
  }
}

void flush_page(Page page, PageHeader* header, Pager* pager) {
  if(!(header->dirty)) {
    printf("Tried to Flush a non dirty page %ld\n", FIRST_PAGE_OFFSET);
    return;
  }

  FILE* fp = pager->fp;

  printf("First Page offset is %ld\n", FIRST_PAGE_OFFSET);
  printf("Writting in page offset %d, page number %d\n", page_position(header), header->page_id);

  fseek(fp, page_position(header), SEEK_SET);
  size_t bytes_written = fwrite(page, PAGE_SIZE, 1, fp);

  if (bytes_written <= 0) {
    printf("Error on flushing page to disk\n");
    exit(EXIT_FAILURE);
  }
}

void fetch_page(Pager* pager, PageHeader* page_header, LinePointer* line_pointer, uint32_t page_id) {
  Page page_read;

  uint32_t page_offset = page_position_by_id(page_id);
  FILE* fp = pager->fp;

  fseek(fp, page_offset, SEEK_SET);
  fread(page_read, PAGE_SIZE, 1, fp);

  memcpy(page_header, page_read, sizeof(PageHeader));
  memcpy(line_pointer, page_read + page_header->lower_limit, sizeof(LinePointer));
}

uint32_t get_free_page_space(PageHeader* header) {
  if (header->lower_limit > header->upper_limit) {
    return 0;
  } else {
    return header->upper_limit - header->lower_limit;
  }
}

/* LinePointer* line_pointer_root_from_page(Page page) { */
/*   return (header->lower_limit - sizeof(PageHeader)) / sizeof(LinePointer); */
/* } */

uint32_t line_pointers_qty(PageHeader* header) {
  return (header->lower_limit - sizeof(PageHeader)) / sizeof(LinePointer);
}

/* ResultSet read_rows(Page page, LinePointer* lp) { */
/*   Page page_read; */

/*   uint32_t page_offset = page_position_by_id(page_id); */
/*   FILE* fp = pager->fp; */

/*   fseek(fp, page_offset, SEEK_SET); */
/*   fread(page_read, PAGE_SIZE, 1, fp); */

/*   /\* memcpy(&read_row, FIRST_PAGE_OFFSET + page_read + read_lp.tuple_offset, read_lp.tuple_length); *\/ */


/* } */


void learning(Table* table) {
  /* printf("Size of line pointer %zu\n", sizeof(LinePointer)); */
  /* printf("Size of bool %zu\n", sizeof(bool)); */
  /* printf("Size of uint32 %zu\n", sizeof(uint32_t)); */
  /* printf("Size of page header %zu\n", sizeof(PageHeader)); */
  /* printf("Size of page header %zu\n", sizeof(Tuple)); */

  PageHeader page_header = new_page_header(*table);
  TupleHeader rhd = {1, 0};
  Tuple row = {rhd, 1, "I'm a lot of data"};
  Page page;
  initialize_page(page);

  unsigned long tuple_offset = PAGE_SIZE - sizeof(Tuple);
  LinePointer lp = {tuple_offset, 0x00, sizeof(Tuple)};

  page_header.lower_limit = page_header.lower_limit + sizeof(LinePointer);
  page_header.upper_limit = page_header.upper_limit - sizeof(Tuple);

  memcpy(page, &page_header, sizeof(PageHeader)); // cpy Header
  memcpy(page + page_header.lower_limit, &lp, sizeof(LinePointer)); // cpy lps
  memcpy(page + lp.tuple_offset, &row, lp.tuple_length); // cpy rows */

  page_header.dirty = true;

  /* flush_page(page, &page_header, table->pager); */

  PageHeader* read_header = malloc(sizeof(PageHeader));
  LinePointer* read_lp = malloc(sizeof(LinePointer));

  fetch_page(table->pager, read_header, read_lp, 0);
  /* Tuple read_row; */

  printf("Free space: %d \n", get_free_page_space(read_header));
  printf("LinePointer qty: %d \n", line_pointers_qty(read_header));

  printf("Page header page_id: %d, page_full: %d, ll: %d, ul: %d \n", read_header->page_id, read_header->page_full, read_header->lower_limit, read_header->upper_limit);
  printf("LinePointer tuple_offset: %d, flags: %x, tuple_length: %d\n",
         read_lp->tuple_offset, read_lp->flags, read_lp->tuple_length);
  /* printf("First Tuple data: %s, id: %d, row header offset: %d\n", */
  /*        read_row.data, read_row.id, read_row.header.lp_offset); */

  free(read_header);
  free(read_lp);
  fclose(table->pager->fp);
}

/* for(uint32_t i = 0; i < sizeof(PageHeader); i++) { */
/*   header[i] = page_read[i]; */
/* } */

/* PageHeader* ph = (PageHeader*)header; */

/* char record[sizeof(Tuple)*ph->num_rows]; */

/* for(uint32_t i = sizeof(PageHeader); i < sizeof(Tuple)*ph->num_rows; i++) { */
/*   record[i - sizeof(PageHeader)] = page_read[i]; */
/* } */

/* Tuple* r = (Tuple*)record; */

/* memcpy(ph, page_read, sizeof(PageHeader)); */
/* Page* p = calloc(1, sizeof(Page)); */

/* printf("Page Row header mapping offset: %d, page_num: %d \n", r->header.offset, r->header.page_num); */
/* printf("Page Rows mapping id: %d, username: %s \n", r->id, r->data); */
/* printf("Page header num_rows: %d, dirty: %d \n", page_read->header.num_rows, page_read->header.dirty); */
/* printf("Page Row header offset: %d, page_id: %d \n", page_read->rows[0].header.offset, page_read->rows[0].header.page_id); */
/* printf("Page Rows id: %d, username: %s \n", page_read->rows[2].id, page_read->rows[2].data); */

/* free(ph); */
int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Needs database file\n");
    exit(EXIT_FAILURE);
  }

  char* filename = argv[1];

  if (strnlen(filename, TABLE_NAME_SIZE) > TABLE_NAME_SIZE) {
    printf("File name too long\n");
    exit(EXIT_FAILURE);
  }

  Table* table = db_open(filename);

  learning(table);
  /* InputBuffer* input_buffer = new_input_buffer(); */
  /* while(true) { */
  /*   print_prompt(); */
  /*   read_input(input_buffer); */

  /*   if (input_buffer->buffer[0] == '.') { */
  /*     switch (do_meta_command(input_buffer, table)) { */
  /*     case (META_COMMAND_SUCCESS): */
  /*       continue; */
  /*     case (META_COMMAND_UNRECOGNIZED_COMMAND): */
  /*       printf("Unrecognized Command '%s' .\n", input_buffer->buffer); */
  /*       continue; */

  /*     } */
  /*   } */

  /*   Statement statement; */

  /*   switch (prepare_statement(input_buffer, &statement)) { */
  /*     case PREPARE_SUCCESS: */
  /*       break; */
  /*     case PREPARE_UNRECOGNIZED_STATEMENT: */
  /*       printf("Unrecognized keyword at start of '%s'. \n", input_buffer->buffer); */
  /*       continue; */
  /*     case PREPARE_STM_SYNTAX_ERROR: */
  /*       printf("Syntax Error on '%s'. \n", input_buffer->buffer); */
  /*       continue; */
  /*   } */

  /*   switch (execute_statement(&statement, table)) { */
  /*       case EXECUTE_SUCCESS: */
  /*         printf("Executed.\n"); */
  /*         break; */
  /*       case EXECUTE_TABLE_FULL: */
  /*         printf("Error: Table Full.\n"); */
  /*         break; */
  /*       case EXECUTE_VALIDATION_FAILURE: */
  /*         printf("Error: Validation Failure.\n"); */
  /*         break; */
  /*       case EXECUTE_FAILURE: */
  /*         printf("Error: Failure.\n"); */
  /*         break; */
  /*   } */
  /* } */
}

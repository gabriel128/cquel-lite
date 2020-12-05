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

  memcpy(page, header, sizeof(PageHeader)); // ensure page has the  latest header

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

uint32_t line_pointers_qty(PageHeader* header) {
  return (header->lower_limit - sizeof(PageHeader)) / sizeof(LinePointer);
}

void fetch_page(Page page, Pager* pager, PageHeader* page_header, LinePointer* line_pointer, uint32_t page_id) {
  uint32_t page_offset = page_position_by_id(page_id);
  FILE* fp = pager->fp;

  fseek(fp, page_offset, SEEK_SET);
  fread(page, PAGE_SIZE, 1, fp);

  memcpy(page_header, page, sizeof(PageHeader));
  memcpy(line_pointer, page + sizeof(PageHeader), sizeof(LinePointer)*line_pointers_qty(page_header));
}

uint32_t get_free_page_space(PageHeader* header) {
  if (header->lower_limit > header->upper_limit) {
    return 0;
  } else {
    return header->upper_limit - header->lower_limit;
  }
}

uint32_t page_has_space_for(PageHeader* header, size_t nbytes) {
  return get_free_page_space(header) >= nbytes;
}

LinePointer* line_pointer_root_from_page(Page page) {
  return (LinePointer*)(page + sizeof(LinePointer));
}

ResultSet* read_rows(Page page, LinePointer* lp, PageHeader* header) {
  ResultSet* result = calloc(1, sizeof(ResultSet));
  result->num_tuples = line_pointers_qty(header);
  result->tuples = calloc(result->num_tuples, sizeof(Tuple));

  for(size_t i = 0; i < result->num_tuples; i++) {
    unsigned int tuple_offset = lp[i].tuple_offset;
    result->tuples[i] = *((Tuple*)(page + tuple_offset));
  }

  return result;
}

bool insert_row(Page page, PageHeader* page_header, char* data) {
  if (!page_has_space_for(page_header, sizeof(Tuple)+ sizeof(LinePointer))) {
    printf("Not enough space for tuple \n");

    return false;
  }

  unsigned long tuple_offset = page_header->upper_limit - sizeof(Tuple);
  LinePointer lp = {tuple_offset, 0x00, sizeof(Tuple)};

  TupleHeader tuple_header = {page_header->page_id, line_pointers_qty(page_header)};

  Tuple row;
  row.header = tuple_header;
  row.id = tuple_header.lp_offset;
  strncpy(row.data, data, strnlen(data, TUPLE_SIZE));

  memcpy(page + lp.tuple_offset, &row, lp.tuple_length); // cpy rows */
  page_header->upper_limit -= sizeof(Tuple);

  memcpy(page + page_header->lower_limit, &lp, sizeof(LinePointer)); // cpy lps
  page_header->lower_limit += sizeof(LinePointer);

  page_header->dirty = true;

  return true;
}

void print_tuple(Tuple *row) {
  printf("Tuple data: %s, id: %d, row header offset: %d\n", row->data, row->id, row->header.lp_offset);
}

void size_of_stuff() {
  printf("Size of line pointer %zu\n", sizeof(LinePointer));
  printf("Size of bool %zu\n", sizeof(bool));
  printf("Size of uint32 %zu\n", sizeof(uint32_t));
  printf("Size of page header %zu\n", sizeof(PageHeader));
  printf("Size of page header %zu\n", sizeof(Tuple));
}

void learning(Table* table) {
  PageHeader page_header = new_page_header(*table);

  Page page;
  initialize_page(page);

  for(int i = 0; i < 30; i++)
    insert_row(page, &page_header, "Blah");

  flush_page(page, &page_header, table->pager);

  PageHeader* read_header = malloc(sizeof(PageHeader));
  LinePointer* read_lp = malloc(sizeof(LinePointer)*line_pointers_qty(read_header));

  Page read_page;
  initialize_page(read_page);

  fetch_page(read_page, table->pager, read_header, read_lp, 0);

  ResultSet* result = read_rows(read_page, read_lp, read_header);

  for (size_t i = 0; i < result->num_tuples; i++) {
    Tuple read_row = result->tuples[i];
    print_tuple(&read_row);
  }

  printf("Free space: %d \n", get_free_page_space(read_header));
  printf("LinePointer qty: %d \n", line_pointers_qty(read_header));
  printf("Page header page_id: %d, page_full: %d, ll: %d, ul: %d \n", read_header->page_id, read_header->page_full, read_header->lower_limit, read_header->upper_limit);
  /* printf("LinePointer tuple_offset: %d, flags: %x, tuple_length: %d\n", */
  /*        read_lp->tuple_offset, read_lp->flags, read_lp->tuple_length); */
  printf("LinePointer 2 tuple_offset: %d, flags: %x, tuple_length: %d\n",
         (read_lp + 1)->tuple_offset, (read_lp + 1)->flags, (read_lp + 1)->tuple_length);

  free(result);
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

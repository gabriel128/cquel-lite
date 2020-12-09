#include "includes/cquel.h"
/* #include "includes/pager.h" */
/* #include "includes/tuples.h" */
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

void size_of_stuff() {
  printf("Size of line pointer %zu\n", sizeof(LinePointer));
  printf("Size of TableHeader %zu\n", sizeof(TableHeader));
  printf("Size of bool %zu\n", sizeof(bool));
  printf("Size of uint32 %zu\n", sizeof(uint32_t));
  printf("Size of Pageheader %zu\n", sizeof(PageHeader));
  printf("Size of Tuple %zu\n", sizeof(Tuple));
}

void test(Table* table) {
  PageHeader* page_header = new_page_header(table->header.page_qty);

  Page* page = new_raw_page();

  for(int i = 0; i < 30; i++) {
    Tuple tuple;
    tuple.id = i;
    tuple.data[0] = 'B';
    tuple.data[1] = 'l';
    tuple.data[2] = 'a';
    tuple.data[3] = 'h';
    tuple.data[4] = '\0';
    insert_tuple(page, page_header, tuple);
  }

  flush_page(page, page_header, table->pager);

  free(page);

  PageHeader* read_header = calloc(1, sizeof(PageHeader));
  LinePointer* read_lp = calloc(line_pointers_qty(read_header), sizeof(LinePointer));

  Page* read_page = fetch_page(table->pager, read_header, read_lp, 0);

  ResultSet* result = read_all_tuples(read_page, read_lp, read_header);

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

  free_result_set(result);
  free(read_header);
  free(read_lp);
  free(read_page);
  fclose(table->pager->fp);
  free(table->pager);
  free(table);
}

int main(int argc, char** argv) {
  /* size_of_stuff(); */

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

  InputBuffer* input_buffer = new_input_buffer();

  while(true) {
    print_prompt();
    read_input(input_buffer);

    if (input_buffer->buffer[0] == '.') {
      switch (do_meta_command(input_buffer, table)) {
      case (META_COMMAND_SUCCESS):
        continue;
      case (META_COMMAND_UNRECOGNIZED_COMMAND):
        printf("Unrecognized Command '%s' .\n", input_buffer->buffer);
        continue;

      }
    }

    Statement statement;

    switch (prepare_statement(input_buffer, &statement)) {
      case PREPARE_SUCCESS:
        break;
      case PREPARE_UNRECOGNIZED_STATEMENT:
        printf("Unrecognized keyword at start of '%s'. \n", input_buffer->buffer);
        continue;
      case PREPARE_STM_SYNTAX_ERROR:
        printf("Syntax Error on '%s'. \n", input_buffer->buffer);
        continue;
    }

    switch (execute_statement(&statement, table)) {
        case EXECUTE_SUCCESS:
          break;
        case EXECUTE_TABLE_FULL:
          printf("Error: Table Full.\n");
          break;
        case EXECUTE_VALIDATION_FAILURE:
          printf("Error: Validation Failure.\n");
          break;
        case EXECUTE_FAILURE:
          printf("Error: Failure.\n");
          break;
    }
  }
}

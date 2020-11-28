#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "includes/table.h"
#include "includes/row.h"
#include "includes/meta_commands.h"
#include "includes/statements.h"
#include "includes/cquel.h"


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


int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Needs database file");
    exit(EXIT_FAILURE);
  }

  char* filename = argv[1];
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
          printf("Executed.\n");
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

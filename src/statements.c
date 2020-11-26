#include "includes/statements.h"

ExecuteResult execute_insert(Statement* statement, Table* table) {
  if (table->num_rows >= TABLE_MAX_ROWS) {
    return EXECUTE_TABLE_FULL;
  }

  if (statement->row_to_insert.id < 0) {
    return EXECUTE_VALIDATION_FAILURE;
  }

  if (strlen(statement->row_to_insert.username) >  COLUMN_USERNAME_SIZE
      || strlen(statement->row_to_insert.email) >  COLUMN_EMAIL_SIZE) {
    return EXECUTE_VALIDATION_FAILURE;
  }

  Row* row_to_insert = &(statement->row_to_insert);

  serialize_row(row_to_insert, row_slot(table, table->num_rows));
  table->num_rows += 1;

  return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(__attribute__((unused)) Statement* statement, Table* table) {
  Row row;
  for (uint32_t i = 0; i < table->num_rows; i++) {
    deserialize_row(row_slot(table, i), &row);
    print_row(&row);
  }


  return EXECUTE_SUCCESS;
}
ExecuteResult execute_statement(Statement* statement, Table* table) {
  switch(statement->type) {
  case STATEMENT_INSERT:
    return execute_insert(statement, table);
  case STATEMENT_SELECT:
    return execute_select(statement, table);
  default:
    return EXECUTE_FAILURE;
  }
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
  if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
    statement->type = STATEMENT_INSERT;
    // %s avoids buffer overflow
    int args_count = sscanf(input_buffer->buffer,
                            "insert %d %255s %255s",
                            &(statement->row_to_insert.id),
                            statement->row_to_insert.username,
                            statement->row_to_insert.email);
    if (args_count < 3) {
      return PREPARE_STM_SYNTAX_ERROR;
    }

    return PREPARE_SUCCESS;
  } else if (strcmp(input_buffer->buffer, "select") == 0) {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  } else {
    return PREPARE_UNRECOGNIZED_STATEMENT;
  }
}

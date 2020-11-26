#ifndef __statements_header__
#define __statements_header__

#include "row.h"

typedef enum {
  PREPARE_SUCCESS,
  PREPARE_STM_SYNTAX_ERROR,
  PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum {
  STATEMENT_INSERT,
  STATEMENT_SELECT
} StatementType;

typedef enum {
  EXECUTE_TABLE_FULL,
  EXECUTE_VALIDATION_FAILURE,
  EXECUTE_SUCCESS,
  EXECUTE_FAILURE
} ExecuteResult;

typedef struct {
  StatementType type;
  Row row_to_insert;
} Statement;

ExecuteResult execute_statement(Statement* statement, Table* table);
PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement);

#endif

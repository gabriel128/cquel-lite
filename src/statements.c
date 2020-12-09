#include "includes/statements.h"

ExecuteResult execute_insert(Statement* statement, Table* table) {
  if (table->header.page_qty >= TABLE_MAX_PAGES) {
    return EXECUTE_TABLE_FULL;
  }

  if (strlen(statement->row_to_insert.data) >  TUPLE_SIZE) {
    return EXECUTE_VALIDATION_FAILURE;
  }

  // TODO
  // - Create the concept of BufferPage that contains a Page, PageHeader
  //   and Line Pointers
  Page* page;
  PageHeader* page_header = calloc(1, sizeof(PageHeader));
  LinePointer* page_lps = calloc(line_pointers_qty(page_header), sizeof(LinePointer));

  if (table->header.page_qty == 0) {
    printf("[Log Insert] Inserting new new new Page \n");

    page = new_raw_page();
    page_header = new_page_header(table->header.page_qty);
    insert_tuple(page, page_header, statement->row_to_insert);
    table->header.page_qty += 1;

  } else {
    page = fetch_page(table->pager, page_header, page_lps, table->header.page_qty-1);


    if (page_has_space_for(page_header, sizeof(Tuple))) {
      insert_tuple(page, page_header, statement->row_to_insert);
    } else {
      printf("[Log Insert] Inserting new Page \n");

      page = new_raw_page();
      page_header = new_page_header(table->header.page_qty);
      insert_tuple(page, page_header, statement->row_to_insert);
      table->header.page_qty += 1;
    }
  }

  printf("[Log] Free space in current page is: %d \n", get_free_page_space(page_header));
  printf("[Log] Size of a Tuple is: %ld \n", sizeof(Tuple));
  // Temporarily flushing pages

  flush_page(page, page_header, table->pager);

  printf("[Log] Flushing page with id: %d \n", page_header->page_id);

  if (page != NULL)
    free(page);

  free(page_header);
  free(page_lps);

  return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(__attribute__((unused)) Statement* statement, Table* table) {
  // Temporal until we get a buffer pool
  PageHeader* header = calloc(1, sizeof(PageHeader));
  LinePointer* lps = calloc(line_pointers_qty(header), sizeof(LinePointer));
  Page* page = NULL;
  ResultSet* result = NULL;
  printf("[Log] Page qty is %d \n", table->header.page_qty);

  for (uint32_t i = 0; i < table->header.page_qty; i++) {
    page = fetch_page(table->pager, header, lps, i);
    result = read_all_tuples(page, lps, header);

    for (size_t i = 0; i < result->num_tuples; i++) {
      Tuple read_row = result->tuples[i];
      print_tuple(&read_row);
    }
  }

  if (result == NULL) {
    printf("No results \n");
  } else {
    free_result_set(result);
  }

  if (page != NULL)
    free(page);

  free(header);
  free(lps);


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
    /* int args_count = sscanf(input_buffer->buffer, */
    /*                         "insert %d %255s %255s", */
    /*                         &(statement->row_to_insert.id), */
    /*                         statement->row_to_insert.username, */
    /*                         statement->row_to_insert.email); */
    int args_count = sscanf(input_buffer->buffer,
                            "insert %u %255s",
                            &(statement->row_to_insert.id),
                            statement->row_to_insert.data);

    if (args_count < 1) {
      return PREPARE_STM_SYNTAX_ERROR;
    }

    return PREPARE_SUCCESS;
  } else if (strcmp(input_buffer->buffer, "select_all") == 0) {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  } else {
    return PREPARE_UNRECOGNIZED_STATEMENT;
  }
}

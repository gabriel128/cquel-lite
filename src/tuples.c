#include "includes/tuples.h"

Tuple* read_tuple_from_lp(Page* page, LinePointer* lp) {
  if(page == NULL) {
    printf("Tried to read tuple from null page \n");
    exit(EXIT_FAILURE);
  }

  Tuple* tuple = calloc(1, sizeof(Tuple));

  unsigned int tuple_offset = lp->tuple_offset;
  tuple = ((Tuple*)(page + tuple_offset));

  return tuple;
}

ResultSet* read_all_tuples(Page* page, LinePointer* lp, PageHeader* header) {
  if(page == NULL) {
    printf("Tried to read all tuples from null page \n");
    exit(EXIT_FAILURE);
  }

  ResultSet* result = calloc(1, sizeof(ResultSet));
  result->num_tuples = line_pointers_qty(header);
  result->tuples = calloc(result->num_tuples, sizeof(Tuple));

  for(size_t i = 0; i < result->num_tuples; i++) {
    unsigned int tuple_offset = lp[i].tuple_offset;
    result->tuples[i] = *((Tuple*)(page + tuple_offset));
  }

  return result;
}

bool insert_tuple(Page* page, PageHeader* page_header, char* data) {
  if(page == NULL) {
    printf("Tried to insert tuple to null page \n");
    exit(EXIT_FAILURE);
  }

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
  row.data[strnlen(data, TUPLE_SIZE) - 1] = '\0';

  // cpy rows
  memcpy(page + lp.tuple_offset, &row, lp.tuple_length);

  page_header->upper_limit -= sizeof(Tuple);

  // cpy lps
  memcpy(page + page_header->lower_limit, &lp, sizeof(LinePointer));

  page_header->lower_limit += sizeof(LinePointer);

  page_header->dirty = true;

  return true;
}

void free_result_set(ResultSet* result) {
  free(result->tuples);
  free(result);
}

void print_tuple(Tuple *tuple) {
  printf("Tuple data: %s, id: %d, tuple header offset: %d\n", tuple->data, tuple->id, tuple->header.lp_offset);
}

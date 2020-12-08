#ifndef __tuple_header__
#define __tuple_header__

#include "cquel.h"
#include "pager.h"

typedef struct {
  uint32_t page_id;
  uint32_t lp_offset;
} TupleHeader;

typedef struct {
  TupleHeader header;
  uint32_t id;
  char data[TUPLE_SIZE];
} Tuple;

typedef struct {
  Tuple* tuples;
  uint32_t num_tuples;
} ResultSet;

/*
 * Read the tuple from a page using a line pointer
 */
Tuple* read_tuple_from_lp(Page* page, LinePointer* lp);

/*
 * Read all tuples from a page
 */
ResultSet* read_all_tuples(Page* page, LinePointer* lp, PageHeader* header);

/*
 * Inserts as tuple in certain page and returns true
 * if it was inserted
 */
bool insert_tuple(Page* page, PageHeader* page_header, char* data);

/*
 * Frees allocation from a result set
 */
void free_result_set(ResultSet* result);

/*
 * Prints a tuple
 */
void print_tuple(Tuple *tuple);

#endif

#ifndef __pager_header__
#define __pager_header__

#include "cquel.h"

/*
 * Inspired by Postgres
 * Page structure
 * +----------------+---------------------------------+
 * | PageHeader | linp1 linp2 linp3 ...               |
 * +-----------+----+---------------------------------+
 * | ... linpN |                                      |
 * +-----------+--------------------------------------+
 * |           ^ lower_limit                          |
 * |                                                  |
 * |             v upper_limit                        |
 * +-------------+------------------------------------+
 * |             | tupleN ...                         |
 * +-------------+------------------+-----------------+
 * |     ... tuple3 tuple2 tuple1                     |
 * +--------------------------------+-----------------+
 *
 *
 */

#define TUPLE_SIZE 256 // 255 + \0
#define PAGE_SIZE 4096

#define FIRST_PAGE_OFFSET sizeof(TableHeader)

#define FLEXIBLE_ARRAY_MEMBER // Empty

typedef unsigned char byte;

typedef byte Page;

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
  unsigned int tuple_offset:12; // offset of the tuple from beggining of the page
  unsigned int flags:2;
  unsigned int tuple_length:12;
} LinePointer;

typedef struct {
  bool dirty;
  uint32_t page_id;
  bool page_full;
  uint16_t lower_limit;
  uint16_t upper_limit;
} PageHeader;

typedef struct {
  FILE* fp;
  uint32_t file_length;
  Page* pages[TABLE_MAX_PAGES];
} Pager;

Pager* pager_open(const char* filename);
Page* get_page(Pager* pager, uint32_t page_num);
void flush_page_old(Pager* pager, uint32_t page_num, uint32_t size);
uint32_t page_position(PageHeader* header);
uint32_t page_position_by_id(uint32_t page_id);

#endif

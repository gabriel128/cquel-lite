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

#define FIRST_PAGE_OFFSET sizeof(TableHeader)

#define FLEXIBLE_ARRAY_MEMBER // Empty

typedef byte Page;

typedef struct {
  // offset of the tuple from beggining of the page
  unsigned int tuple_offset:12;
  // Unused atm
  unsigned int flags:2;
  // length of a given tuple in disk
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

/*
  Creates a new page raw_page in the heap
 */
Page* new_raw_page();

/*
  Returns the offset of certain Page
  in the Table File
 */
uint32_t page_offset(PageHeader* header);

/*
  Returns the offset of certain Page
  in the Table File by page_id
*/
uint32_t page_offset_by_id(uint32_t page_id);

/*
  Creates a new page header in the heap
 */
PageHeader* new_page_header(uint32_t ncurrent_pages);

/*
  Returns the qty of line pointers in a page
 */
uint32_t line_pointers_qty(PageHeader* header);

/*
  Reads a page from disk
 */
Page* fetch_page(Pager* pager, PageHeader* page_header, LinePointer* line_pointer, uint32_t page_id);

/*
  Returns how many free bytes a page has
 */

uint32_t get_free_page_space(PageHeader* header);

/*
  Checks if a page has space for N bytes
 */
bool page_has_space_for(PageHeader* header, size_t nbytes);


/*
 *  Writes a page back to disk
 */
void flush_page(Page* page, PageHeader* header, Pager* pager);

#endif

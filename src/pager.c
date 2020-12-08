#include "includes/pager.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

Pager* pager_open(const char* filename) {
  FILE* fp = fopen(filename, "rb+");

  if (fp == NULL) {
    printf("Unable to open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fseek(fp, 0, SEEK_END);
  off_t file_length = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  printf("File size %ld\n", file_length);

  Pager* pager = malloc(sizeof(Pager));
  pager->fp = fp;
  pager->file_length = file_length;
  for(uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    pager->pages[i] = NULL;
  }

  return pager;
}

Page* new_raw_page() {
  Page* raw_page = calloc(1, PAGE_SIZE);
  return raw_page;
}


uint32_t page_offset(PageHeader* header) {
  return FIRST_PAGE_OFFSET + (header->page_id * PAGE_SIZE);
}

uint32_t page_offset_by_id(uint32_t page_id) {
  return FIRST_PAGE_OFFSET + (page_id * PAGE_SIZE);
}

PageHeader* new_page_header(uint32_t ncurrent_pages) {
  printf("Page qty was %d\n", ncurrent_pages);

  PageHeader* page_header = calloc(1, sizeof(PageHeader));

  page_header->page_id = ncurrent_pages;
  page_header->dirty = false;
  page_header->lower_limit = sizeof(PageHeader);
  page_header->upper_limit = PAGE_SIZE;

  return page_header;
}

void flush_page(Page* page, PageHeader* header, Pager* pager) {
  if(!(header->dirty)) {
    printf("Tried to Flush a non dirty page %ld\n", FIRST_PAGE_OFFSET);
    return;
  }

  if(page == NULL) {
    printf("Tried to Flush a null page \n");
    exit(EXIT_FAILURE);
  }

  memcpy(page, header, sizeof(PageHeader)); // ensure page has the  latest header

  FILE* fp = pager->fp;

  fseek(fp, page_offset(header), SEEK_SET);
  size_t bytes_written = fwrite(page, PAGE_SIZE, 1, fp);

  if (bytes_written <= 0) {
    printf("Error on flushing page to disk\n");
    exit(EXIT_FAILURE);
  }
}

uint32_t line_pointers_qty(PageHeader* header) {
  return (header->lower_limit - sizeof(PageHeader)) / sizeof(LinePointer);
}

Page* fetch_page(Pager* pager, PageHeader* page_header, LinePointer* line_pointer, uint32_t page_id) {
  Page* page = new_raw_page();

  if(page == NULL) {
    printf("Tried to fetch a null page \n");
    exit(EXIT_FAILURE);
  }

  uint32_t page_offset = page_offset_by_id(page_id);
  FILE* fp = pager->fp;

  fseek(fp, 0, SEEK_SET);
  fseek(fp, page_offset, SEEK_SET);
  fread(page, PAGE_SIZE, 1, fp);

  memcpy(page_header, page, sizeof(PageHeader));
  memcpy(line_pointer, page + sizeof(PageHeader), sizeof(LinePointer)*line_pointers_qty(page_header));

  return page;
}

uint32_t get_free_page_space(PageHeader* header) {
  if (header->lower_limit > header->upper_limit) {
    return 0;
  } else {
    return header->upper_limit - header->lower_limit;
  }
}

bool page_has_space_for(PageHeader* header, size_t nbytes) {
  return get_free_page_space(header) >= nbytes;
}

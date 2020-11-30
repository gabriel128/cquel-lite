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

void* get_page(Pager* pager, uint32_t page_num) {
  if (page_num > TABLE_MAX_PAGES || ((page_num * PAGE_SIZE) > pager->file_length)) {
    printf("Out of bonds page\n");
    exit(EXIT_FAILURE);
  }

  if (pager->pages[page_num] == NULL) {
    // Cache miss
    void* page = malloc(PAGE_SIZE);
    uint32_t num_pages = pager->file_length / PAGE_SIZE;

    if (page_num <= num_pages) {
      fseek(pager->fp, page_num * PAGE_SIZE, SEEK_SET);
      fread(page, PAGE_SIZE, 1, pager->fp);

      if (ferror(pager->fp)) {
        printf("Error reading page\n");
        exit(EXIT_FAILURE);
      }
    } else {
      printf("Trying to get non existent page\n");
      exit(EXIT_FAILURE);
    }
    pager->pages[page_num] = page;
  }

  return pager->pages[page_num];
}

void flush_page(Pager* pager, uint32_t page_num, uint32_t size) {
  if (pager->pages[page_num] == NULL)  {
    printf("Tried to flush a null page\n");
    exit(EXIT_FAILURE);
  }

  printf("Flushing page_num %d\n", page_num);
  off_t offset = fseek(pager->fp, page_num * PAGE_SIZE, SEEK_SET);

  printf("Offset writing %ld\n", offset);

  if (offset == -1) {
    printf("Error on seek\n");
    exit(EXIT_FAILURE);
  }

  size_t bytes_written = fwrite(pager->pages[page_num], size, 1, pager->fp);

  if (bytes_written <= 0) {
    printf("Error on flushing page\n");
    exit(EXIT_FAILURE);
  }
}

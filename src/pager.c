#include "includes/pager.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

Pager* pager_open(const char* filename) {
  int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

  if (fd == -1) {
    printf("Unable to open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  off_t file_length = lseek(fd, 0, SEEK_END);

  Pager* pager = malloc(sizeof(Pager));
  pager->fd = fd;
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
      lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET);
      ssize_t bytes_read = read(pager->fd, page, PAGE_SIZE);

      if (bytes_read == -1) {
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

void flush_page(Pager* pager, uint32_t page_num) {

  printf("page num %u\n", page_num);

  if (pager->pages[page_num] == NULL)  {
    printf("Tried to flush a null page\n");
    exit(EXIT_FAILURE);
  }

  off_t offset = lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET);

  if (offset == -1) {
    printf("Error on seek\n");
    exit(EXIT_FAILURE);
  }

  ssize_t bytes_written = write(pager->fd, pager->pages[page_num], PAGE_SIZE);

  if (bytes_written == -1) {
    printf("Error on flushing page\n");
    exit(EXIT_FAILURE);
  }
}

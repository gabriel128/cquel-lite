#ifndef __cquel__
#define __cquel__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


#define TABLE_NAME_SIZE 30
#define TABLE_MAX_PAGES 100
#define TUPLE_SIZE 256 // 255 + \0
#define PAGE_SIZE 4096


typedef unsigned char byte;

typedef struct {
  uint32_t page_qty;
  char* table_name;
} TableHeader;

/* #define size_of_attr(Struct, Attr) sizeof(((Struct*)0)->Attr) */
/* #define COLUMN_USERNAME_SIZE 32 */
/* #define COLUMN_EMAIL_SIZE 255 */
/* typedef struct { */
/*   int id; */
/*   char username[COLUMN_USERNAME_SIZE + 1]; */
/*   char email[COLUMN_EMAIL_SIZE + 1]; */
/* } Row; */

typedef struct {
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

#endif

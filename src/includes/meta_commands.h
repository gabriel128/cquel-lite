#ifndef __meta_commands_header__
#define __meta_commands_header__

#include "cquel.h"

typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

MetaCommandResult do_meta_command(InputBuffer* input_buffer);

#endif

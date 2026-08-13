#include "store.h"
#include <stddef.h>

typedef enum {
  ARGS_KEY,
  ARGS_FLAGS,
  ARGS_EXPTIME,
  ARGS_LENGTH,
  ARGS_BYTES,
  ARGS_CMD
} ArgumentName;

typedef enum { ARGS_REQUIRED, ARGS_OPTIONAL } ArgumentType;

typedef enum {
  COMMAND_OK,
  COMMAND_ERROR,
  COMMAND_NOMEM,
  COMMAND_INVALID
} CommandResult;

typedef struct {
  const ArgumentName name;
  const ArgumentType type;
  const char* arg;
} Argument;

typedef CommandResult (*command_handle)(Store* store, Argument* args,
                                        size_t arg_count);

typedef struct {
  const char* name;
  const Argument* args;
  size_t arg_count;

  command_handle handle;
  const char* help;
} Command;

const char* command_get_arg(Argument* args, size_t arg_count,
                            ArgumentName name);

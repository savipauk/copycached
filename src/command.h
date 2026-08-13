#ifndef COMMAND_H
#define COMMAND_H

#include "store.h"
#include <stddef.h>

typedef enum {
  ARGS_KEY,
  ARGS_FLAGS,
  ARGS_EXPTIME,
  ARGS_LENGTH,
  ARGS_BYTES,
  ARGS_DATA,
  ARGS_CMD
} ArgumentName;

typedef enum { ARGS_REQUIRED, ARGS_OPTIONAL } ArgumentType;

typedef enum { COMMAND_OK, COMMAND_ERROR, COMMAND_INVALID } Status;

typedef struct {
  Status type;
  const char* err;
} CommandResult;

typedef struct {
  const ArgumentName name;
  const ArgumentType type;
  const char* arg;
} Argument;

typedef CommandResult (*command_handle)(Store* store, Argument* args,
                                        size_t arg_count, VariableEntry* var);

typedef struct {
  const char* name;
  Argument* args;
  size_t arg_count;

  command_handle handle;
  const char* help;
} Command;

const char* command_get_arg(Argument* args, size_t arg_count,
                            ArgumentName name);

Command* command_find(const char* name);

static inline CommandResult command_ok() {
  return (CommandResult){
      .type = COMMAND_OK,
      .err = NULL,
  };
}

static inline CommandResult command_error(const char* err) {
  return (CommandResult){
      .type = COMMAND_ERROR,
      .err = err,
  };
}

static inline CommandResult command_invalid(const char* err) {
  return (CommandResult){
      .type = COMMAND_INVALID,
      .err = err,
  };
}

#endif

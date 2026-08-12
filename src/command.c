#include "command.h"
#include "store.h"

/*
typedef enum { KEY, FLAGS, EXPTIME, LENGTH, BYTES, CMD } ArgumentName;
typedef enum { REQUIRED, OPTIONAL } ArgumentType;

typedef struct {
  const ArgumentName name;
  const ArgumentType type;
  const char* arg;
} Argument;

typedef void* (*command_handle)(Argument* args, size_t arg_count);

typedef struct {
  const char* name;
  const Argument* args;
  size_t arg_count;

  command_handle handle;
  const char* help;
} Command;


set <key> <flags> <exptime> <bytes>\r\n
<data block>\r\n
→ STORED\r\n
*/

Argument set_args[] = {
    {.name = ARGS_KEY, .type = ARGS_REQUIRED, .arg = "key"},
    {.name = ARGS_FLAGS, .type = ARGS_OPTIONAL, .arg = "flags"},
    {.name = ARGS_EXPTIME, .type = ARGS_OPTIONAL, .arg = "exptime"},
    {.name = ARGS_BYTES, .type = ARGS_REQUIRED, .arg = "bytes"},
};

void* set_handle(S);

static const Command commands[] = {
    {
        .name = "set",
        .args = set_args,
        .arg_count = sizeof(set_args) / sizeof(set_args[0]),
        .handle = 
    },
};




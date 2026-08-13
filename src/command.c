#include "command.h"

/*
set <key> <flags> <exptime> <bytes>\r\n
<data block>\r\n
→ STORED\r\n
*/

const char* command_get_arg(Argument* args, size_t arg_count,
                            ArgumentName name) {
  for (size_t i = 0; i < arg_count; ++i) {
    if (args[i].name == name) {
      return args[i].arg;
    }
  }

  return NULL;
}

Argument set_args[] = {
    {.name = ARGS_KEY, .type = ARGS_REQUIRED, .arg = "key"},
    {.name = ARGS_FLAGS, .type = ARGS_OPTIONAL, .arg = "flags"},
    {.name = ARGS_EXPTIME, .type = ARGS_OPTIONAL, .arg = "exptime"},
    {.name = ARGS_BYTES, .type = ARGS_REQUIRED, .arg = "bytes"},
};

Argument get_args[] = {
    {.name = ARGS_KEY, .type = ARGS_REQUIRED, .arg = "key"},
};

CommandResult set_handle(Store* store, Argument* args, size_t arg_count) {
  // args must have set_args
  const char* key = command_get_arg(args, arg_count, ARGS_KEY);
  const char* flags = command_get_arg(args, arg_count, ARGS_FLAGS);
  const char* exptime = command_get_arg(args, arg_count, ARGS_EXPTIME);
  const char* bytes = command_get_arg(args, arg_count, ARGS_BYTES);

  if (!key) {
    return COMMAND_ERROR;
  }

  return COMMAND_OK;
}

static const Command commands[] = {
    {.name = "set",
     .args = set_args,
     .arg_count = sizeof(set_args) / sizeof(set_args[0]),
     .handle = set_handle,
     .help = "set <key> <value> <optional: type>"},
};

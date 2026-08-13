#include "command.h"
#include <string.h>

/*
set <key> <flags> <exptime> <bytes size>\r\n<data block>\r\n
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
    {.name = ARGS_DATA, .type = ARGS_REQUIRED, .arg = "data"},
};

Argument get_args[] = {
    {.name = ARGS_KEY, .type = ARGS_REQUIRED, .arg = "key"},
};

CommandResult set_handle(Store* store, Argument* args, size_t arg_count,
                         VariableEntry* var) {
  (void)var;
  const char* key = command_get_arg(args, arg_count, ARGS_KEY);
  const char* flags = command_get_arg(args, arg_count, ARGS_FLAGS);
  const char* exptime = command_get_arg(args, arg_count, ARGS_EXPTIME);
  const char* bytes = command_get_arg(args, arg_count, ARGS_BYTES);
  const uint8_t* string_data =
      (const uint8_t*)command_get_arg(args, arg_count, ARGS_DATA);

  if (!key) {
    return command_invalid("missing key");
  }

  if (!bytes) {
    return command_invalid("missing bytes");
  }

  // StoreResult result = store_set(store, var);
  // switch (result) {
  // }

  return command_ok();
}

static const Command commands[] = {
    {.name = "set",
     .args = set_args,
     .arg_count = sizeof(set_args) / sizeof(set_args[0]),
     .handle = set_handle,
     .help = "set <key> <flags> <exptime> <bytes size>\r\n<data block>\r\n"},
};

const Command* command_find(const char* name) {
  for (size_t i = 0; i < sizeof(commands) / sizeof(Command); ++i) {
    if (strcmp(name, commands[i].name) == 0) {
      return &commands[i];
    }
  }

  return NULL;
}


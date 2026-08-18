#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* command_get_arg(Argument* args, size_t arg_count,
                            ArgumentName name) {
  for (size_t i = 0; i < arg_count; ++i) {
    if (args[i].name == name) {
      return args[i].arg;
    }
  }

  return NULL;
}

Argument help_args[] = {
    {.name = ARGS_CMD, .type = ARGS_OPTIONAL, .arg = NULL},
};

Argument set_args[] = {
    {.name = ARGS_KEY, .type = ARGS_REQUIRED, .arg = NULL},
    {.name = ARGS_FLAGS, .type = ARGS_REQUIRED, .arg = NULL},
    {.name = ARGS_BYTES, .type = ARGS_REQUIRED, .arg = NULL},
    {.name = ARGS_DATA, .type = ARGS_REQUIRED, .arg = NULL},
};

Argument get_args[] = {
    {.name = ARGS_KEY, .type = ARGS_REQUIRED, .arg = NULL},
};

// Forward declare because we need to register help as a command, but the
// help_handle needs to print .help of other commands, so commands needs to be
// defined.
CommandResult help_handle(Store* store, Argument* args, size_t arg_count,
                          Variable* var);

CommandResult set_handle(Store* store, Argument* args, size_t arg_count,
                         Variable* var) {
  const char* key = command_get_arg(args, arg_count, ARGS_KEY);
  const char* flags = command_get_arg(args, arg_count, ARGS_FLAGS);
  const char* bytes = command_get_arg(args, arg_count, ARGS_BYTES);
  const uint8_t* string_data =
      (const uint8_t*)command_get_arg(args, arg_count, ARGS_DATA);

  if (!key) {
    return command_invalid("missing key");
  }

  if (!flags) {
    return command_invalid("missing flags");
  }

  if (!bytes) {
    return command_invalid("missing bytes");
  }

  if (!string_data) {
    return command_invalid("missing data");
  }

  var->key = strdup(key);
  if (!var->key) {
    return command_error("out of memory");
  }
  var->flags = strtoul(flags, NULL, 0);
  var->size = strtoul(bytes, NULL, 0);
  var->data = malloc(var->size);
  if (!var->data) {
    variable_free(var);
    return command_error("out of memory");
  }
  memcpy(var->data, string_data, var->size);

  StoreResult result = store_set(store, *var);
  switch (result) {
  case STORE_OK:
    break;
  case STORE_FULL:
  case STORE_UNDEFINED:
  case STORE_NOT_FOUND:
  case STORE_NOMEM:
    variable_free(var);
    return command_error("something broke");
  }

  printf("SET: %s -> %.*s\n", var->key, (int)var->size, (char*)var->data);

  return command_ok();
}

CommandResult get_handle(Store* store, Argument* args, size_t arg_count,
                         Variable* var) {
  const char* key = command_get_arg(args, arg_count, ARGS_KEY);

  if (!key) {
    return command_invalid("missing key");
  }

  StoreResult result = store_get(store, key, var);
  switch (result) {
  case STORE_OK:
    break;
  case STORE_NOT_FOUND:
    return command_error("key not found");
  case STORE_NOMEM:
    return command_error("out of memory");
  case STORE_FULL:
    return command_error("store full during get, big problem; undefined error");
  case STORE_UNDEFINED:
    return command_error("undefined error");
  }

  printf("GET: %s -> %.*s\n", key, (int)var->size, (char*)var->data);
  variable_free(var);

  return command_ok();
}

static Command commands[] = {
    {.name = "help",
     .args = help_args,
     .arg_count = sizeof(help_args) / sizeof(help_args[0]),
     .handle = help_handle,
     .help = "help <optional: command name>"},
    {.name = "set",
     .args = set_args,
     .arg_count = sizeof(set_args) / sizeof(set_args[0]),
     .handle = set_handle,
     .help = "set <key> <flags> <bytes size>\\r\\n<data block>\\r\\n"},
    {.name = "get",
     .args = get_args,
     .arg_count = sizeof(get_args) / sizeof(get_args[0]),
     .handle = get_handle,
     .help = "get <key>"}};

Command* command_find(const char* name) {
  for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
    if (strcmp(name, commands[i].name) == 0) {
      return &commands[i];
    }
  }

  return NULL;
}

CommandResult help_handle(Store* store, Argument* args, size_t arg_count,
                          Variable* var) {
  (void)var;
  (void)store;

  const char* cmd = command_get_arg(args, arg_count, ARGS_CMD);

  if (!cmd) {
    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
      printf("%s\n", commands[i].help);
    }
  } else {
    Command* c = command_find(cmd);
    printf("%s\n", c->help);
  }

  return command_ok();
}

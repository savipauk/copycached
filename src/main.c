#include "parser.h"
#include "store.h"
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int switch_result(ParserResult result) {
  switch (result) {
  case PARSER_OK:
    return 0;
  case PARSER_INCOMPLETE:
    printf("parser incomplete\n");
    break;
  case PARSER_INVALID:
    printf("parser invalid\n");
    break;
  case PARSER_NOMEM:
    printf("parser nomem\n");
    break;
  }

  return -1;
}

int main() {
  char* line = NULL;
  size_t cap = 0;
  ssize_t nread;

  Store store = store_init();
  Parser parser = {0};
  parser.mode = PARSER_REPL;

  while ((nread = getline(&line, &cap, stdin)) != -1) {
    ParserResult result;
    result = parser_feed(&parser, line, (size_t)nread);
    if (switch_result(result) == -1) {
      break;
    }

    Command* cmd;
    result = parser_parse(&parser, &cmd);
    if (switch_result(result) == -1) {
      printf("parse error\n");
    }

    VariableEntry var = {0};
    CommandResult cmd_result =
        cmd->handle(&store, cmd->args, cmd->arg_count, NULL);
    if (cmd_result.type != COMMAND_OK) {
      printf("%s\n", cmd_result.err);
    }
  }

  store_cleanup(&store);
  free(parser.data);
  free(line);

  return 0;
}

int old_main() {
  char* line = NULL;
  size_t cap = 0;

  Store store = store_init();

  while (getline(&line, &cap, stdin) != -1) {
    char* cmd = strtok(line, " \t\r\n");

    if (!cmd) {
      cmd = "help";
    }

    if (strcmp(cmd, "set") == 0) {
      char* key = strtok(NULL, " \t\r\n");

      if (!key) {
        printf("ERROR: set missing key.\n");
        continue;
      }

      char* value = strtok(NULL, " \t\r\n");

      if (!value) {
        printf("ERROR: set missing value.\n");
        continue;
      }

      char* type = strtok(NULL, " \t\r\n");

      Variable v;
      v.key = strdup(key);
      if (type) {
        if (strcmp(type, "int") == 0) {
          // try to cast to int, print error if fail
          v.type = TYPE_INT;

          char* end;
          errno = 0;

          long n = strtol(value, &end, 10);

          if (errno == ERANGE || *end != '\0' || end == value || n < INT_MIN ||
              n > INT_MAX) {
            printf("Invalid integer: %s. Saved as string instead.\n", value);
            v.type = TYPE_STRING;
            v.value.s = strdup(value);
          } else {
            v.value.i = (int)n;
            printf("Parsed int: %d.\n", (int)n);
          }
        } else if (strcmp(type, "float") == 0) {
          // try to cast to float, print error if fail
          v.type = TYPE_FLOAT;

          char* end;
          errno = 0;

          float f = strtof(value, &end);

          if (errno == ERANGE || *end != '\0' || end == value) {
            printf("Invalid float: %s. Saved as string instead.\n", value);
            v.type = TYPE_STRING;
            v.value.s = strdup(value);
          } else {
            v.value.f = f;
            printf("Parsed float: %f.\n", f);
          }
        } else /* if (strcmp(type, "string") == 0) */ {
          v.type = TYPE_STRING;
          v.value.s = strdup(value);
        }
      } else {
        char* end;
        errno = 0;
        long n = strtol(value, &end, 10);

        if (errno != ERANGE && *end == '\0' && end != value && n >= INT_MIN &&
            n <= INT_MAX) {
          v.type = TYPE_INT;
          v.value.i = (int)n;
        } else {
          char* end;
          errno = 0;
          float f = strtof(value, &end);

          if (errno != ERANGE && *end == '\0' && end != value) {
            v.type = TYPE_FLOAT;
            v.value.f = f;
          } else {
            v.type = TYPE_STRING;
            v.value.s = strdup(value);
          }
        }
      }

      StoreResult result = store_set(&store, v);
      if (result != STORE_OK) {
        variable_free(&v);
      }
      switch (result) {
      case STORE_OK:
        break;
      case STORE_FULL:
        printf("ERROR: store full.\n");
        continue;
      case STORE_UNDEFINED:
        printf("ERROR: store not defined.\n");
        continue;
      default:
        printf("ERROR: store_set error.\n");
        continue;
      }
    } else if (strcmp(cmd, "get") == 0) {
      char* key = strtok(NULL, " \t\r\n");

      if (!key) {
        printf("ERROR: get missing key.\n");
        continue;
      }

      Variable v;
      StoreResult result = store_get(&store, key, &v);

      switch (result) {
      case STORE_OK:
        break;
      case STORE_UNDEFINED:
        printf("ERROR: store not defined.\n");
        continue;
      case STORE_NOT_FOUND:
        printf("ERROR: no key found in store.\n");
        continue;
      case STORE_NOMEM:
        printf("ERROR: out of memory.\n");
        continue;
      default:
        printf("ERROR: store_get error.\n");
        continue;
      }

      printf("key: %s\n", key);
      printf("type: %s\n", type_to_string(v.type));
      switch (v.type) {
      case TYPE_INT:
        printf("value: %d\n", v.value.i);
        break;
      case TYPE_FLOAT:
        printf("value: %f\n", v.value.f);
        break;
      case TYPE_STRING:
        printf("value: %s\n", v.value.s);
        break;
      }

      variable_free(&v);
    } else if (strcmp(cmd, "del") == 0) {
      char* key = strtok(NULL, " \t\r\n");

      if (!key) {
        printf("ERROR: get missing key.\n");
        continue;
      }

      StoreResult result = store_delete(&store, key);

      switch (result) {
      case STORE_OK:
        break;
      case STORE_UNDEFINED:
        printf("ERROR: store not defined.\n");
        continue;
      case STORE_NOT_FOUND:
        printf("ERROR: no key found in store.\n");
        continue;
      default:
        printf("ERROR: store_get error.\n");
        continue;
      }

      printf("Successfully deleted %s from the store.\n", key);
    } else {
      char* cmd_to_help = strtok(NULL, " \t\r\n");

      if (!cmd_to_help) {
        printf("help <command>\n");
        printf("set <key> <value> <optional: type>\n");
        printf("get <key>\n");
        printf("del <key>\n");
        continue;
      }

      if (strcmp(cmd_to_help, "set") == 0) {
        printf("set <key> <value> <optional: type>\n");
      } else if (strcmp(cmd_to_help, "get") == 0) {
        printf("get <key>\n");
      } else if (strcmp(cmd_to_help, "del") == 0) {
        printf("del <key>\n");
      } else {
        printf("help <command>\n");
      }
    }
  }

  store_cleanup(&store);
  free(line);

  return 0;
}

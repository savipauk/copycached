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
      continue;
    }

    Variable var = {0};
    CommandResult cmd_result =
        cmd->handle(&store, cmd->args, cmd->arg_count, &var);
    if (cmd_result.type != COMMAND_OK) {
      printf("%s\n", cmd_result.err);
    }
  }

  store_cleanup(&store);
  free(parser.data);
  free(line);

  return 0;
}

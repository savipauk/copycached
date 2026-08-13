#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ParserResult parser_feed(Parser* parser, const char* data, size_t len) {
  if (len == 0) {
    return PARSER_INVALID;
  }

  size_t required = parser->len + len + 1;

  if (required > parser->capacity) {
    size_t capacity = parser->capacity ? parser->capacity : 64;

    while (capacity < required) {
      capacity *= 2;
    }

    char* new_data = realloc(parser->data, capacity);
    if (!new_data) {
      return PARSER_NOMEM;
    }

    parser->data = new_data;
    parser->capacity = capacity;
  }

  memcpy(parser->data + parser->len, data, len);
  parser->len += len;
  parser->data[parser->len] = '\0';

  return PARSER_OK;
}

ParserResult parser_parse(Parser* parser, Command** cmd) {
  if (!parser->data || parser->len == 0) {
    return PARSER_INCOMPLETE;
  }

  char* cmd_name = strtok(parser->data, " \t\r\n");

  if (!cmd_name) {
    cmd_name = "help";
  }

  *cmd = command_find(cmd_name);

  if (!cmd) {
    return PARSER_INVALID;
  }

  for (size_t i = 0; i < (*cmd)->arg_count; ++i) {
    ParserResult argument_parse =
        parser_parse_argument(parser, &(*cmd)->args[i]);

    if (argument_parse != PARSER_OK) {
      return argument_parse;
    }
  }

  parser->len = 0;

  return PARSER_OK;
}

ParserResult parser_parse_argument(Parser* parser, Argument* arg) {
  (void)parser;
  char* arg_arg = strtok(NULL, " \t\r\n");

  if (!arg_arg) {
    return PARSER_INCOMPLETE;
  }

  arg->arg = arg_arg;

  switch (arg->name) {
  case ARGS_KEY:
    printf("ARGS_KEY: ");
    break;
  case ARGS_FLAGS:
    printf("ARGS_FLAGS: ");
    break;
  case ARGS_EXPTIME:
    printf("ARGS_EXPTIME: ");
    break;
  case ARGS_LENGTH:
    printf("ARGS_LENGTH: ");
    break;
  case ARGS_BYTES:
    printf("ARGS_BYTES: ");
    break;
  case ARGS_DATA:
    printf("ARGS_DATA: ");
    break;
  case ARGS_CMD:
    printf("ARGS_CMD: ");
    break;
  default:
    return PARSER_INVALID;
  }

  printf("%s\n", arg_arg);

  return PARSER_OK;
}

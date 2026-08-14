#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
expect:
set <key> <flags> <bytes size> <data block>\r\n
→ STORED
read bytes-size from data block and expect \r\n
*/

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
    printf("no data or len = 0\n");
    return PARSER_INCOMPLETE;
  }

  char* cmd_name = strtok(parser->data, " \t\r\n");

  if (!cmd_name) {
    cmd_name = "help";
  }

  *cmd = command_find(cmd_name);

  if (!*cmd) {
    parser->len = 0;
    return PARSER_INVALID;
  }

  for (size_t i = 0; i < (*cmd)->arg_count; ++i) {
    (*cmd)->args[i].arg = NULL;
  }

  position = strlen((*cmd)->name) + 1;

  for (size_t i = 0; i < (*cmd)->arg_count; ++i) {
    ParserResult argument_parse =
        parser_parse_argument(parser, &(*cmd)->args[i]);

    switch (argument_parse) {
    case PARSER_INCOMPLETE:
    case PARSER_INVALID:
    case PARSER_NOMEM:
      parser->len = 0;
      return argument_parse;
    case PARSER_WHITESPACE:
      continue;
    case PARSER_OK:
      continue;
    }
  }

  parser->len = 0;

  return PARSER_OK;
}

ParserResult parser_parse_argument(Parser* parser, Argument* arg) {
  (void)parser;

  ParserResult next_res = parser_parse_next(parser, 0);

  switch (next_res) {
  case PARSER_INCOMPLETE:
  case PARSER_INVALID:
  case PARSER_NOMEM:
  case PARSER_WHITESPACE:
    return next_res;
  case PARSER_OK:
    break;
  }

  printf("parser_parse_next -> %s\n", constructed_string);

  char* arg_arg = constructed_string;

  if (!arg_arg) {
    if (arg->type == ARGS_REQUIRED) {
      printf("no arg_arg\n");
      return PARSER_INCOMPLETE;
    } else {
      return PARSER_OK;
    }
  }

  arg->arg = arg_arg;

  switch (arg->name) {
  case ARGS_KEY:
    printf("ARGS_KEY: ");
    break;
  case ARGS_FLAGS:
    printf("ARGS_FLAGS: ");
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

ParserResult parser_parse_next(Parser* parser, size_t count) {
  if (parser->len <= position) {
    return PARSER_INCOMPLETE;
  }

  char read = parser->data[position];
  printf("read char %d at pos %zu\n", read, position);

  switch (read) {
  case ' ':
  case '\t':
  case '\r':
    printf("encountered whitespace\n");
    position++;
    return PARSER_WHITESPACE;
  default:
    break;
  }

  if (!constructed_string) {
    constructed_string = malloc(1);
    if (!constructed_string) {
      return PARSER_NOMEM;
    }
  } else {
    char* tmp = realloc(constructed_string, count + 1);
    if (!tmp) {
      return PARSER_NOMEM;
    }

    constructed_string = tmp;
  }

  constructed_string[count] = read;
  position++;

  return parser_parse_next(parser, count++);
}

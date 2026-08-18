#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
expect:
set <key> <flags> <bytes size> <data block>\n
→ STORED
read bytes-size from data block and expect \n
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
  parser->position = 0;
  parser->count = 0;
  
  if (!parser->data || parser->len == 0) {
    printf("no data or len = 0\n");
    return PARSER_INCOMPLETE;
  }

  ParserResult next_res = parser_parse_next(parser);

  if (!parser->constructed_string) {
    printf("no constructed string\n");
  } else {
    printf("cmd_name parser_parse_next size %d -> %.*s\n", (int)parser->count,
           (int)parser->count, parser->constructed_string);
  }

  switch (next_res) {
  case PARSER_INCOMPLETE:
  case PARSER_INVALID:
  case PARSER_NOMEM:
    return next_res;
  case PARSER_WHITESPACE:
  case PARSER_OK:
    break;
  }

  char* cmd_name = malloc(parser->count + 1);
  memcpy(cmd_name, parser->constructed_string, parser->count);
  cmd_name[parser->count] = '\0';

  *cmd = command_find(cmd_name);
  free(cmd_name);

  if (!*cmd) {
    parser->len = 0;
    printf("found no function\n");
    return PARSER_INVALID;
  }

  for (size_t i = 0; i < (*cmd)->arg_count; ++i) {
    (*cmd)->args[i].arg = NULL;
  }

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
    case PARSER_OK:
      continue;
    }
  }

  parser->len = 0;

  return PARSER_OK;
}

ParserResult parser_parse_argument(Parser* parser, Argument* arg) {
  parser->count = 0;
  ParserResult next_res = parser_parse_next(parser);

  switch (next_res) {
  case PARSER_INCOMPLETE:
  case PARSER_INVALID:
  case PARSER_NOMEM:
  case PARSER_WHITESPACE:
    return next_res;
  case PARSER_OK:
    break;
  }

  printf("parser_parse_next -> %s\n", parser->constructed_string);

  char* arg_arg = strdup(parser->constructed_string);

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

ParserResult parser_parse_next(Parser* parser) {
  if (parser->len <= parser->position) {
    return PARSER_INCOMPLETE;
  }

  char read = parser->data[parser->position];
  printf("read char %d at pos %zu\n", read, parser->position);

  switch (read) {
  case ' ':
  case '\t':
  case '\n':
    printf("encountered whitespace\n");
    parser->position++;
    if (parser->count == 0) {
      return PARSER_WHITESPACE;
    }
    parser->constructed_string[parser->count] = '\0';
    return PARSER_OK;
  default:
    break;
  }

  if (!parser->constructed_string) {
    parser->constructed_string = malloc(2);
    if (!parser->constructed_string) {
      return PARSER_NOMEM;
    }
  } else {
    char* tmp = realloc(parser->constructed_string, parser->count + 2);
    if (!tmp) {
      return PARSER_NOMEM;
    }

    parser->constructed_string = tmp;
  }

  parser->constructed_string[parser->count] = read;
  parser->position++;
  parser->count++;

  return parser_parse_next(parser);
}

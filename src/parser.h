#ifndef PARSER_H
#define PARSER_H

#include "command.h"

typedef enum { PARSER_REPL, PARSER_TCP } ParserMode;

typedef enum {
  PARSER_OK,
  PARSER_INCOMPLETE,
  PARSER_INVALID,
  PARSER_NOMEM,
  PARSER_WHITESPACE
} ParserResult;

typedef struct {
  ParserMode mode;

  char* data;
  size_t len;
  size_t capacity;

  char* constructed_string;
  size_t count;
  size_t position;
} Parser;

ParserResult parser_feed(Parser* parser, const char* data, size_t len);

ParserResult parser_parse(Parser* parser, Command** cmd);

ParserResult parser_parse_argument(Parser* parser, Argument* arg);

ParserResult parser_parse_next(Parser* parser);

#endif

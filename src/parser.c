#include "parser.h"
#include <string.h>

void parse_line(char* entry) {
  char* cmd = strtok(entry, " \t\r\n");

  if (strcmp(cmd, "set") == 0) {

  } else if (strcmp(cmd, "get") == 0) {

  }
}

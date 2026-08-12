#include <stddef.h>
/*

set <key> <flags> <exptime> <bytes>\r\n
<data block>\r\n
→ STORED\r\n

get <key>\r\n
→ VALUE <key> <flags> <bytes>\r\n
  <data block>\r\n
  END\r\n

*/

typedef enum { ARGS_KEY, ARGS_FLAGS, ARGS_EXPTIME, ARGS_LENGTH, ARGS_BYTES, ARGS_CMD } ArgumentName;
typedef enum { ARGS_REQUIRED, ARGS_OPTIONAL } ArgumentType;

typedef struct {
  const ArgumentName name;
  const ArgumentType type;
  const char* arg;
} Argument;

typedef void* (*command_handle)(Argument* args, size_t arg_count);

typedef struct {
  const char* name;
  const Argument* args;
  size_t arg_count;

  command_handle handle;
  const char* help;
} Command;

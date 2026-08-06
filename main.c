#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STORE_SIZE 128

typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_STRING } Type;

typedef enum {
  STORE_OK,
  STORE_FULL,
  STORE_UNDEFINED,
  STORE_NOT_FOUND,
  STORE_NOMEM
} StoreResult;

// TODO
typedef enum { SLOT_EMPTY, SLOT_OCCUPIED, SLOT_DELETED } SlotState;

typedef union {
  int i;
  float f;
  char* s;
} Value;

typedef struct {
  char* key;
  Type type;
  Value value;
} Variable;

typedef struct {
  Variable* arr;
  size_t count;
} Store;

void variable_free(Variable* v) {
  free(v->key);
  if (v->type == TYPE_STRING) {
    free(v->value.s);
  }
}

int variable_copy(const Variable* src, Variable* dst) {
  *dst = *src;

  dst->key = strdup(src->key);
  if (!dst->key) {
    return -1;
  }

  if (src->type == TYPE_STRING) {
    dst->value.s = strdup(src->value.s);
    if (!dst->value.s) {
      free(dst->key);
      return -1;
    }
  }

  return 0;
}

uint64_t hash_string(const char* s) {
  uint64_t hash = 14695981039346656037ULL;

  while (*s) {
    hash ^= (unsigned char)*s++;
    hash *= 1099511628211ULL;
  }

  return hash;
}

StoreResult store_set(Store* store, Variable var) {
  if (!store) {
    return STORE_UNDEFINED;
  }

  size_t index = hash_string(var.key) % STORE_SIZE;

  size_t i = 0;
  while (store->arr[index].key != NULL) {
    if (i == STORE_SIZE) {
      return STORE_FULL;
    }

    // if trying to write to key 'foo' which already exists
    if (strcmp(store->arr[index].key, var.key) == 0) {
      variable_free(&store->arr[index]);
      store->arr[index] = var;
      return STORE_OK;
    }
    index = (index + 1) % STORE_SIZE;
    i++;
  }

  store->arr[index] = var;
  store->count++;

  return STORE_OK;
}

// On STORE_OK, *var is a deep copy owned by the caller. Release it with
// variable_free().
StoreResult store_get(Store* store, const char* key, Variable* var) {
  if (!store) {
    return STORE_UNDEFINED;
  }

  size_t index = hash_string(key) % STORE_SIZE;

  while (store->arr[index].key != NULL) {
    if (strcmp(store->arr[index].key, key) == 0) {
      if (variable_copy(&store->arr[index], var) != 0) {
        return STORE_NOMEM;
      }
      return STORE_OK;
    }

    index = (index + 1) % STORE_SIZE;
  }

  return STORE_NOT_FOUND;
}

char* type_to_string(Type t) {
  switch (t) {
  case TYPE_INT:
    return "TYPE_INT";
  case TYPE_FLOAT:
    return "TYPE_FLOAT";
  case TYPE_STRING:
    return "TYPE_STRING";
  }
  return "";
}

int main() {
  char* line = NULL;
  size_t cap = 0;

  Variable arr[STORE_SIZE] = {0};
  Store store = {.arr = arr, .count = 0};

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
    } else {
      char* cmd_to_help = strtok(NULL, " \t\r\n");

      if (!cmd_to_help) {
        printf("help <command>\n");
        printf("set <key> <value> <optional: type>\n");
        printf("get <key>\n");
        continue;
      }

      if (strcmp(cmd_to_help, "set") == 0) {
        printf("set <key> <value> <optional: type>\n");
      } else if (strcmp(cmd_to_help, "get") == 0) {
        printf("get <key>\n");
      } else {
        printf("help <command>\n");
      }
    }
  }

  for (size_t i = 0; i < STORE_SIZE; i++) {
    if (arr[i].key) {
      variable_free(&arr[i]);
    }
  }
  free(line);

  return 0;
}

#include <stddef.h>
#include <stdint.h>
#define STORE_SIZE 128

typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_STRING } Type;

typedef enum {
  STORE_OK,
  STORE_FULL,
  STORE_UNDEFINED,
  STORE_NOT_FOUND,
  STORE_NOMEM
} StoreResult;

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

void variable_free(Variable* v);

int variable_copy(const Variable* src, Variable* dst);

uint64_t hash_string(const char* s);

StoreResult store_set(Store* store, Variable var);

StoreResult store_get(Store* store, const char* key, Variable* var);

char* type_to_string(Type t);

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

typedef enum { FIND_FOUND, FIND_ABSENT, FIND_FULL } FindStatus;

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
  SlotState state;
  Variable var;
} Slot;

typedef struct {
  Slot arr[STORE_SIZE];
  size_t count;
  size_t deleted;
} Store;

typedef struct {
  size_t index;
  FindStatus status;
} StoreFind;

void variable_free(Variable* v);

int variable_copy(const Variable* src, Variable* dst);

uint64_t hash_string(const char* s);

StoreFind store_find(Store* store, const char* key);

StoreResult store_set(Store* store, Variable var);

StoreResult store_get(Store* store, const char* key, Variable* var);

StoreResult store_delete(Store* store, const char* key);

Store store_init();

void store_cleanup(Store* store);

char* type_to_string(Type t);

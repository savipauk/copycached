#ifndef STORE_H
#define STORE_H

#include <stddef.h>
#include <stdint.h>
#define STORE_SIZE 128
#define FLAGS_SIZE 32

typedef enum {
  STORE_OK,
  STORE_FULL,
  STORE_UNDEFINED,
  STORE_NOT_FOUND,
  STORE_NOMEM
} StoreResult;

typedef enum { SLOT_EMPTY, SLOT_OCCUPIED, SLOT_DELETED } SlotState;

typedef enum { FIND_FOUND, FIND_ABSENT, FIND_FULL } FindStatus;

typedef struct {
  char* key;
  uint8_t* data;
  uint32_t flags;
  size_t size;
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

#endif

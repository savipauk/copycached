#include "store.h"
#include <stdlib.h>
#include <string.h>

void variable_free(Variable* v) {
  free(v->key);
  free(v->data);
}

int variable_copy(const Variable* src, Variable* dst) {
  *dst = (Variable){0};

  dst->key = strdup(src->key);
  if (!dst->key) {
    return -1;
  }

  dst->data = malloc(src->size);
  if (!dst->data) {
    free(dst->key);
    dst->key = NULL;
    return -1;
  }

  memcpy(dst->data, src->data, src->size);
  dst->size = src->size;
  dst->flags = src->flags;

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

StoreFind store_find(Store* store, const char* key) {
  size_t index = hash_string(key) % STORE_SIZE;
  size_t tomb = STORE_SIZE;

  for (size_t i = 0; i < STORE_SIZE; ++i) {
    Slot* slot = &store->arr[index];

    if (slot->state == SLOT_EMPTY) {
      return (StoreFind){tomb == STORE_SIZE ? index : tomb, FIND_ABSENT};
    }

    if (slot->state == SLOT_DELETED) {
      if (tomb == STORE_SIZE) {
        tomb = index;
      }
    } else if (strcmp(slot->var.key, key) == 0) {
      return (StoreFind){index, FIND_FOUND};
    }

    index = (index + 1) % STORE_SIZE;
  }

  if (tomb == STORE_SIZE) {
    return (StoreFind){0, FIND_FULL};
  }

  return (StoreFind){tomb, FIND_ABSENT};
}

StoreResult store_set(Store* store, Variable var) {
  if (!store) {
    return STORE_UNDEFINED;
  }

  StoreFind find = store_find(store, var.key);
  Slot* slot = &store->arr[find.index];

  switch (find.status) {
  case FIND_FOUND:
    variable_free(&slot->var);
    slot->var = var;
    return STORE_OK;

  case FIND_ABSENT:
    if (slot->state == SLOT_DELETED) {
      store->deleted--;
    }

    slot->var = var;
    slot->state = SLOT_OCCUPIED;
    store->count++;
    return STORE_OK;

  case FIND_FULL:
    return STORE_FULL;
  }

  return STORE_UNDEFINED;
}

// On STORE_OK, *var is a deep copy owned by the caller. Release it with
// variable_free().
StoreResult store_get(Store* store, const char* key, Variable* var) {
  if (!store) {
    return STORE_UNDEFINED;
  }

  StoreFind find = store_find(store, key);
  Slot* slot = &store->arr[find.index];

  if (find.status == FIND_FOUND) {
    if (variable_copy(&slot->var, var) != 0) {
      return STORE_NOMEM;
    }
    return STORE_OK;
  }

  return STORE_NOT_FOUND;
}

StoreResult store_delete(Store* store, const char* key) {
  if (!store) {
    return STORE_UNDEFINED;
  }

  StoreFind find = store_find(store, key);

  if (find.status != FIND_FOUND) {
    return STORE_NOT_FOUND;
  }

  Slot* slot = &store->arr[find.index];
  variable_free(&slot->var);

  slot->var = (Variable){0};
  slot->state = SLOT_DELETED;
  store->count--;
  store->deleted++;

  return STORE_OK;
}

Store store_init() {
  Store store;

  for (size_t i = 0; i < STORE_SIZE; ++i) {
    store.arr[i].state = SLOT_EMPTY;
    store.arr[i].var.key = NULL;
    store.arr[i].var.data = NULL;
    store.arr[i].var.size = 0;
  }

  store.count = 0;
  store.deleted = 0;

  return store;
}

void store_cleanup(Store* store) {
  for (size_t i = 0; i < STORE_SIZE; i++) {
    if (store->arr[i].state == SLOT_OCCUPIED) {
      variable_free(&store->arr[i].var);
    }
    store->arr[i] = (Slot){.state = SLOT_EMPTY, .var = {0}};
  }
  store->count = 0;
  store->deleted = 0;
}

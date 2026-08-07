#include "store.h"
#include <stdlib.h>
#include <string.h>

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

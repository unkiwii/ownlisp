#include "env.h"
#include "utils.h"
#include <string.h>

lenv* lenv_new(void)
{
  lenv* e = malloc(sizeof(lenv));
  e->debug = 0;
  e->parser = lparser_new();
  e->parent = NULL;
  e->count = 0;
  e->syms = NULL;
  e->vals = NULL;
  return e;
}

lenv* lenv_copy(lenv* e)
{
  lenv* n = malloc(sizeof(lenv));
  n->debug = e->debug;
  n->parser = NULL;
  n->parent = e->parent;
  n->count = e->count;
  n->syms = malloc(sizeof(char*) * n->count);
  n->vals = malloc(sizeof(lval*) * n->count);
  for (int i = 0; i < n->count; i++) {
    n->syms[i] = malloc(strlen(e->syms[i]) + i);
    strcpy(n->syms[i], e->syms[i]);
    n->vals[i] = lval_copy(e->vals[i]);
  }
  return n;
}

void lenv_def(lenv* e, lval* k, lval* v)
{
  while (e->parent) {
    e = e->parent;
  }

  lenv_put(e, k, v);
}

void lenv_del(lenv* e)
{
  if (e->parser) {
    lparser_del(e->parser);
  }
  for (int i = 0; i < e->count; i++) {
    free(e->syms[i]);
    lval_del(e->vals[i]);
  }
  free(e->syms);
  free(e->vals);
  free(e);
}

lval* lenv_get(lenv* e, lval* k)
{
  /* go over all items in environment */
  for (int i = 0; i < e->count; i++) {
    /* if the symbol is found then return a copy of it */
    if (is(e->syms[i], k->sym)) {
      return lval_copy(e->vals[i]);
    }
  }

  if (e->parent) {
    /* if no symbol was found then look for it in the parent environment */
    return lenv_get(e->parent, k);
  } else {
    /* if no symbol was found and there is no parent environment, then the symbols doesn't exist */
    return lval_err("unbound symbol %s", k->sym);
  }
}

void lenv_put(lenv* e, lval* k, lval* v)
{
  /* go over all items in environment */
  for (int i = 0; i < e->count; i++) {
    /* if the symbol is found then delete it and replace it with the new one */
    if (is(e->syms[i], k->sym)) {
      lval_del(e->vals[i]);
      e->vals[i] = lval_copy(v);
      return;
    }
  }

  /* if no existing entry found, then allocate space for new entry */
  e->count++;
  e->vals = realloc(e->vals, sizeof(lval*) * e->count);
  e->syms = realloc(e->syms, sizeof(char*) * e->count);

  /* and save a new entry */
  e->vals[e->count - 1] = lval_copy(v);
  e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
  strcpy(e->syms[e->count - 1], k->sym);
}

lparser* lenv_getparser(lenv* e)
{
  if (e->parser) {
    return e->parser;
  }

  if (e->parent) {
    return lenv_getparser(e->parent);
  }

  return NULL;
}

#ifndef LISPY_ENV_H
#define LISPY_ENV_H

#include "fwd.h"
#include "val.h"
#include "parser.h"

/**
 * The language environment, an "scope" where all symbols are defined
 */
struct lenv
{
  /** to debug or no debug **/
  int debug;

  /** the parser (used by load) **/
  lparser* parser;

  /** parent environment **/
  lenv* parent;

  /** number of items in the environment **/
  int count;

  /** list of symbol names (as char*) **/
  char** syms;

  /** list of symbols (as lval*) **/
  lval** vals;
};

/**
 * Creates a new environment
 *
 * return     a new and empty lenv*
 */
lenv* lenv_new(void);

/**
 * Copy an environment
 *
 * lenv* e    the environment to be copied
 *
 * return     a new copy with the same (copied) symbols of the original one
 */
lenv* lenv_copy(lenv* e);

/**
 * Destroy an anvironment
 *
 * lenv* e    the environment to be destroyed, also destroy every symbol added
 */
void lenv_del(lenv* e);

/**
 * Retrieve a symbol from an environment
 *
 * lenv* e      the environment to search
 * lval* key    an lval* of type LVAL_SYM to search in the environment, if
 *              the key is found, then its corresponding value is returned
 *              if not found, the search is performed in the parent environment
 *              until no parent or key is found, then an "unbound symbol" error
 *              is returned if no symbol can be found
 *
 *  return      a copy of the value of the symbol or "unbound symbol" error
 *              as an lval* of type LVAL_ERR
 */
lval* lenv_get(lenv* e, lval* key);

/**
 * Adds a symbol to the environment
 *
 * lenv* e      the environment to modify
 * lval* key    an lval* of type LVAL_SYM to be used as the name of the defined value
 * lval* value  an lval* of any type to be added
 */
void lenv_put(lenv* e, lval* key, lval* value);

/**
 * Adds a symbol to the global environment. Starting with the passed environment
 * it goes "up" searching the parent of every environment until the global env
 * is found (with no parent) and then calls lenv_put() on that given environment
 *
 * lenv* e      the environment to start to search
 * lval* key    the key to pass to lenv_put()
 * lval* value  the value to pass to lenv_put()
 */
void lenv_def(lenv* e, lval* key, lval* value);

/**
 * Given any environment, it returns the parser that only
 * exists in the global environment
 */
lparser* lenv_getparser(lenv* e);

#endif//LISPY_ENV_H

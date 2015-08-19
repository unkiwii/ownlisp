#ifndef LISPY_EVAL_H
#define LISPY_EVAL_H

#include "env.h"
#include "val.h"

/**
 * Eval an S-Expression
 */
lval* leval_sexpr(lenv* e, lval* v);

/**
 * Eval an lval
 */
lval* leval(lenv* e, lval* v);

/**
 * Calls a function
 */
lval* lcall(lenv* e, lval* f, lval* a);

#endif//LISPY_EVAL_H

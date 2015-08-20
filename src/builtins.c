#include "builtins.h"
#include "eval.h"
#include "utils.h"
#include "parser.h"

#define LASSERT(args, cond, fmt, ...)         \
  if (!(cond)) {                              \
    lval* err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args);                           \
    return err;                               \
  }

#define LASSERT_TYPE(func, args, index, expect)                             \
  LASSERT(args, args->cell[index]->type == expect,                          \
      "function '%s' passed incorrect type for argument %i. "               \
      "got '%s', expected '%s'.",                                           \
      func, index, ltype_name(args->cell[index]->type), ltype_name(expect))

#define LASSERT_NUM(func, args, num)                         \
  LASSERT(args, args->count == num,                          \
      "function '%s' passed incorrect number of arguments. " \
      "got '%i', expected '%i'.",                            \
      func, args->count, num)

#define LASSERT_NUM_OR(func, args, num1, num2)               \
  LASSERT(args, args->count == num1 || args->count == num2,  \
      "function '%s' passed incorrect number of arguments. " \
      "got %i, expected %i or %i.",                          \
      func, args->count, num1, num2)

#define LASSERT_NOT_EMPTY(func, args, index)      \
  LASSERT(args, args->cell[index]->count != 0,    \
      "function '%s' passed {} for argument %i. " \
      func, index)

typedef void(*ldef)(lenv*, lval*, lval*);

lval* _bt_def(lenv* e, lval* a, ldef func, char* fname)
{
  /**
   * def has 2 forms:
   *
   *  def a b                   (1)
   *
   *  def {a b c} {d e f}       (2)
   *
   * in (1) only one symbol is defined
   * in (2) more than one symbol is defined
   */

  /* more than one symbol defined */
  if (a->cell[0]->type == LVAL_QEXPR) {
    lval* syms = a->cell[0];
    for (int i = 0; i < syms->count; i++) {
      /* every element of the first parameter of def must be a symbol to define */
      LASSERT(a, (syms->cell[i]->type == LVAL_SYM),
          "function '%s' cannot define non-symbol. "
          "got '%s', expected '%s'.", fname,
          ltype_name(syms->cell[i]->type),
          ltype_name(LVAL_SYM));
    }

    LASSERT(a, syms->count == a->count - 1,
        "function '%s' cannot define incorrect number of values to symbols. "
        "got %i, expected %i.", fname, syms->count, a->count - 1);

    for (int i = 0; i < syms->count; i++) {
      func(e, syms->cell[i], a->cell[i + 1]);
    }
    lval_del(a);

  /* one symbol defined */
  } else if (a->cell[0]->type == LVAL_SYM) {
    /* if the first parameter is a symbol only 2 parameters are allowed */
    LASSERT_NUM(fname, a, 2);
    func(e, a->cell[0], a->cell[1]);
    lval_del(a);

  /* neither one or more symbols defined is an error */
  } else {
    return lval_err("only symbols can be defined");
  }


  return lval_sexpr();
}

BUILTIN(GDEF) { return _bt_def(e, a, lenv_def, KW_GDEF); }
BUILTIN(LDEF) { return _bt_def(e, a, lenv_put, KW_LDEF); }

BUILTIN(HEAD)
{
  /* must have only one argument */
  LASSERT_NUM(KW_HEAD, a, 1);

  /* and that argument must be a Q-Expression */
  LASSERT_TYPE(KW_HEAD, a, 0, LVAL_QEXPR);

  /* and the argument must have len > 0 */
  LASSERT_NOT_EMPTY(KW_HEAD, a, 0);

  /* take the first argument */
  lval* v = lval_take(a, 0);

  /* delete all elements that are not the head */
  while (v->count > 1) {
    lval_del(lval_pop(v, 1));
  }

  return v;
}

BUILTIN(TAIL)
{
  /* must have only one argument */
  LASSERT_NUM(KW_TAIL, a, 1);

  /* and that argument must be a Q-Expression */
  LASSERT_TYPE(KW_TAIL, a, 0, LVAL_QEXPR);

  /* and the argument must have len > 0 */
  LASSERT_NOT_EMPTY(KW_TAIL, a, 0);

  /* take the first argument */
  lval* v = lval_take(a, 0);

  /* delete the first element */
  lval_del(lval_pop(v, 0));

  return v;
}

BUILTIN(LIST)
{
  /* list just transforms any expression into a qexpr */
  a->type = LVAL_QEXPR;
  return a;
}

BUILTIN(EVAL)
{
  /* must have only one argument */
  LASSERT_NUM(KW_EVAL, a, 1);

  /* and that argument must be a Q-Expression */
  LASSERT_TYPE(KW_EVAL, a, 0, LVAL_QEXPR);

  /* take the first argument */
  lval* v = lval_take(a, 0);
  /* make it an sexpr */
  v->type = LVAL_SEXPR;
  /* and evaluate it */
  return leval(e, v);
}

BUILTIN(JOIN)
{
  /* can receive any number of arguments ... */
  for (int i = 0; i < a->count; i++) {
    /* ...but every argument must be a Q-Expression */
    LASSERT_TYPE(KW_JOIN, a, i, LVAL_QEXPR);
  }

  /* take first element */
  lval* v = lval_pop(a, 0);

  /* append the rest elements to the first one */
  while (a->count) {
    v = lval_join(v, lval_pop(a, 0));
  }

  lval_del(a);
  return v;
}

BUILTIN(LAMBDA)
{
  /* must have 2 arguments */
  LASSERT_NUM(KW_LAMBDA, a, 2);

  /* every argument must be a Q-Expression */
  LASSERT_TYPE(KW_LAMBDA, a, 0, LVAL_QEXPR);
  LASSERT_TYPE(KW_LAMBDA, a, 1, LVAL_QEXPR);

  /* the formal parameters must be only symbols */
  for (int i = 0; i < a->cell[0]->count; i++) {
    LASSERT(a, a->cell[0]->cell[i]->type == LVAL_SYM,
        "cannot define non-symbol. got '%s', expected '%s.'",
        ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
  }

  /* pop the arguments */
  lval* formals = lval_pop(a, 0);
  lval* body = lval_pop(a, 0);
  lval_del(a);

  return lval_lambda(formals, body);
}

lval* _bt_op(lenv* e, lval* a, char* op)
{
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM) {
      lval* err = lval_err("function '%s' passed incorrect type for argument %i. got '%s', expected '%s'",
          op, i, ltype_name(a->cell[i]->type), ltype_name(LVAL_NUM));
      lval_del(a);
      return err;
    }
  }

  /* pop the first element */
  lval* x = lval_pop(a, 0);

  if ((is(op, KW_SUB)) && a->count == 0) {
    x->num = -x->num;
  }

  while (a->count > 0) {
    /* pop the next element */
    lval* y = lval_pop(a, 0);

    if (is(op, KW_ADD)) { x->num += y->num; }
    if (is(op, KW_SUB)) { x->num -= y->num; }
    if (is(op, KW_MUL)) { x->num *= y->num; }
    if (is(op, KW_DIV)) {
      if (y->num == 0) {
        lval_del(x);
        x = lval_err("division by zero");
      } else {
        x->num /= y->num;
      }
    }

    lval_del(y);
  }

  lval_del(a);
  return x;
}

BUILTIN(ADD) { return _bt_op(e, a, KW_ADD); }
BUILTIN(SUB) { return _bt_op(e, a, KW_SUB); }
BUILTIN(MUL) { return _bt_op(e, a, KW_MUL); }
BUILTIN(DIV) { return _bt_op(e, a, KW_DIV); }

lval* _bt_ord(lenv* e, lval* a, char* op)
{
  /* must have two arguments */
  LASSERT_NUM(op, a, 2);

  /* and those arguments must be numbers */
  LASSERT_TYPE(op, a, 0, LVAL_NUM);
  LASSERT_TYPE(op, a, 1, LVAL_NUM);

  int num = 0;

  int left = a->cell[0]->num;
  int right = a->cell[1]->num;

  lval_del(a);

  if      (is(op, KW_GTE))  { num = left >= right; }
  else if (is(op, KW_LTE))  { num = left <= right; }
  else if (is(op, KW_GT))   { num = left >  right; }
  else if (is(op, KW_LT))   { num = left <  right; }

  return lval_num(num);
}

BUILTIN(GT)  { return _bt_ord(e, a, KW_GT);  }
BUILTIN(GTE) { return _bt_ord(e, a, KW_GTE); }
BUILTIN(LT)  { return _bt_ord(e, a, KW_LT);  }
BUILTIN(LTE) { return _bt_ord(e, a, KW_LTE); }

lval* _bt_cmp(lenv* e, lval* a, char* op)
{
  /* must have two arguments */
  LASSERT_NUM(op, a, 2);

  lval* left = lval_pop(a, 0);
  lval* right = lval_pop(a, 0);

  lval* result = NULL;

  if (is(op, KW_EQ)) {
    result = lval_num(lval_eq(left, right));
  } else if (is(op, KW_NEQ)) {
    result = lval_num(!lval_eq(left, right));
  }

  if (result) {
    lval_del(left);
    lval_del(right);
    lval_del(a);
    return result;
  }

  return lval_err("invalid comparison operator %s", op);
}

BUILTIN(EQ)  { return _bt_cmp(e, a, KW_EQ);  }
BUILTIN(NEQ) { return _bt_cmp(e, a, KW_NEQ); }

BUILTIN(IF)
{
  /** must have 2 or 3 arguments **/
  LASSERT_NUM_OR(KW_IF, a, 2, 3);

  /** the first 2 must be are required and must be a Number and a Q-Expr **/
  LASSERT_TYPE(KW_IF, a, 0, LVAL_NUM);
  LASSERT_TYPE(KW_IF, a, 1, LVAL_QEXPR);

  lval* x = lval_sexpr();

  if (a->cell[0]->num) {
    /** if the first argument is true, evaluate the "true" part **/
    a->cell[1]->type = LVAL_SEXPR;
    x = leval(e, lval_pop(a, 1));
  } else if (a->count == 3) {
    /** if the first argument is false, evaluate the "false" part **/
    LASSERT_TYPE(KW_IF, a, 2, LVAL_QEXPR);
    a->cell[2]->type = LVAL_SEXPR;
    x = leval(e, lval_pop(a, 2));
  }

  lval_del(a);

  return x;
}

BUILTIN(LOAD)
{
  LASSERT_NUM(KW_LOAD, a, 1);
  LASSERT_TYPE(KW_LOAD, a, 0, LVAL_STR);

  lval* r = lparser_parse(e, a->cell[0]->str);
  if (r) {
    while (r->count) {
      lval* p = lval_pop(r, 0);
      lval* x = leval(e, p);
      if (x->type == LVAL_ERR) {
        lval_println(x);
      }
      lval_del(x);
    }
    lval_del(r);
    lval_del(a);
    return lval_sexpr();
  } else {
    lval* err = lval_err("could not load %s", a->cell[0]->str);
    lval_del(a);
    return err;
  }
}

BUILTIN(PRINT)
{
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type == LVAL_STR) {
      lval_print_str(a->cell[i], NULL, NULL);
    } else {
      lval_print(a->cell[i]);
    }
  }
  lval_del(a);
  return lval_sexpr();
}

BUILTIN(PRINTLN)
{
  lval* v = BTNAME(PRINT)(e, a);
  putchar('\n');
  return v;
}

BUILTIN(ERROR)
{
  /** must have 1 argument **/
  LASSERT_NUM(KW_ERROR, a, 1);

  /** that argument must be a string **/
  LASSERT_TYPE(KW_ERROR, a, 0, LVAL_STR);

  lval* err = lval_err(a->cell[0]->str);
  lval_del(a);

  return err;
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin func)
{
  lval* k = lval_sym(name);
  lval* v = lval_fun(func);
  lenv_put(e, k, v);
  lval_del(k);
  lval_del(v);
}

#define ADD_BTIN(N) lenv_add_builtin(e, KW_ ## N, BTNAME(N))

void lenv_add_builtins(lenv* e)
{
  /** mathematical functions **/
  ADD_BTIN(ADD);
  ADD_BTIN(SUB);
  ADD_BTIN(MUL);
  ADD_BTIN(DIV);

  /** order functions **/
  ADD_BTIN(GT);
  ADD_BTIN(GTE);
  ADD_BTIN(LT);
  ADD_BTIN(LTE);

  /** equality functions **/
  ADD_BTIN(EQ);
  ADD_BTIN(NEQ);

  /** list functions **/
  ADD_BTIN(LIST);
  ADD_BTIN(HEAD);
  ADD_BTIN(TAIL);
  ADD_BTIN(EVAL);
  ADD_BTIN(JOIN);

  /** lambda **/
  ADD_BTIN(LAMBDA);

  /** def functions **/
  ADD_BTIN(GDEF);
  ADD_BTIN(LDEF);

  /** conditionals function **/
  ADD_BTIN(IF);

  /** load function **/
  ADD_BTIN(LOAD);
  ADD_BTIN(PRINT);
  ADD_BTIN(PRINTLN);
  ADD_BTIN(ERROR);
}

#undef ADD_BTIN

#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

#ifdef _WIN32
#include "prompt_win.h"
#else
#include <editline/readline.h>
#endif

#define VERSION "0.0.1"

#define LASSERT(args, cond, err) \
  if (!(cond)) { lval_del(args); return lval_err(err); }


/** forward declarations **/
struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;


/** TYPES of lval **/
enum {  LVAL_ERR, LVAL_SYM,   LVAL_NUM,
        LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR };


typedef lval*(*lbuiltin)(lenv*, lval*);


/****** lval struct ******/
struct lval
{
  /** the type of lval (one of the enum 'TYPES') **/
  int type;

  /** value for type LVAL_NUM **/
  long num;

  /** value for type LVAL_ERR **/
  char* err;

  /** value for type LVAL_SYM **/
  char* sym;

  /** value for type LVAL_FUN **/
  lbuiltin fun;

  /** values for type LVAL_SEXPR and LVAL_QEXPR **/
  int count;
  lval** cell;
};


/****** lenv struct ******/
struct lenv
{
  /** number of items in the environment **/
  int count;

  /** list of symbol names (as char*) **/
  char** syms;

  /** list of symbols (as lval*) **/
  lval** vals;
};


/* ====== ENV ====== */
lenv* lenv_new(void);
void lenv_del(lenv* e);
lval* lenv_get(lenv* e, lval* key);
void lenv_put(lenv* e, lval* key, lval* value);
void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
void lenv_add_builtins(lenv* e);
lval* lval_copy(lval* v);


/* ====== CONSTRUCTORS ====== */
lval* lval_num(long n);
lval* lval_err(char* m);
lval* lval_sym(char* s);
lval* lval_fun(lbuiltin func);
lval* lval_sexpr(void);
lval* lval_qexpr(void);

/* ====== DESTRUCTOR ====== */
void lval_del(lval* v);

/* ====== READERS ====== */
lval* lval_read_num(mpc_ast_t* t);
lval* lval_add(lval* v, lval* x);
lval* lval_read(mpc_ast_t* t);

/* ====== PRINTERS ====== */
void lval_expr_print(lval* v, char open, char close);
void lval_print(lval* v);
void lval_println(lval* v);

/* ====== LIST MANAGEMENT ====== */
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* lval_join(lval* x, lval* y);

/* ====== BUILTINS ====== */
lval* builtin_head(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_list(lenv* e, lval* a);
lval* builtin_eval(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_op(lenv* e, lval* a, char* op);

/* ====== EVAL ====== */
lval* lval_eval_sexpr(lenv* e, lval* v);
lval* lval_eval(lenv* e, lval* v);


/****** ENV ******/
lenv* lenv_new(void)
{
  lenv* e = malloc(sizeof(lenv));
  e->count = 0;
  e->syms = NULL;
  e->vals = NULL;
  return e;
}

void lenv_del(lenv* e)
{
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
    if (strcmp(e->syms[i], k->sym) == 0) {
      return lval_copy(e->vals[i]);
    }
  }

  /* if the symbol wasn't found then is not bounded, you should bound it first */
  return lval_err("unbound symbol!");
}

void lenv_put(lenv* e, lval* k, lval* v)
{
  /* go over all items in environment */
  for (int i = 0; i < e->count; i++) {
    /* if the symbol is found then delete it and replace it with the new one */
    if (strcmp(e->syms[i], k->sym) == 0) {
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

lval* lval_copy(lval* v)
{
  lval* x = malloc(sizeof(lval));
  x->type = v->type;

  switch (v->type)
  {
    case LVAL_FUN:
      x->fun = v->fun;
      break;

    case LVAL_NUM:
      x->num = v->num;
      break;

    case LVAL_ERR:
      x->err = malloc(strlen(v->err) + 1);
      strcpy(x->err, v->err);
      break;

    case LVAL_SYM:
      x->sym = malloc(strlen(v->sym) + 1);
      strcpy(x->sym, v->sym);
      break;

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      x->count = v->count;
      x->cell = malloc(sizeof(lval*) * x->count);
      for (int i = 0; i < x->count; i++) {
        x->cell[i] = lval_copy(v->cell[i]);
      }
      break;
  }

  return x;
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin func)
{
  lval* k = lval_sym(name);
  lval* v = lval_fun(func);
  lenv_put(e, k, v);
  lval_del(k);
  lval_del(v);
}

void lenv_add_builtins(lenv* e)
{
  /** mathematical functions **/
  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);

  /** list functions **/
  lenv_add_builtin(e, "list", builtin_list);
  lenv_add_builtin(e, "head", builtin_head);
  lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "eval", builtin_eval);
  lenv_add_builtin(e, "join", builtin_join);
}


/****** CONSTRUCTORS ******/
lval* lval_num(long n)
{
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = n;
  return v;
}

lval* lval_err(char* m)
{
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(m) + 1);
  strcpy(v->err, m);
  return v;
}

lval* lval_sym(char* s)
{
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

lval* lval_fun(lbuiltin func)
{
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->fun = func;
  return v;
}

lval* lval_sexpr(void)
{
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

lval* lval_qexpr(void)
{
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}


/****** DESTRUCTOR ******/
void lval_del(lval* v)
{
  switch (v->type)
  {
    case LVAL_NUM:
    case LVAL_FUN:
        break;

    case LVAL_ERR:
        free(v->err);
        break;

    case LVAL_SYM:
        free(v->sym);
        break;

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      for (int i = 0; i < v->count; i++) {
        lval_del(v->cell[i]);
      }
      free(v->cell);
      break;
  }

  free(v);
}


/****** PRINT ******/
void lval_expr_print(lval* v, char open, char close)
{
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    lval_print(v->cell[i]);
    if (i != (v->count - 1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void lval_print(lval* v)
{
  switch (v->type)
  {
    case LVAL_NUM:
      printf("%li", v->num);
      break;

    case LVAL_ERR:
      printf("Error: %s", v->err);
      break;

    case LVAL_SYM:
      printf("%s", v->sym);
      break;

    case LVAL_FUN:
      printf("<function [TODO]>");
      break;

    case LVAL_SEXPR:
      lval_expr_print(v, '(', ')');
      break;

    case LVAL_QEXPR:
      lval_expr_print(v, '{', '}');
      break;
  }
}

void lval_println(lval* v)
{
  lval_print(v);
  putchar('\n');
}


/****** READ ******/
lval* lval_read_num(mpc_ast_t* t)
{
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval* lval_add(lval* v, lval* x)
{
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  v->cell[v->count - 1] = x;
  return v;
}

lval* lval_read(mpc_ast_t* t)
{
  if (strstr(t->tag, "number")) { return lval_read_num(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

  lval* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
  if (strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }

  return x;
}


/****** LISTS MANAGEMENT ******/
lval* lval_pop(lval* v, int i)
{
  lval* x = v->cell[i];
  memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*) * (v->count - i - 1));
  v->count--;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  return x;
}

lval* lval_take(lval* v, int i)
{
  lval* x = lval_pop(v, i);
  lval_del(v);
  return x;
}

lval* lval_join(lval* x, lval* y)
{
  while (y->count) {
    x = lval_add(x, lval_pop(y, 0));
  }

  lval_del(y);
  return x;
}


/****** BULTINS ******/
lval* builtin_head(lenv* e, lval* a)
{
  LASSERT(a, a->count == 1,
      "function 'head' passed too many arguments!");

  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
      "function 'head' passed incorrect type!");

  LASSERT(a, a->cell[0]->count != 0,
      "function 'head' passed an empty list");

  /* take the first argument */
  lval* v = lval_take(a, 0);

  /* delete all elements that are not the head */
  while (v->count > 1) {
    lval_del(lval_pop(v, 1));
  }

  return v;
}

lval* builtin_tail(lenv* e, lval* a)
{
  LASSERT(a, a->count == 1,
      "function 'tail' passed too many arguments!");

  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
      "function 'tail' passed incorrect type!");

  LASSERT(a, a->cell[0]->count != 0,
      "function 'tail' passed an empty list");

  /* take the first argument */
  lval* v = lval_take(a, 0);

  /* delete the first element */
  lval_del(lval_pop(v, 0));

  return v;
}

lval* builtin_list(lenv* e, lval* a)
{
  /* list just transforms any expression into a qexpr */
  a->type = LVAL_QEXPR;
  return a;
}

lval* builtin_eval(lenv* e, lval* a)
{
  LASSERT(a, a->count == 1,
      "function 'eval' passed too many arguments!");

  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
      "function 'eval' passed incorrect type!");

  /* take the first argument */
  lval* v = lval_take(a, 0);
  /* make it an sexpr */
  v->type = LVAL_SEXPR;
  /* and evaluate it */
  return lval_eval(e, v);
}

lval* builtin_join(lenv* e, lval* a)
{
  for (int i = 0; i < a->count; i++) {
    LASSERT(a, a->cell[i]->type == LVAL_QEXPR,
        "function 'join' passed incorrect type");
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

lval* builtin_add(lenv* e, lval* a)
{
  return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a)
{
  return builtin_op(e, a, "-");
}

lval* builtin_mul(lenv* e, lval* a)
{
  return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a)
{
  return builtin_op(e, a, "/");
}

lval* builtin_op(lenv* e, lval* a, char* op)
{
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM) {
      lval_del(a);
      return lval_err("cannot operate on non-number!");
    }
  }

  /* pop the first element */
  lval* x = lval_pop(a, 0);

  if ((strcmp(op, "-") == 0) && a->count == 0) {
    x->num = -x->num;
  }

  while (a->count > 0) {
    /* pop the next element */
    lval* y = lval_pop(a, 0);

    if (strcmp(op, "+") == 0) { x->num += y->num; }
    if (strcmp(op, "-") == 0) { x->num -= y->num; }
    if (strcmp(op, "*") == 0) { x->num *= y->num; }
    if (strcmp(op, "/") == 0) {
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


/****** EVAL ******/
lval* lval_eval_sexpr(lenv* e, lval* v)
{
  /* evaluate all children of expression, if any of those is an error, return that */
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(e, v->cell[i]);
    if (v->cell[i]->type == LVAL_ERR) {
      return lval_take(v, i);
    }
  }

  /* expression with no children */
  if (v->count == 0) {
    return v;
  }

  /* expression with just one children: return that children */
  if (v->count == 1) {
    return lval_take(v, 0);
  }

  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_FUN) {
    lval_del(v);
    lval_del(f);
    return lval_err("S-expression does not start with a function!");
  }

  lval* result = f->fun(e, v);
  lval_del(f);
  return result;
}

lval* lval_eval(lenv* e, lval* v)
{
  if (v->type == LVAL_SYM) {
    lval* x = lenv_get(e, v);
    lval_del(v);
    return x;
  }

  if (v->type == LVAL_SEXPR) {
    return lval_eval_sexpr(e, v);
  }

  return v;
}


/****** REPL ******/
int main (int argc, char** argv)
{
  /* Create some parsers */
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* Sexpr = mpc_new("sexpr");
  mpc_parser_t* Qexpr = mpc_new("qexpr");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  /* Define them with the folowwing Language */
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                       \
      number    : /-?[0-9]+/ ;                              \
      symbol    : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;        \
      sexpr     : '(' <expr>* ')' ;                         \
      qexpr     : '{' <expr>* '}' ;                         \
      expr      : <number> | <symbol> | <sexpr> | <qexpr> ; \
      lispy     : /^/ <expr>* /$/ ;                         \
    ",
    Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

  /* Start repl */
  puts("Lisp Version " VERSION);
  puts("Press Ctrl+c to Exit\n");

  lenv* e = lenv_new();
  lenv_add_builtins(e);

  while (1)
  {
    char* input = readline("lisp> ");
    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      lval* x = lval_eval(e, lval_read(r.output));
      lval_println(x);
      lval_del(x);
//       mpc_ast_print(r.output);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  lenv_del(e);

  mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

  return 0;
}

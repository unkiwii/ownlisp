#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

#ifdef _WIN32
#include "prompt_win.h"
#else
#include <editline/readline.h>
#endif

#define DEBUG 0

#define VERSION "0.0.1"

#define ERR_MSG_BUFFER_SIZE 512

#define LASSERT(args, cond, fmt, ...)         \
  if (!(cond)) {                              \
    lval* err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args);                           \
    return err;                               \
  }


/** forward declarations **/
struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;


/** TYPES of lval **/
enum {  LVAL_ERR, LVAL_SYM,   LVAL_NUM,
        LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR,
        LVAL_CMD }; // only for repl

/** TYPES of function **/
enum { FUN_BUILTIN, FUN_DEF };


typedef lval*(*lbuiltin)(lenv*, lval*);
typedef void(*lcmd)(lenv*);


/****** lval struct ******/
struct lval
{
  /** the type of lval (one of the enum 'TYPES') **/
  int type;

  /** cmd for type LVAL_CMD **/
  lcmd cmd;

  /** value for type LVAL_NUM **/
  long num;

  /** value for type LVAL_ERR **/
  char* err;

  /** value for type LVAL_SYM **/
  char* sym;

  /** value for type LVAL_FUN **/
  lbuiltin fun;
  int funtype;

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


/****** lrepl struct ******/
typedef struct lrepl
{
  int cmd_count;
  char** cmd_names;
  lcmd* cmds;
} lrepl;


int is(char* a, char* b)
{
  if (!a) { return 0; }
  if (!b) { return 0; }
  return strncmp(a, b, strlen(b)) == 0;
}

int has(char* a, char* b)
{
  if (!a) { return 0; }
  if (!b) { return 0; }
  return strstr(a, b) != 0;
}


/* ====== DEBUG ====== */
char* ltype_name(int type)
{
  switch (type)
  {
    case LVAL_CMD: return "Interpreter Command";
    case LVAL_FUN: return "Function";
    case LVAL_NUM: return "Number";
    case LVAL_ERR: return "Error";
    case LVAL_SYM: return "Symbol";
    case LVAL_SEXPR: return "S-Expression";
    case LVAL_QEXPR: return "Q-Expression";
  }

  return "Unknown";
}

/* ====== REPL ====== */
lrepl* lrepl_new();
void lrepl_add_cmd(lrepl* r, char* name, lcmd func);
void lrepl_add_cmds(lrepl* r);

/* ====== REPL COMMANDS ======*/
void cmd_exit(lenv* e);
void cmd_help(lenv* e);
void cmd_show_env(lenv* e);

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
lval* lval_err(char* fmt, ...);
lval* lval_sym(char* s);
lval* lval_fun(lbuiltin func, int funtype);
lval* lval_cmd(lcmd func);
lval* lval_sexpr(void);
lval* lval_qexpr(void);

/* ====== DESTRUCTOR ====== */
void lval_del(lval* v);

/* ====== READERS ====== */
lval* lval_read_num(mpc_ast_t* t);
lval* lval_add(lval* v, lval* x);
lval* lval_read(mpc_ast_t* t, int n);

/* ====== PRINTERS ====== */
void lval_expr_print(lval* v, char open, char close);
void lval_print(lval* v);
void lval_println(lval* v);

/* ====== LIST MANAGEMENT ====== */
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* lval_join(lval* x, lval* y);

/* ====== BUILTINS ====== */
lval* builtin_def(lenv* e, lval* a);
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
lval* lval_eval_sexpr(lenv* e, lval* v, lrepl* r);
lval* lval_eval(lenv* e, lval* v, lrepl* r);


/****** REPL COMMANDS ******/
void cmd_exit(lenv* e)
{
  exit(0);
}

void cmd_help(lenv* e)
{
  printf("\n"
    "repl commands:\n"
    "\n"
    "  exit, quit, q  exits from repl\n"
    "  env            prints environment\n"
    "  help           prints this message\n"
    "\n");
}

void cmd_show_env(lenv* e)
{
  puts("{");
  for (int i = 0; i < e->count; i++) {
    printf("  %s: ", e->syms[i]);
    lval_println(e->vals[i]);
  }
  puts("}");
}


/****** REPL ******/
lrepl* lrepl_new()
{
  lrepl* repl = malloc(sizeof(lrepl));
  repl->cmd_count = 0;
  repl->cmd_names = NULL;
  repl->cmds = NULL;
  return repl;
}

void lrepl_add_cmd(lrepl* r, char* name, lcmd func)
{
  /* go over all items in repl */
  for (int i = 0; i < r->cmd_count; i++) {
    /* if the name is found then replace it */
    if (is(r->cmd_names[i], name)) {
      r->cmd_names[i] = name;
      r->cmds[i] = func;
      return;
    }
  }

  /* if no existing entry found, then allocate space for new entry */
  r->cmd_count++;
  r->cmd_names = realloc(r->cmd_names, sizeof(char*) * r->cmd_count);
  r->cmds = realloc(r->cmds, sizeof(lcmd) * r->cmd_count);

  /* and save a new entry */
  r->cmds[r->cmd_count - 1] = func;
  r->cmd_names[r->cmd_count - 1] = malloc(strlen(name) + 1);
  strcpy(r->cmd_names[r->cmd_count - 1], name);
}

void lrepl_add_cmds(lrepl* r)
{
  lrepl_add_cmd(r, "exit", cmd_exit);
  lrepl_add_cmd(r, "quit", cmd_exit);
  lrepl_add_cmd(r, "q", cmd_exit);
  lrepl_add_cmd(r, "help", cmd_help);
  lrepl_add_cmd(r, "env", cmd_show_env);
}


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
    if (is(e->syms[i], k->sym)) {
      return lval_copy(e->vals[i]);
    }
  }

  /* if the symbol wasn't found then is not bounded, you should bound it first */
  return lval_err("unbound symbol %s", k->sym);
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

lval* lval_copy(lval* v)
{
  lval* x = malloc(sizeof(lval));
  x->type = v->type;

  switch (v->type)
  {
    case LVAL_FUN:
      x->fun = v->fun;
      x->funtype = v->funtype;
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
  lval* v = lval_fun(func, FUN_BUILTIN);
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
  lenv_add_builtin(e, "def", builtin_def);
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

lval* lval_err(char* fmt, ...)
{
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;

  va_list va;
  va_start(va, fmt);

  v->err = malloc(ERR_MSG_BUFFER_SIZE);
  vsnprintf(v->err, ERR_MSG_BUFFER_SIZE - 1, fmt, va);
  v->err = realloc(v->err, strlen(v->err) + 1);

  va_end(va);

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

lval* lval_fun(lbuiltin func, int funtype)
{
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->fun = func;
  v->funtype = funtype;
  return v;
}

lval* lval_cmd(lcmd func)
{
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_CMD;
  v->cmd = func;
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
    case LVAL_CMD:
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
      if (v->funtype == FUN_BUILTIN) {
        printf("<builtin>");
      } else {
        printf("<function>");
      }
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

void debug_read(char* m, int n, int children_num)
{
#if DEBUG
  for (int i = 0; i < n; i++ ) {
    printf("  ");
  }
  printf("%s (%i)\n", m, children_num);
#endif
}

lval* lval_read(mpc_ast_t* t, int n)
{
  if (has(t->tag, "number")) {
    debug_read("number", n, 0);
    return lval_read_num(t);
  }

  if (has(t->tag, "symbol")) {
    debug_read("symbol", n, 0);
    return lval_sym(t->contents);
  }

  lval* x = NULL;

  if (has(t->tag, ">")) {
    debug_read("sepxr >", n, t->children_num);
    x = lval_sexpr();
  }

  if (has(t->tag, "sexpr")) {
    debug_read("sepxr", n, t->children_num);
    x = lval_sexpr();
  }

  if (has(t->tag, "qexpr")) {
    debug_read("qepxr", n, t->children_num);
    x = lval_qexpr();
  }

  for (int i = 0; i < t->children_num; i++) {
    if (is(t->children[i]->contents, "(")) { continue; }
    if (is(t->children[i]->contents, ")")) { continue; }
    if (is(t->children[i]->contents, "{")) { continue; }
    if (is(t->children[i]->contents, "}")) { continue; }
    if (is(t->children[i]->tag, "regex")) { continue; }
    debug_read(t->children[i]->tag, n + 1, 0);
    x = lval_add(x, lval_read(t->children[i], n + 1));
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
lval* builtin_def(lenv* e, lval* a)
{
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
      "function 'def' passed incorrect type for argument 0. "
      "got '%s', expected '%s'.", ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

  /* first argument is a symbol list */
  lval* syms = a->cell[0];

  for (int i = 0; i < syms->count; i++) {
    LASSERT(a, syms->cell[i]->type == LVAL_SYM,
        "function 'def' cannot define non-symbol. "
        "got '%s', expected '%s'.", ltype_name(a->cell[i]->type), ltype_name(LVAL_QEXPR));
  }

  LASSERT(a, syms->count == a->count - 1,
      "function 'def' cannot define incorrect number of values to symbols. "
      "got %i, expected %i.", syms->count, a->count - 1);

  for (int i = 0; i < syms->count; i++) {
    lenv_put(e, syms->cell[i], a->cell[i + 1]);
  }

  lval_del(a);

  return lval_sexpr();
}

lval* builtin_head(lenv* e, lval* a)
{
  LASSERT(a, a->count == 1,
      "function 'head' passed too many arguments. "
      "got %i, expected %i", a->count, 1);

  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
      "function 'head' passed incorrect type. "
      "got '%s', expected '%s'.", ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

  LASSERT(a, a->cell[0]->count != 0,
      "function 'head' passed an empty list.");

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
      "function 'tail' passed too many arguments. "
      "got %i, expected %i", a->count, 1);

  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
      "function 'tail' passed incorrect type. "
      "got '%s', expected '%s'.", ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

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
      "function 'eval' passed too many arguments. "
      "got %i, expected %i", a->count, 1);

  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
      "function 'eval' passed incorrect type. "
      "got '%s', expected '%s'.", ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

  /* take the first argument */
  lval* v = lval_take(a, 0);
  /* make it an sexpr */
  v->type = LVAL_SEXPR;
  /* and evaluate it */
  return lval_eval(e, v, NULL);
}

lval* builtin_join(lenv* e, lval* a)
{
  for (int i = 0; i < a->count; i++) {
    LASSERT(a, a->cell[i]->type == LVAL_QEXPR,
        "function 'join' passed incorrect type. "
        "got '%s', expected '%s'.", ltype_name(a->cell[i]->type), ltype_name(LVAL_QEXPR));
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
      lval* err = lval_err("function '%s' passed incorrect type for argument %i. got '%s', expected '%s'",
          op, i, ltype_name(a->cell[i]->type), ltype_name(LVAL_NUM));
      lval_del(a);
      return err;
    }
  }

  /* pop the first element */
  lval* x = lval_pop(a, 0);

  if ((is(op, "-")) && a->count == 0) {
    x->num = -x->num;
  }

  while (a->count > 0) {
    /* pop the next element */
    lval* y = lval_pop(a, 0);

    if (is(op, "+")) { x->num += y->num; }
    if (is(op, "-")) { x->num -= y->num; }
    if (is(op, "*")) { x->num *= y->num; }
    if (is(op, "/")) {
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
lval* lval_eval_sexpr(lenv* e, lval* v, lrepl* r)
{
  /* evaluate all children of expression, if any of those is an error, return that */
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(e, v->cell[i], r);
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
    lval* err = lval_err("%s does not start with a function", ltype_name(v->type));
    lval_del(v);
    lval_del(f);
    return err;
  }

  lval* result = f->fun(e, v);
  lval_del(f);
  return result;
}

lval* lval_eval(lenv* e, lval* v, lrepl* r)
{
  if (v->type == LVAL_SYM) {
    /* check if the symbol is a repl command */
    for (int i = 0; i < r->cmd_count; i++) {
      if (is(v->sym, r->cmd_names[i])) {
        lval_del(v);
        return lval_cmd(r->cmds[i]);
      }
    }
    lval* x = lenv_get(e, v);
    lval_del(v);
    return x;
  }

  if (v->type == LVAL_SEXPR) {
    return lval_eval_sexpr(e, v, r);
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

  lrepl* repl = lrepl_new();
  lrepl_add_cmds(repl);

  lenv* e = lenv_new();
  lenv_add_builtins(e);

  while (1)
  {
    char* input = readline("lisp> ");
    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
#if DEBUG
      mpc_ast_print(r.output);
#endif
      lval* x = lval_eval(e, lval_read(r.output, 0), repl);
      if (x->type == LVAL_CMD) {
        x->cmd(e);
      } else {
        lval_println(x);
      }
      lval_del(x);
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

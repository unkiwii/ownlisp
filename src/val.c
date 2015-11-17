#include "val.h"
#include "utils.h"
#include "mpc.h"

#define ERR_MSG_BUFFER_SIZE 512

char* ltype_name(int type)
{
  switch (type) {
    case LVAL_FUN:    return  "Function";
    case LVAL_NUM:    return  "Number";
    case LVAL_ERR:    return  "Error";
    case LVAL_SYM:    return  "Symbol";
    case LVAL_STR:    return  "String";
    case LVAL_SEXPR:  return  "S-Expression";
    case LVAL_QEXPR:  return  "Q-Expression";
  }

  return "Unknown";
}

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

lval* lval_str(char* s)
{
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_STR;
  v->str = malloc(strlen(s) + 1);
  strcpy(v->str, s);
  return v;
}

lval* lval_fun(lbuiltin func)
{
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->builtin = func;
  return v;
}

lval* lval_lambda(lval* formals, lval* body)
{
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->builtin = NULL;
  v->env = lenv_new();
  v->formals = formals;
  v->body = body;
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

lval* lval_copy(lval* v)
{
  lval* x = malloc(sizeof(lval));
  x->type = v->type;

  switch (v->type)
  {
    case LVAL_FUN:
      if (v->builtin) {
        x->builtin = v->builtin;
      } else {
        x->builtin = NULL;
        x->env = lenv_copy(v->env);
        x->formals = lval_copy(v->formals);
        x->body = lval_copy(v->body);
      }
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

    case LVAL_STR:
      x->str = malloc(strlen(v->str) + 1);
      strcpy(x->str, v->str);
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

void lval_del(lval* v)
{
  switch (v->type)
  {
    case LVAL_NUM:
      break;

    case LVAL_FUN:
      if (!v->builtin) {
        lenv_del(v->env);
        lval_del(v->formals);
        lval_del(v->body);
      }
      break;

    case LVAL_ERR:
      free(v->err);
      break;

    case LVAL_SYM:
      free(v->sym);
      break;

    case LVAL_STR:
      free(v->str);
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

lval* lval_add(lval* v, lval* x)
{
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  v->cell[v->count - 1] = x;
  return v;
}

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

int lval_print_expr(lval* v, char open, char close)
{
  if (v->count > 0) {
    putchar(open);
    for (int i = 0; i < v->count; i++) {
      lval_print(v->cell[i]);
      if (i != (v->count - 1)) {
        putchar(' ');
      }
    }
    putchar(close);
    return 1;
  }

  return 0;
}

void lval_print_str(lval* v, char* open, char* close)
{
  char* escaped = malloc(strlen(v->str) + 1);
  strcpy(escaped, v->str);
  escaped = mpcf_escape(escaped);
  if (open) { printf("%s", open); }
  printf("%s", escaped);
  if (close) { printf("%s", close); }
  free(escaped);
}

int lval_print(lval* v)
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

    case LVAL_STR:
      lval_print_str(v, "\"", "\"");
      break;

    case LVAL_FUN:
      if (v->builtin) {
        printf("<builtin>");
      } else {
        printf("(\\ ");
        lval_print(v->formals);
        putchar(' ');
        lval_print(v->body);
        putchar(')');
      }
      break;

    case LVAL_SEXPR:
      return lval_print_expr(v, '(', ')');

    case LVAL_QEXPR:
      return lval_print_expr(v, '{', '}');

    default:
      return 0;
  }

  return 1;
}

void lval_println(lval* v)
{
  if (lval_print(v)) {
    putchar('\n');
  }
}

int lval_eq(lval* a, lval* b)
{
  if (a->type != b->type) {
    return 0;
  }

  switch (a->type) {
    case LVAL_NUM: return a->num == b->num;

    case LVAL_ERR: return is(a->err, b->err);
    case LVAL_SYM: return is(a->sym, b->sym);
    case LVAL_STR: return is(a->str, b->str);

    case LVAL_FUN:
      if (a->builtin || b->builtin) {
        return (a->builtin == b->builtin);
      } else {
        return lval_eq(a->formals, b->formals) && lval_eq(a->body, b->body);
      }

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      if (a->count != b->count) {
        return 0;
      }

      for (int i = 0; i < a->count; i++) {
        if (!lval_eq(a->cell[i], b->cell[i])) {
          return 0;
        }
      }

      return 1;
  }

  return 0;
}

#include "parser.h"
#include "val.h"
#include "utils.h"

lparser* lparser_new(void)
{
  lparser* p = malloc(sizeof(lparser));

  p->Number = mpc_new("number");
  p->String = mpc_new("string");
  p->Comment = mpc_new("comment");
  p->Symbol = mpc_new("symbol");
  p->Sexpr = mpc_new("sexpr");
  p->Qexpr = mpc_new("qexpr");
  p->Expr = mpc_new("expr");
  p->Lispy = mpc_new("lispy");

  /* Define them with the folowwing Language */
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                       \
      number    : /-?[0-9]+/ ;                              \
      symbol    : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!:\?]+/ ;      \
      string    : /\"(\\\\.|[^\"])*\"/ ;                    \
      comment   : /;[^\\r\\n]*/ ;                           \
      sexpr     : '(' <expr>* ')' ;                         \
      qexpr     : '{' <expr>* '}' ;                         \
      expr      : <number>  | <string> | <symbol>           \
                | <comment> | <sexpr>  | <qexpr> ;          \
      lispy     : /^/ <expr>* /$/ ;                         \
    ",
    p->Number, p->String, p->Comment,
    p->Symbol, p->Sexpr, p->Qexpr,
    p->Expr, p->Lispy);

  return p;
}

void lparser_del(lparser* p)
{
  mpc_cleanup(8,
      p->Number, p->String, p->Comment, p->Symbol,
      p->Sexpr, p->Qexpr, p->Expr, p->Lispy);

  free(p);
}

lval* lparser_read_num(mpc_ast_t* t)
{
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval* lparser_read_str(mpc_ast_t* t)
{
  char* str = t->contents;
  str[strlen(str) - 1] = '\0';
  char* unescaped = malloc(strlen(str + 1) + 1);
  strcpy(unescaped, str + 1);
  unescaped = mpcf_unescape(unescaped);
  lval* s = lval_str(unescaped);
  free(unescaped);
  return s;
}

lval* lparser_read(mpc_ast_t* t)
{
  if (has(t->tag, "number")) {
    return lparser_read_num(t);
  }

  if (has(t->tag, "string")) {
    return lparser_read_str(t);
  }

  if (has(t->tag, "symbol")) {
    return lval_sym(t->contents);
  }

  lval* x = NULL;

  if (has(t->tag, ">")) {
    x = lval_sexpr();
  }

  if (has(t->tag, "sexpr")) {
    x = lval_sexpr();
  }

  if (has(t->tag, "qexpr")) {
    x = lval_qexpr();
  }

  for (int i = 0; i < t->children_num; i++) {
    if (is(t->children[i]->contents, "("))   { continue; }
    if (is(t->children[i]->contents, ")"))   { continue; }
    if (is(t->children[i]->contents, "{"))   { continue; }
    if (is(t->children[i]->contents, "}"))   { continue; }
    if (is(t->children[i]->tag, "regex"))    { continue; }
    if (has(t->children[i]->tag, "comment")) { continue; }
    x = lval_add(x, lparser_read(t->children[i]));
  }

  return x;
}

lval* lparser_parse_stdin(lenv* e, char* data)
{
  lparser* p = lenv_getparser(e);
  lval* x = NULL;
  mpc_result_t r;
  if (mpc_parse("<stdin>", data, p->Lispy, &r)) {
    x = lparser_read(r.output);
    if (e->debug) {
      printf("\nAST: ");
      mpc_ast_print(r.output);
      printf("\nEXPR: ");
      lval_println(x);
      printf("\nRESULT: ");
    }
    mpc_ast_delete(r.output);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }
  return x;
}

lval* lparser_parse(lenv* e, char* data)
{
  lparser* p = lenv_getparser(e);
  lval* x = NULL;
  mpc_result_t r;
  if (mpc_parse_contents(data, p->Lispy, &r)) {
    x = lparser_read(r.output);
    if (e->debug) {
      printf("\nAST: ");
      mpc_ast_print(r.output);
      printf("\nEXPR: ");
      lval_println(x);
      printf("\nRESULT: ");
    }
    mpc_ast_delete(r.output);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }
  return x;
}

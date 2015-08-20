#ifndef LISPY_BUILTINS_H
#define LISPY_BUILTINS_H

#include "fwd.h"
#include "env.h"
#include "val.h"

/* lang keywords */
#define   KW_VARG     ":"
#define   KW_GDEF     "def"
#define   KW_LDEF     "="
#define   KW_HEAD     "head"
#define   KW_TAIL     "tail"
#define   KW_LIST     "list"
#define   KW_EVAL     "eval"
#define   KW_JOIN     "join"
#define   KW_LAMBDA   "\\"
#define   KW_ADD      "+"
#define   KW_SUB      "-"
#define   KW_MUL      "*"
#define   KW_DIV      "/"
#define   KW_GT       ">"
#define   KW_GTE      ">="
#define   KW_LT       "<"
#define   KW_LTE      "<="
#define   KW_EQ       "=?"
#define   KW_NEQ      "!="
#define   KW_IF       "if"
#define   KW_LOAD     "load"
#define   KW_PRINT    "print"
#define   KW_PRINTLN  "println"
#define   KW_ERROR    "error"

#define BTNAME(N) builtin_ ## N
#define BUILTIN(N) lval* BTNAME(N) (lenv* e, lval* a)

BUILTIN(GDEF);    /*  def     */
BUILTIN(LDEF);    /*  =       */
BUILTIN(HEAD);    /*  head    */
BUILTIN(TAIL);    /*  tail    */
BUILTIN(LIST);    /*  list    */
BUILTIN(EVAL);    /*  eval    */
BUILTIN(JOIN);    /*  join    */
BUILTIN(LAMBDA);  /*  \       */
BUILTIN(ADD);     /*  +       */
BUILTIN(SUB);     /*  -       */
BUILTIN(MUL);     /*  *       */
BUILTIN(DIV);     /*  /       */
BUILTIN(GT);      /*  >       */
BUILTIN(GTE);     /*  >=      */
BUILTIN(LT);      /*  <       */
BUILTIN(LTE);     /*  <=      */
BUILTIN(EQ);      /*  ==      */
BUILTIN(NEQ);     /*  !=      */
BUILTIN(IF);      /*  if      */
BUILTIN(LOAD);    /*  load    */
BUILTIN(PRINT);   /*  print   */
BUILTIN(PRINTLN); /*  println */
BUILTIN(ERROR);   /*  error   */

void lenv_add_builtins(lenv* e);

#endif//LISPY_BUILTINS_H

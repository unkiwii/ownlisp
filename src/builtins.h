#ifndef LISPY_BUILTINS_H
#define LISPY_BUILTINS_H

#include "fwd.h"
#include "env.h"
#include "val.h"

/* lang keywords */
#define   KW_VARG     ":"
#define   KW_GDEF     "def"
#define   KW_LDEF     ":="
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
#define   KW_EQ       "="
#define   KW_NEQ      "!="
#define   KW_IF       "if"
#define   KW_LOAD     "load"
#define   KW_PRINT    "print"
#define   KW_ERROR    "error"

void  lenv_add_builtin    (lenv* e, char* name, lbuiltin func);

lval* builtin_global_def  (lenv* e, lval* a);  /*    def     */
lval* builtin_local_def   (lenv* e, lval* a);  /*    =       */
lval* builtin_head        (lenv* e, lval* a);  /*    head    */
lval* builtin_tail        (lenv* e, lval* a);  /*    tail    */
lval* builtin_list        (lenv* e, lval* a);  /*    list    */
lval* builtin_eval        (lenv* e, lval* a);  /*    eval    */
lval* builtin_join        (lenv* e, lval* a);  /*    join    */
lval* builtin_lambda      (lenv* e, lval* a);  /*    \       */
lval* builtin_add         (lenv* e, lval* a);  /*    +       */
lval* builtin_sub         (lenv* e, lval* a);  /*    -       */
lval* builtin_mul         (lenv* e, lval* a);  /*    *       */
lval* builtin_div         (lenv* e, lval* a);  /*    /       */
lval* builtin_gt          (lenv* e, lval* a);  /*    >       */
lval* builtin_gte         (lenv* e, lval* a);  /*    >=      */
lval* builtin_lt          (lenv* e, lval* a);  /*    <       */
lval* builtin_lte         (lenv* e, lval* a);  /*    <=      */
lval* builtin_eq          (lenv* e, lval* a);  /*    ==      */
lval* builtin_neq         (lenv* e, lval* a);  /*    !=      */
lval* builtin_if          (lenv* e, lval* a);  /*    if      */
lval* builtin_load        (lenv* e, lval* a);  /*    load    */
lval* builtin_print       (lenv* e, lval* a);  /*    print   */
lval* builtin_error       (lenv* e, lval* a);  /*    error   */

void  lenv_add_builtins   (lenv* e);

#endif//LISPY_BUILTINS_H

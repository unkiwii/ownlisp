#ifndef LISPY_VAL_H
#define LISPY_VAL_H

#include "env.h"
#include "builtins.h"

/**
 * TYPES of values in the language
 */
enum {  LVAL_ERR, LVAL_SYM,   LVAL_NUM,
        LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR,
        LVAL_STR };

char* ltype_name(int type);

/**
 * A value in the language, every construction, every string, number or function
 * is represented as an lval
 */
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

  /** value for type LVAL_STR **/
  char* str;

  /** value for type LVAL_FUN **/
  lbuiltin builtin;
  lenv* env;
  lval* formals;
  lval* body;

  /** values for type LVAL_SEXPR and LVAL_QEXPR **/
  int count;
  lval** cell;
};

/**
 * Creates a Number (long integer)
 *
 * long n     assigned to v->num
 *
 * return     an lval* of type LVAL_NUM
 */
lval* lval_num(long n);

/**
 * Creates an Error
 *
 * char* fmt, ...     just like printf(), assigned to v->err
 *
 * return     an lval* of type LVAL_ERR
 */
lval* lval_err(char* fmt, ...);

/**
 * Creates a Symbol
 *
 * A Symbol is a name for any variable or function
 *
 * char* s    assigned to v->sym
 *
 * return     an lval* of type LVAL_SYM
 */
lval* lval_sym(char* s);

/**
 * Creates a String
 *
 * char* s    assigned to v->str
 *
 * return     an lval* of type LVAL_STR
 */
lval* lval_str(char* s);

/**
 * Creates a builtin function
 *
 * Every builtin function is defined in builtins.h
 *
 * lbultin func   assigned to v->builtin
 *
 * return   an lval* of type LVAL_FUN, with v->formals
 *          and v->body set to NULL
 */
lval* lval_fun(lbuiltin func);

/**
 * Creates a function
 *
 * This is used to create user-defined functions and for many
 * operations in the standard library
 *
 * lval* formals    a list (LVAL_QEXPR) with all formal parameters
 * lval* body       a list (LVAL_QEXPR) of operations to evaluate
 *
 * return     an lval* of type LVAL_FUN, with v->builtin set to NULL
 */
lval* lval_lambda(lval* formals, lval* body);

/**
 * Creates an S-Expression
 *
 * An S-Expression or "Symbolic Expression" is a notation for nested list
 * data, invented and used in Lisp.
 *
 * return     an empty lval* of type LVAL_SEXPR, must use lval_add() to
 *            add childs to the returned list
 */
lval* lval_sexpr(void);

/**
 * Creates an Q-Expression
 *
 * An Q-Expression or "Quoted Expression" is a notation for a literal
 * S-Expression, this is used to treat code like data
 *
 * return     an empty lval* of type LVAL_QEXPR, must use lval_add() to
 *            add childs to the returned list
 */
lval* lval_qexpr(void);

/**
 * Creates a copy of an existing lval*
 *
 * lval* v    the lval* to be copied, can be of any type
 *
 * return     an exact copy with the same type and data
 */
lval* lval_copy(lval* v);

/**
 * Destroys an lval*, for list it destroy all its children first
 *
 * lval* v    the lval* to be destroyed, can be of any type
 */
void  lval_del(lval* v);

/**
 * Adds an lval* to a list (S or Q Expression)
 *
 * lval* v    the list
 * lval* x    the lval* to add to v
 *
 * return     the list with the new value added to it
 */
lval* lval_add(lval* v, lval* x);

/**
 * Removes an lval* from a list (S or Q Expression) at index i
 *
 * lval* v    the list
 * int i      the index at wich to remove the child
 *
 * return     the removed child from the list, the list is modified
 */
lval* lval_take(lval* v, int i);

/**
 * Removes an lval* from a list (S or Q Expression) at index i
 * and destroys the list
 *
 * lval* v    the list
 * int i      the index at wich to remove the child
 *
 * return     the removed child from the list, the list is destroyed
 */
lval* lval_pop(lval* v, int i);

/**
 * Add all children of y to x, both can be either S or Q Expressions
 *
 * lval* x    the list to append the children
 * lval* y    the list to remove the children
 *
 * return     x with all children of y appended to it, y is destroyed
 */
lval* lval_join(lval* x, lval* y);

/**
 * Print an list (S or Q Expression) to stdout
 *
 * lval* v      the list to print
 * char open    the char to show at the start of the output
 * char close   the char to show at the end of the output
 *
 * The result is showed as:
 *
 *    [open char] [all childs of v] [close char]
 *
 * Examples:
 *
 *    list: a b c, open: (, close: )      (a b c)
 *    list: 1 2 3, open: {, close: }      {1 2 3}
 */
void lval_print_expr(lval* v, char open, char close);

/**
 * Print an string to stdout
 *
 * lval* v        the string to pring
 * char* open     a string to show at the start of
 *                the output, can be NULL
 * char* close    a string to show at the end of the
 *                output, can be NULL
 *
 * The result is showed as:
 *
 *    [open str][string][close str]
 *
 * Examples:
 *
 *    string: hello, open: ', close: '        'hello'
 *    string: world, open: [", close: "]      ["world"]
 *    string: only, open: NULL, close: NULL   only
 */
void lval_print_str(lval* v, char* open, char* close);

/**
 * Print an lval* of any type to stdout
 *
 * for LVAL_SEXPR and LVAL_QEXPR this calls lval_print_expr()
 * for LVAL_STR this calls lval_print_str()
 * for any other type of lval* it just prints its contents
 *
 * lval* v      the lval* to print
 */
void lval_println(lval* v);

/**
 * Print an lval* of any type to stdout and add a newline
 * char to the end
 *
 * lval* v      the lval* to print
 */
void lval_print(lval* v);

/**
 * Test 2 lval* for equality, not equality of pointers but of values
 *
 * return     1 (true) if both lval* are equal, 0 (false) otherwise
 */
int lval_eq(lval* a, lval* b);

#endif//LISPY_VAL_H

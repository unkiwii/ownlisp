#ifndef LISPY_PARSER_H
#define LISPY_PARSER_H

#include "mpc.h"
#include "val.h"

struct lparser
{
  mpc_parser_t* Comment;
  mpc_parser_t* Lispy;    /* Expr*                            */
  mpc_parser_t* Expr;     /* String | Number | Sexpr | Qexpr  */
  mpc_parser_t* String;   /* LVAL_STR                         */
  mpc_parser_t* Number;   /* LVAL_NUM                         */
  mpc_parser_t* Sexpr;    /* LVAL_SEXPR                       */
  mpc_parser_t* Qexpr;    /* LVAL_QEXPR                       */
  mpc_parser_t* Symbol;   /* LVAL_SYM                         */
};


/**
 * Creates a new parser for the language
 *
 * return     a new parser for the language
 */
lparser* lparser_new();

/**
 * Deletes a parser created with lparser_new()
 *
 * lparser* p     the parser to destroy
 */
void lparser_del(lparser* p);

/**
 * Reads a Number from the syntax tree
 *
 * mpc_ast_t* t     the syntax tree
 *
 * return     an lval* of type LVAL_NUM
 */
lval* lparser_read_num(mpc_ast_t* t);

/**
 * Reads a String from the syntax tree
 *
 * mpc_ast_t* t     the syntax tree
 *
 * return     an lval* of type LVAL_STR
 */
lval* lparser_read_str(mpc_ast_t* t);

/**
 * Reads an lval from the syntax tree
 *
 * for Number this calls lparser_read_num(t)
 * for String this calls lparser_read_str(t)
 * for any other type it just creates the lval of that given type
 *
 * mpc_ast_t* t     the syntax tree
 *
 * return     an lval* of the read type
 */
lval* lparser_read(mpc_ast_t* t);

/**
 * Parse stream of chars from stdin and returns the
 * lval* of that read
 *
 * lparser* p     the parser to use
 * char* data     the data to parse
 *
 * return       and lval* with the result of the reading,
 *              this lval* must be evaluated and deleted
 *              later
 */
lval* lparser_parse_stdin(lparser* p, char* data);

/**
 * Parse stream of chars and returns the lval* of that read
 *
 * lparser* p     the parser to use
 * char* data     the data to parse
 *
 * return       and lval* with the result of the reading,
 *              this lval* must be evaluated and deleted
 *              later
 */
lval* lparser_parse(lparser* p, char* data);

#endif//LISPY_PARSER_H

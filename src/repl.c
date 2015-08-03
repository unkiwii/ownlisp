#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

#ifdef _WIN32
#include "prompt_win.h"
#else
#include <editline/readline.h>
#endif

#define VERSION "0.0.1"

int main (int argc, char** argv)
{
  /* Create some parsers */
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  /* Define them with the folowwing Language */
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
      number    : /-?[0-9]+/ ;                            \
      operator  : '+' | '-' | '*' | '/' ;                 \
      expr      : <number> | '(' <operator> <expr>+ ')' ; \
      lispy     : /^/ <operator> <expr>+ /$/ ;            \
    ",
    Number, Operator, Expr, Lispy);

  /* Start repl */
  puts("Lisp Version " VERSION);
  puts("Press Ctrl+c to Exit\n");

  while (1)
  {
    char* input = readline("lisp> ");
    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      mpc_ast_print(r.output);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  mpc_cleanup(4, Number, Operator, Expr, Lispy);

  return 0;
}

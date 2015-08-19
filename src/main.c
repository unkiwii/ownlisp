#include "env.h"
#include "eval.h"
#include "utils.h"
#include "val.h"

#ifdef _WIN32
#include "prompt_win.h"
#else
#include <editline/readline.h>
#endif

#define VERSION "0.0.1"

void cmd_exit()
{
  exit(0);
}

void cmd_help()
{
  printf("\n"
    "repl commands:\n"
    "\n"
    "  .help    prints this message\n"
    "  .env     prints environment\n"
    "  .exit    exits from repl\n"
    );
}

void cmd_env(lenv* e)
{
  puts("{");
  for (int i = 0; i < e->count; i++) {
    printf("  %s: ", e->syms[i]);
    lval_println(e->vals[i]);
  }
  puts("}");
}

int main(int argc, char** argv)
{
  lenv* env = lenv_new();
  lenv_add_builtins(env);

  if (argc >= 2) {
    /* read every file passed and load it */
    for (int i = 1; i < argc; i++) {
      lval* f = lval_add(lval_sexpr(), lval_str(argv[i]));
      lval* x = builtin_load(env, f);
      if (x->type == LVAL_ERR) {
        lval_println(x);
      }
      lval_del(x);
    }
  } else {
    /* Start repl */
    puts("Lisp Version " VERSION);
    puts("type .help if you want to know more");

    while (1)
    {
      char* input = readline("\nlisp> ");
      add_history(input);

      if      (is(input, ".exit")) { cmd_exit();          }
      else if (is(input, ".help")) { cmd_help();          }
      else if (is(input, ".env"))  { cmd_env(env); }
      else {
        lval* r = lparser_parse_stdin(env->parser, input);
        if (r) {
          lval* x = leval(env, r);
          lval_println(x);
          lval_del(x);
        }
      }

      free(input);
    }
  }

  lenv_del(env);

  return 0;
}

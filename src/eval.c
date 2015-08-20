#include "eval.h"
#include "builtins.h"
#include "utils.h"

lval* leval_sexpr(lenv* e, lval* v)
{
  int isdef  = (v->count > 0) && (v->cell[0]->type == LVAL_SYM)
            && (is(v->cell[0]->sym, KW_GDEF) || is(v->cell[0]->sym, KW_LDEF));

  /* evaluate all children of expression, if any of those is an error, return that */
  for (int i = 0; i < v->count; i++) {
    /**
     * for definitions (def and :=) do not evaluate the second child if
     * that child is just a symbol
     */
    if (isdef && i == 1 && v->cell[1]->type == LVAL_SYM) {
      continue;
    }
    v->cell[i] = leval(e, v->cell[i]);
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

  lval* result = lcall(e, f, v);
  lval_del(f);
  return result;
}

lval* leval(lenv* e, lval* v)
{
  if (v->type == LVAL_SYM) {
    lval* x = lenv_get(e, v);
    lval_del(v);
    return x;
  }

  if (v->type == LVAL_SEXPR) {
    return leval_sexpr(e, v);
  }

  return v;
}

lval* lcall(lenv* e, lval* f, lval* a)
{
  /* if is a builtin, call it directly */
  if (f->builtin) {
    return f->builtin(e, a);
  }

  int args_given = a->count;
  int args_total = f->formals->count;

  while (a->count) {
    if (f->formals->count == 0) {
      lval_del(a);
      return lval_err(
          "function passed too many arguments. got %i, expected %i", args_given, args_total);
    }

    /* pop the next symbol from the formal parameters */
    lval* sym = lval_pop(f->formals, 0);

    /* if function parameters are {x : xs} */
    if (is(sym->sym, KW_VARG)) {
      if (f->formals->count != 1) {
        lval_del(a);
        return lval_err(
            "function format invalid. symbol '%s' not followed by a syngle symbol.", KW_VARG);
      }

      /* next formal should be bound to remaining arguments */
      lval* nsym = lval_pop(f->formals, 0);
      lenv_put(f->env, nsym, BTNAME(LIST)(e, a));
      lval_del(sym);
      lval_del(nsym);
      break;

    /* if function parameters are {a b c ...} */
    } else {
      lval* val = lval_pop(a, 0);
      lenv_put(f->env, sym, val);
      lval_del(sym);
      lval_del(val);
    }
  }

  lval_del(a);

  /* if ':' remains in formal list bind to empty list */
  if (f->formals->count > 0 && is(f->formals->cell[0]->sym, KW_VARG)) {
    if (f->formals->count != 2) {
      return lval_err(
          "function format invalid. symbol ':' not followed by single symbol.");
    }

    /* pop and delete the ':' symbol */
    lval_del(lval_pop(f->formals, 0));

    /* pop next symbol, create empty list and bind them */
    lval* sym = lval_pop(f->formals, 0);
    lval* val = lval_qexpr();
    lenv_put(f->env, sym, val);
    lval_del(sym);
    lval_del(val);
  }

  if (f->formals->count == 0) {
    /* if all formals have been bound evaluate the function */
    f->env->parent = e;
    return BTNAME(EVAL)(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
  }

  /* if there are more parameters to be bound, return a copy */
  return lval_copy(f);
}

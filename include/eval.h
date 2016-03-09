#ifndef EVAL_H
#define EVAL_H

#include "mpc.h"
#include "lval.h"

#define LASSERT(arg, cond, err) \
   if(cond) {                \
      lval_del(arg);            \
      return lval_err(err);     \
   }

lval *lval_eval_sexpr(lval *v);
lval *lval_eval(lval *v);
lval *lval_pop(lval *v, int iChild);
lval *lval_take(lval *v, int iChild);

lval *builtin_op(lval *a, char *op);

lval *builtin(lval *arg, char *func);
lval *builtin_list(lval *arg);
lval *builtin_head(lval *arg);
lval *builtin_tail(lval *arg);
lval *builtin_join(lval *arg);
lval *lval_join(lval *x, lval *y);
lval *builtin_eval(lval *arg);

#endif

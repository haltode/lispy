#ifndef EVAL_H
#define EVAL_H

#include "mpc.h"
#include "lval.h"

#define LASSERT(arg, cond, fmt, ...)            \
   if(cond) {                                   \
      lval *err = lval_err(fmt, ##__VA_ARGS__); \
      lval_del(arg);                            \
      return err;                               \
   }

lval *lval_eval_sexpr(lenv *env, lval *val);
lval *lval_eval(lenv *env, lval *val);

lval *builtin_op(lenv *env, lval *arg, char *op);
lval *builtin_eval(lenv *env, lval *arg);

lval *builtin_def(lenv *env, lval *arg);

lval *builtin_add(lenv *env, lval *arg);
lval *builtin_sub(lenv *env, lval *arg);
lval *builtin_mul(lenv *env, lval *arg);
lval *builtin_div(lenv *env, lval *arg);
lval *builtin_mod(lenv *env, lval *arg);
lval *builtin_pow(lenv *env, lval *arg);
lval *builtin_min(lenv *env, lval *arg);
lval *builtin_max(lenv *env, lval *arg);

lval *builtin_list(lenv *env, lval *arg);
lval *builtin_head(lenv *env, lval *arg);
lval *builtin_tail(lenv *env, lval *arg);
lval *builtin_join(lenv *env, lval *arg);

#endif

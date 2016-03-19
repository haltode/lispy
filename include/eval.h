#ifndef EVAL_H
#define EVAL_H

#include "mpc.h"
#include "lval.h"

#define LASSERT(arg, cond, fmt, ...)                            \
   if(cond) {                                                   \
      lval *err = lval_err(fmt, ##__VA_ARGS__);                 \
      lval_del(arg);                                            \
      return err;                                               \
   }

#define LASSERT_ARG(fun, arg, num)                              \
   LASSERT(arg, arg->count != num,                              \
      "Function '%s' passed incorrect number of arguments : "   \
      "got %i (expected %i)", fun, arg->count, num)

#define LASSERT_TYPE(fun, arg, index, expected)                 \
   LASSERT(arg, arg->cell[index]->type != expected,             \
      "Function '%s' passed incorrect type for argument %i : "  \
      "got %s (expected '%s')", fun, index,                     \
      ltype_name(arg->cell[index]->type), ltype_name(expected))

lval *lval_eval_sexpr(lenv *env, lval *val);
lval *lval_eval(lenv *env, lval *val);

lval *builtin_op(lenv *env, lval *arg, char *op);
lval *builtin_eval(lenv *env, lval *arg);

lval *builtin_def(lenv *env, lval *arg);
lval *builtin_lambda(lenv *env, lval *arg);
lval *builtin_put(lenv *env, lval *arg);
lval *builtin_var(lenv *env, lval *arg, char *func);

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

lval *builtin_if(lenv *env, lval *arg);
lval *builtin_ord(lenv *env, lval *arg, char *op);
lval *builtin_cmp(lenv *env, lval *arg, char *op);
lval *builtin_gt(lenv *env, lval *arg);
lval *builtin_lt(lenv *env, lval *arg);
lval *builtin_ge(lenv *env, lval *arg);
lval *builtin_le(lenv *env, lval *arg);
lval *builtin_eq(lenv *env, lval *arg);
lval *builtin_ne(lenv *env, lval *arg);

lval *builtin_load(lenv *env, lval *arg);
lval *builtin_print(lenv *env, lval *arg);
lval *builtin_error(lenv *env, lval *arg);

#endif

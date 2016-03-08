#ifndef EVAL_H
#define EVAL_H

#include "mpc.h"
#include "lval.h"

lval *lval_eval_sexpr(lval *v);
lval *lval_eval(lval *v);
lval *lval_pop(lval *v, int iChild);
lval *lval_take(lval *v, int iChild);
lval *builtin_op(lval *a, char *op);

#endif

#ifndef EVAL_H
#define EVAL_H

#include "mpc.h"
#include "lval.h"

lval eval_op(char *op, lval x, lval y);
lval eval(mpc_ast_t *token);

#endif

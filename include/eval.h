#ifndef EVAL_H
#define EVAL_H

#include "mpc.h"

long eval_op(char *op, long x, long y);
long eval(mpc_ast_t *t);

#endif

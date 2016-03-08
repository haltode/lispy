#ifndef LVAL_H
#define LVAL_H

#include "mpc.h"

// Lispy Value
typedef struct lval lval;
struct lval {
   int type;
   long num;
   char *sym;

   char *err;

   lval **cell;
   int count;
};

// Possible lval types
enum { LVAL_NUM, LVAL_SYM, LVAL_SEXPR, LVAL_ERR };

// Possible error types
enum { LERR_DIV_ZERO, LERR_MOD_ZERO, LERR_BAD_OP, LERR_BAD_NUM };


lval *lval_num(long num);
lval *lval_sym(char *sym);
lval *lval_sexpr(void);
lval *lval_err(char *err);

void lval_del(lval *v);

lval *lval_read_num(mpc_ast_t *t);
lval *lval_read(mpc_ast_t *t);
lval *lval_add(lval *v, lval *x);

void lval_expr_print(lval *v, char open, char close);
void lval_print(lval *v);
void lval_println(lval *v);

#endif

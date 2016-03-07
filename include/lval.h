#ifndef LVAL_H
#define LVAL_H

// Lispy Value
typedef struct lval lval;
struct lval {
   int type;
   long num;
   int err;
};

// Possible lval types
enum { LVAL_NUM, LVAL_ERR };

// Possible error types
enum { LERR_DIV_ZERO, LERR_MOD_ZERO, LERR_BAD_OP, LERR_BAD_NUM };


lval lval_num(long num);
lval lval_err(int err);
void lval_print(lval v);
void lval_println(lval v);

#endif

#ifndef LVAL_H
#define LVAL_H

#include "mpc.h"

#define ERROR_BUFFER_SIZE 512

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

typedef lval*(*lbuiltin)(lenv*, lval*);

// Lispy Value
struct lval {
   int type;

   // Basic
   long num;
   char *str;
   char *sym;
   char *err;

   // Function
   lbuiltin fun;
   lenv *env;
   lval *formal;
   lval *body;

   // Expression
   lval **cell;
   int count;
};

// Environment
struct lenv {
   lenv *parent;

   int count;
   char **sym;
   lval **val;
};

// Possible lval types
enum {   LVAL_NUM, LVAL_STR, LVAL_SYM, LVAL_FUN, 
         LVAL_SEXPR, LVAL_QEXPR, 
         LVAL_ERR };


/* ---------- lval ---------- */
lval *lval_num(long num);
lval *lval_str(char *str);
lval *lval_sym(char *sym);
lval *lval_fun(lbuiltin func);
lval *lval_lambda(lval *formal, lval *body);
lval *lval_sexpr(void);
lval *lval_qexpr(void);
lval *lval_err(char *fmt, ...);

void lval_del(lval *val);

lval *lval_read_num(mpc_ast_t *token);
lval *lval_read_str(mpc_ast_t *token);
lval *lval_read(mpc_ast_t *token);
lval *lval_add(lval *val, lval *x);

void lval_print_expr(lval *val, char open, char close);
void lval_print_str(lval *val);
void lval_print(lval *val);
void lval_println(lval *val);

lval *lval_pop(lval *val, int iChild);
lval *lval_take(lval *val, int iChild);
lval *lval_join(lval *x, lval *y);
lval *lval_copy(lval *val);
lval *lval_call(lenv *env, lval *fun, lval *arg);
int lval_eq(lval *x, lval *y);
char *ltype_name(int t);

/* ---------- lenv ---------- */
lenv *lenv_new(void);
void lenv_del(lenv *env);
lval *lenv_get(lenv *env, lval *var);
void lenv_put(lenv *env, lval *var, lval *val);
void lenv_def(lenv *env, lval *var, lval *val);
lenv *lenv_copy(lenv *env);
void lenv_add_builtin(lenv *env, char *name, lbuiltin func);
void lenv_add_builtins(lenv *env);

#endif

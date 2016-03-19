#include <string.h>
#include <math.h>

#include "eval.h"
#include "lval.h"
#include "parser.h"

mpc_parser_t *Lispy;


/* ---------- lval eval ---------- */

lval *lval_eval_sexpr(lenv *env, lval *val)
{
   int iChild;

   // Evaluate all the children
   for(iChild = 0; iChild < val->count; ++iChild)
      val->cell[iChild] = lval_eval(env, val->cell[iChild]);

   // Check for errors
   for(iChild = 0; iChild < val->count; ++iChild)
      if(val->cell[iChild]->type == LVAL_ERR)
         return lval_take(val, iChild);

   // Empty expression
   if(val->count == 0)
      return val;

   // Single expression
   if(val->count == 1)
      return lval_take(val, 0);

   // Make sure the first element is a function
   lval *first = lval_pop(val, 0);
   if(first->type != LVAL_FUN) {
      lval *error = lval_err("S-Expression starts with incorrect type : "
                              "'%s' (expected 'Function')", 
                              ltype_name(first->type));
      lval_del(first);
      lval_del(val);
      return error;
   }

   lval *res = lval_call(env, first, val);
   lval_del(first);

   return res;
}

lval *lval_eval(lenv *env, lval *val)
{
   // Get the value
   if(val->type == LVAL_SYM) {
      lval *x = lenv_get(env, val);
      lval_del(val);
      return x;
   }
   // Take down the S-expression into smaller pieces
   if(val->type == LVAL_SEXPR)
      return lval_eval_sexpr(env, val);

   // The rest is already fine
   return val;
}

/* ---------- Builtin operators/evaluation ---------- */

lval *builtin_op(lenv *env, lval *arg, char *op)
{
   // Check if all arguments are numbers
   int iChild;
   for(iChild = 0; iChild < arg->count; ++iChild) {
      if(arg->cell[iChild]->type != LVAL_NUM) {
         lval *error = lval_err("Cannot operate on non-number, passed : '%s'",
                                 ltype_name(arg->cell[iChild]->type));
         lval_del(arg);
         return error;
      }
   }

   // Get the first element
   lval *x = lval_pop(arg, 0);

   // Special case: no argument and just the sub operation
   if(arg->count == 0 && !strcmp(op, "-"))
      x->num = -x->num;

   while(arg->count > 0) {
      // Get the next element
      lval *y = lval_pop(arg, 0);

      if(!strcmp(op, "+")) x->num += y->num;
      if(!strcmp(op, "-")) x->num -= y->num;
      if(!strcmp(op, "*")) x->num *= y->num;
      if(!strcmp(op, "/")) {
         if(y->num == 0) {
            lval_del(x);
            lval_del(y);
            x = lval_err("Division by zero");
            break;
         }
         x->num /= y->num;
      }
      if(!strcmp(op, "%")) {
         if(y->num == 0) {
            lval_del(x);
            lval_del(y);
            x = lval_err("Modulo by zero");
            break;
         }
         x->num %= y->num;
      }
      if(!strcmp(op, "^")) x->num = pow(x->num, y->num);
      if(!strcmp(op, "min")) x->num = x->num < y->num ? x->num : y->num;
      if(!strcmp(op, "max")) x->num = x->num > y->num ? x->num : y->num;

      lval_del(y);
   }

   lval_del(arg);

   return x;
}

lval *builtin_eval(lenv *env, lval *arg)
{
   LASSERT_ARG("eval", arg, 1);
   LASSERT_TYPE("eval", arg, 0, LVAL_QEXPR);

   lval *x = lval_take(arg, 0);
   x->type = LVAL_SEXPR;
   return lval_eval(env, x);
}

/* ---------- Function operators ---------- */

lval *builtin_def(lenv *env, lval *arg)
{
   return builtin_var(env, arg, "def");
}

lval *builtin_lambda(lenv *env, lval *arg)
{
   LASSERT_ARG("lambda", arg, 2);
   LASSERT_TYPE("lambda", arg, 0, LVAL_QEXPR);
   LASSERT_TYPE("lambda", arg, 1, LVAL_QEXPR);
   
   // Check if the first Q-Expression contains only symbols
   int iCell;
   for(iCell = 0; iCell < arg->cell[0]->count; ++iCell) {
      LASSERT(arg, (arg->cell[0]->cell[iCell]->type != LVAL_SYM),
         "Cannot define non-symbol, got '%s' (expected 'Symbol')",
         ltype_name(arg->cell[0]->cell[iCell]->type));
   }

   // Get the component of the lambda function and return it
   lval *formal = lval_pop(arg, 0);
   lval *body   = lval_pop(arg, 0);
   lval_del(arg);

   return lval_lambda(formal, body);
}

lval *builtin_put(lenv *env, lval *arg)
{
   return builtin_var(env, arg, "=");
}

lval *builtin_var(lenv *env, lval *arg, char *func)
{
   LASSERT_TYPE(func, arg, 0, LVAL_QEXPR);
   
   int iSym;
   // First argument is symbol list
   lval *sym = arg->cell[0];
   for(iSym = 0; iSym < sym->count; ++iSym)
      LASSERT(arg, sym->cell[iSym]->type != LVAL_SYM,
         "Function '%s' cannot define non-symbol : "
         "'%s' (expected 'Symbol')", func, ltype_name(sym->cell[iSym]->type));

   LASSERT(arg, (sym->count != arg->count - 1),
      "Function '%s' passed too many arguments for symbols : "
      "got %i (expected %i)", func, sym->count, arg->count - 1);

   for(iSym = 0; iSym < sym->count; ++iSym) {
      // Define globally
      if(!strcmp(func, "def"))
         lenv_def(env, sym->cell[iSym], arg->cell[iSym + 1]);
      // Define locally
      if(!strcmp(func, "="))
         lenv_put(env, sym->cell[iSym], arg->cell[iSym + 1]);
   }

   lval_del(arg);
   return lval_sexpr();
}

/* ---------- Math operators ---------- */

lval *builtin_add(lenv *env, lval *arg)
{
   return builtin_op(env, arg, "+");
}

lval *builtin_sub(lenv *env, lval *arg)
{
   return builtin_op(env, arg, "-");
}

lval *builtin_mul(lenv *env, lval *arg)
{
   return builtin_op(env, arg, "*");
}

lval *builtin_div(lenv *env, lval *arg)
{
   return builtin_op(env, arg, "/");
}

lval *builtin_mod(lenv *env, lval *arg)
{
   return builtin_op(env, arg, "%");
}

lval *builtin_pow(lenv *env, lval *arg)
{
   return builtin_op(env, arg, "^");
}

lval *builtin_min(lenv *env, lval *arg)
{
   return builtin_op(env, arg, "min");
}

lval *builtin_max(lenv *env, lval *arg)
{
   return builtin_op(env, arg, "max");
}

/* ---------- List operators ---------- */

lval *builtin_list(lenv *env, lval *arg)
{
   arg->type = LVAL_QEXPR;
   return arg;
}

lval *builtin_head(lenv *env, lval *arg)
{
   LASSERT_ARG("head", arg, 1);
   LASSERT_TYPE("head", arg, 0, LVAL_QEXPR);
   LASSERT(arg, arg->cell[0]->count == 0, "Function 'head' passed {}");

   // Take the first argument and delete all the elements that are not the head
   lval *first = lval_take(arg, 0);
   while(first->count > 1)
      lval_del(lval_pop(first, 1));
   return first;
}

lval *builtin_tail(lenv *env, lval *arg)
{
   LASSERT_ARG("tail", arg, 1);
   LASSERT_TYPE("tail", arg, 0, LVAL_QEXPR);
   LASSERT(arg, arg->cell[0]->count == 0, "Function 'tail' passed {}");

   // Take the first argument and delete every elements until it reach the tail
   lval *first = lval_take(arg, 0);
   lval_del(lval_pop(first, 0));
   return first;
}

lval *builtin_join(lenv *env, lval *arg)
{
   int iCell;
   for(iCell = 0; iCell < arg->count; ++iCell)
      LASSERT_TYPE("join", arg, iCell, LVAL_QEXPR);
   
   lval *x = lval_pop(arg, 0);
   while(arg->count) {
      lval *y = lval_pop(arg, 0);
      x = lval_join(x, y); 
   }

   lval_del(arg);
   return x;
}

/* ---------- Conditional operators ---------- */

lval *builtin_if(lenv *env, lval *arg)
{
   LASSERT_ARG("if", arg, 3);
   LASSERT_TYPE("if", arg, 0, LVAL_NUM);
   LASSERT_TYPE("if", arg, 1, LVAL_QEXPR);
   LASSERT_TYPE("if", arg, 2, LVAL_QEXPR);

   lval *x;

   // Mark the expressions as evaluable
   arg->cell[1]->type = LVAL_SEXPR;
   arg->cell[2]->type = LVAL_SEXPR;

   if(arg->cell[0]->num)
      x = lval_eval(env, lval_pop(arg, 1));
   else
      x = lval_eval(env, lval_pop(arg, 2));

   lval_del(arg);
   return x;
}

lval *builtin_ord(lenv *env, lval *arg, char *op)
{
   LASSERT_ARG(op, arg, 2);
   LASSERT_TYPE(op, arg, 0, LVAL_NUM);
   LASSERT_TYPE(op, arg, 1, LVAL_NUM);
   
   int res;
   if(!strcmp(op, ">"))
      res = (arg->cell[0]->num > arg->cell[1]->num);
   if(!strcmp(op, "<"))
      res = (arg->cell[0]->num < arg->cell[1]->num);
   if(!strcmp(op, ">="))
      res = (arg->cell[0]->num >= arg->cell[1]->num);
   if(!strcmp(op, "<="))
      res = (arg->cell[0]->num <= arg->cell[1]->num);

   lval_del(arg);
   return lval_num(res);
}

lval *builtin_cmp(lenv *env, lval *arg, char *op)
{
   LASSERT_ARG(op, arg, 2);

   int res;
   if(!strcmp(op, "=="))
      res = lval_eq(arg->cell[0], arg->cell[1]);
   if(!strcmp(op, "!="))
      res = !lval_eq(arg->cell[0], arg->cell[1]);

   lval_del(arg);
   return lval_num(res);
}

lval *builtin_gt(lenv *env, lval *arg)
{
   return builtin_ord(env, arg, ">");
}

lval *builtin_lt(lenv *env, lval *arg)
{
   return builtin_ord(env, arg, "<");
}

lval *builtin_ge(lenv *env, lval *arg)
{
   return builtin_ord(env, arg, ">=");
}

lval *builtin_le(lenv *env, lval *arg)
{
   return builtin_ord(env, arg, "<=");
}

lval *builtin_eq(lenv *env, lval *arg)
{
   return builtin_cmp(env, arg, "==");
}

lval *builtin_ne(lenv *env, lval *arg)
{
   return builtin_cmp(env, arg, "!=");
}

/* ---------- String operators ---------- */

lval *builtin_load(lenv *env, lval *arg)
{
   LASSERT_ARG("load", arg, 1);
   LASSERT_TYPE("load", arg, 0, LVAL_STR);

   // Parse file given by string name
   mpc_result_t res;
   if(mpc_parse_contents(arg->cell[0]->str, Lispy, &res)) {
      lval *expr = lval_read(res.output);
      mpc_ast_delete(res.output);

      while(expr->count) {
         lval *x = lval_eval(env, lval_pop(expr, 0));
         if(x->type == LVAL_ERR)
            lval_println(x);
         lval_del(x);
      }

      lval_del(expr);
      lval_del(arg);

      return lval_sexpr();
   }
   else {
      char *err_msg = mpc_err_string(res.error);
      mpc_err_delete(res.error);

      lval *err = lval_err("Could not load library '%s'", err_msg);
      free(err_msg);
      lval_del(arg);

      return err;
   }
}

lval *builtin_print(lenv *env, lval *arg)
{
   int iCell;
   for(iCell = 0; iCell < arg->count; ++iCell) {
      lval_print(arg->cell[iCell]);
      putchar(' ');
   }

   putchar('\n');
   lval_del(arg);

   return lval_sexpr();
}

lval *builtin_error(lenv *env, lval *arg)
{
   LASSERT_ARG("error", arg, 1);
   LASSERT_TYPE("error", arg, 0, LVAL_STR);

   lval *err = lval_err(arg->cell[0]->str);
   lval_del(arg);
   return err;
}

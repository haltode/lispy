#include <string.h>
#include <math.h>

#include "eval.h"
#include "lval.h"

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

   lval *res = first->fun(env, val);
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
   LASSERT(arg, arg->count != 1,
         "Function 'eval' passed too many arguments, "
         "got %i and expected %i", arg->count, 1);
   LASSERT(arg, arg->cell[0]->type != LVAL_QEXPR,
         "Function 'eval' passed incorrect type "
         ": '%s' (expected 'Q-Expression')", 
         ltype_name(arg->cell[0]->type))

   lval *x = lval_take(arg, 0);
   x->type = LVAL_SEXPR;
   return lval_eval(env, x);
}

/* ---------- Function operators ---------- */

lval *builtin_def(lenv *env, lval *arg)
{
   LASSERT(arg, arg->cell[0]->type != LVAL_QEXPR,
      "Function 'def' passed incorrect type "
      ": '%s' (expected 'Q-Expression')",
      ltype_name(arg->cell[0]->type));

   int iSym;
   // First argument is symbol list
   lval *sym = arg->cell[0];
   for(iSym = 0; iSym < sym->count; ++iSym)
      LASSERT(arg, sym->cell[iSym]->type != LVAL_SYM,
         "Function 'def' cannot define non-symbol : "
         "'%s' (expected 'Symbol')", ltype_name(sym->cell[iSym]->type));

   LASSERT(arg, (sym->count != arg->count - 1),
      "Function 'def' passed too many arguments for symbols : "
      "got %i (expected %i)", sym->count, arg->count - 1);

   for(iSym = 0; iSym < sym->count; ++iSym)
      lenv_put(env, sym->cell[iSym], arg->cell[iSym + 1]);

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
   LASSERT(arg, arg->count != 1,
         "Function 'head' passed too many arguments : "
         "got %i (expected 1)", arg->count);
   LASSERT(arg, arg->cell[0]->type != LVAL_QEXPR,
         "Function 'head' passed incorrect type for argument 0 : "
         "'%s' (expected 'Q-Expression')",
         ltype_name(arg->cell[0]->type));
   LASSERT(arg, arg->cell[0]->count == 0,
         "Function 'head' passed {}");

   // Take the first argument and delete all the elements that are not the head
   lval *first = lval_take(arg, 0);
   while(first->count > 1)
      lval_del(lval_pop(first, 1));
   return first;
}

lval *builtin_tail(lenv *env, lval *arg)
{
   LASSERT(arg, arg->count != 1,
      "Function 'tail' passed too many arguments : "
      "got %i (expected 1)", arg->count);
   LASSERT(arg, arg->cell[0]->type != LVAL_QEXPR,
      "Function 'tail' passed incorrect type for argument 0 : "
      "'%s' (expected 'Q-Expression')",
      ltype_name(arg->cell[0]->type));
   LASSERT(arg, arg->cell[0]->count == 0,
      "Function 'tail' passed {}");

   // Take the first argument and delete every elements until it reach the tail
   lval *first = lval_take(arg, 0);
   lval_del(lval_pop(first, 0));
   return first;
}

lval *builtin_join(lenv *env, lval *arg)
{
   int iCell;
   for(iCell = 0; iCell < arg->count; ++iCell)
      LASSERT(arg, arg->cell[iCell]->type != LVAL_QEXPR,
            "Function 'join' passed incorrect type : "
            "'%s' (expected 'Q-Expression')",
            ltype_name(arg->cell[iCell]->type));
   
   lval *x = lval_pop(arg, 0);
   while(arg->count) {
      lval *y = lval_pop(arg, 0);
      x = lval_join(x, y); 
   }

   lval_del(arg);
   return x;
}

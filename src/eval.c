#include <string.h>
#include <math.h>

#include "eval.h"

lval *lval_eval_sexpr(lval *v)
{
   int iChild;

   // Evaluate all the children
   for(iChild = 0; iChild < v->count; ++iChild)
      v->cell[iChild] = lval_eval(v->cell[iChild]);

   // Check for errors
   for(iChild = 0; iChild < v->count; ++iChild)
      if(v->cell[iChild]->type == LVAL_ERR)
         return lval_take(v, iChild);

   // Empty expression
   if(v->count == 0)
      return v;

   // Single expression
   if(v->count == 1)
      return lval_take(v, 0);

   // Make sure the first element is a symbol
   lval *first = lval_pop(v, 0);
   if(first->type != LVAL_SYM) {
      lval_del(first);
      lval_del(v);
      return lval_err("S-expression does not start with symbol");
   }

   lval *res = builtin(v, first->sym);
   lval_del(first);

   return res;
}

lval *lval_eval(lval *v)
{
   // Take down the S-expression into smaller pieces
   if(v->type == LVAL_SEXPR)
      return lval_eval_sexpr(v);

   // The rest is already fine
   return v;
}

// Extract an element (target) from an S-expression and shift the rest of the 
// list backward
lval *lval_pop(lval *v, int iChild)
{
   lval *target = v->cell[iChild];

   // Make the shift
   memmove( &v->cell[iChild], &v->cell[iChild + 1],
            sizeof(lval*) * (v->count - iChild - 1));

   // Update the list
   --(v->count);
   v->cell = realloc(v->cell, sizeof(lval*) * v->count);

   return target;
}

// Extract an element (target) and delete the rest of the list
lval *lval_take(lval *v, int iChild)
{
   lval *target = lval_pop(v, iChild);
   lval_del(v);
   return target;
}

lval *builtin_op(lval *arg, char *op)
{
   // Check if all arguments are numbers
   int iChild;
   for(iChild = 0; iChild < arg->count; ++iChild) {
      if(arg->cell[iChild]->type != LVAL_NUM) {
         lval_del(arg);
         return lval_err("Cannot operate on non-number");
      }
   }

   // Get the first element
   lval *x = lval_pop(arg, 0);

   // Special case : no argument and just the sub operation
   if(arg->count == 0 &&
      (!strcmp(op, "-") || !strcmp(op, "sub")))
      x->num = -x->num;

   while(arg->count > 0) {
      // Get the next element
      lval *y = lval_pop(arg, 0);

      if(!strcmp(op, "+") || !strcmp(op, "add")) x->num += y->num;
      if(!strcmp(op, "-") || !strcmp(op, "sub")) x->num -= y->num;
      if(!strcmp(op, "*") || !strcmp(op, "mul")) x->num *= y->num;
      if(!strcmp(op, "/") || !strcmp(op, "div")) {
         if(y->num == 0) {
            lval_del(x);
            lval_del(y);
            x = lval_err("Division by zero");
            break;
         }
         x->num /= y->num;
      }
      if(!strcmp(op, "%") || !strcmp(op, "mod")) {
         if(y->num == 0) {
            lval_del(x);
            lval_del(y);
            x = lval_err("Modulo by zero");
            break;
         }
         x->num %= y->num;
      }
      if(!strcmp(op, "^") || !strcmp(op, "pow")) x->num = pow(x->num, y->num);
      if(!strcmp(op, "min")) x->num = x->num < y->num ? x->num : y->num;
      if(!strcmp(op, "max")) x->num = x->num > y->num ? x->num : y->num;

      lval_del(y);
   }

   lval_del(arg);

   return x;
}

lval *builtin(lval *arg, char *func)
{
   if(!strcmp("list", func)) return builtin_list(arg);
   if(!strcmp("head", func)) return builtin_head(arg);
   if(!strcmp("tail", func)) return builtin_tail(arg);
   if(!strcmp("join", func)) return builtin_join(arg);
   if(!strcmp("eval", func)) return builtin_eval(arg);
   if(strstr("+-*/%", func)) return builtin_op(arg, func);

   lval_del(arg);
   return lval_err("Unknown function");
}

lval *builtin_list(lval *arg)
{
   arg->type = LVAL_QEXPR;
   return arg;
}

lval *builtin_head(lval *arg)
{
   LASSERT(arg, arg->count != 1,
         "Function 'head' passed too many arguments");
   LASSERT(arg, arg->cell[0]->type != LVAL_QEXPR,
         "Function 'head' passed incorrect type");
   LASSERT(arg, arg->cell[0]->count == 0,
         "Function 'head' passed {}");

   // Take the first argument and delete all the elements that are not the head
   lval *first = lval_take(arg, 0);
   while(first->count > 1)
      lval_del(lval_pop(first, 1));
   return first;
}

lval *builtin_tail(lval *arg)
{
   LASSERT(arg, arg->count != 1,
      "Function 'tail' passed too many arguments");
   LASSERT(arg, arg->cell[0]->type != LVAL_QEXPR,
      "Function 'tail' passed incorrect type");
   LASSERT(arg, arg->cell[0]->count == 0,
      "Function 'tail' passed {}");

   // Take the first argument and delete every elements until it reach the tail
   lval *first = lval_take(arg, 0);
   lval_del(lval_pop(first, 0));
   return first;
}

lval *builtin_join(lval *arg)
{
   int iCell;
   for(iCell = 0; iCell < arg->count; ++iCell)
      LASSERT(arg, arg->cell[iCell]->type != LVAL_QEXPR,
            "Function 'join' passed incorrect type");
   
   lval *x = lval_pop(arg, 0);
   while(arg->count)
      x = lval_join(x, lval_pop(arg, 0)); 

   lval_del(arg);
   return x;
}

lval *lval_join(lval *x, lval *y)
{
   // Add each y's cell to x
   while(y->count)
      x = lval_add(x, lval_pop(y, 0));

   lval_del(y);
   return x;
}

lval *builtin_eval(lval *arg)
{
   LASSERT(arg, arg->count != 1,
         "Function 'eval' passed too many arguments");
   LASSERT(arg, arg->cell[0]->type != LVAL_QEXPR,
         "Function 'eval' passed incorrect type")

   lval *x = lval_take(arg, 0);
   x->type = LVAL_SEXPR;
   return lval_eval(x);
}

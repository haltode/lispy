#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lval.h"
#include "eval.h"

/* ---------- Constructors & Destructors ---------- */

lval *lval_num(long num)
{
   lval *val = malloc(sizeof(lval));
   val->type = LVAL_NUM;
   val->num  = num;
   return val;
}

lval *lval_sym(char *sym)
{
   lval *val = malloc(sizeof(lval));
   val->type = LVAL_SYM;
   val->sym  = malloc(strlen(sym) + 1);
   strcpy(val->sym, sym);
   return val;
}

lval *lval_fun(lbuiltin func)
{
   lval *val = malloc(sizeof(lval));
   val->type = LVAL_FUN;
   val->fun  = func;
   return val;
}

lval *lval_lambda(lval *formal, lval *body)
{
   lval *val   = malloc(sizeof(lval));
   val->type   = LVAL_FUN;
   val->fun    = NULL;
   val->env    = lenv_new();
   val->formal = formal;
   val->body   = body;
   return val;
}

lval *lval_sexpr(void)
{
   lval *val  = malloc(sizeof(lval));
   val->type  = LVAL_SEXPR;
   val->cell  = NULL;
   val->count = 0;
   return val;
}

lval *lval_qexpr(void)
{
   lval *val  = malloc(sizeof(lval));
   val->type  = LVAL_QEXPR;
   val->cell  = NULL;
   val->count = 0;
   return val;
}

lval *lval_err(char *fmt, ...)
{
   lval *val = malloc(sizeof(lval));
   val->type = LVAL_ERR;

   va_list va;
   va_start(va, fmt);

   val->err = malloc(ERROR_BUFFER_SIZE);
   vsnprintf(val->err, ERROR_BUFFER_SIZE - 1, fmt, va);
   val->err = realloc(val->err, strlen(val->err) + 1);

   va_end(va);

   return val;
}

void lval_del(lval *val)
{
   // Free everything that the lval is pointing to...
   switch(val->type) {
      case LVAL_NUM:
         break;
      case LVAL_SYM:
         free(val->sym);
         break;
      case LVAL_FUN:
         if(!val->fun) {
            lenv_del(val->env);
            lval_del(val->formal);
            lval_del(val->body);
         }
         break;
      case LVAL_SEXPR:
      case LVAL_QEXPR:
         for(int iCell = 0; iCell < val->count; ++iCell)
            lval_del(val->cell[iCell]);
         free(val->cell);
         break;
      case LVAL_ERR:
         free(val->err);
         break;
   }

   // ...before freeing the lval itself
   free(val);
}

/* ---------- Reading lval ---------- */

lval *lval_read_num(mpc_ast_t *token)
{
   errno = 0;
   long x = strtol(token->contents, NULL, 10);
   return errno != ERANGE ? lval_num(x) : lval_err("Invalid number");
}

lval *lval_read(mpc_ast_t *token)
{
   // Numbers and symbols don't need any special treatment
   if(strstr(token->tag, "number")) return lval_read_num(token);
   if(strstr(token->tag, "symbol")) return lval_sym(token->contents);

   // If this is arg root (>) or an sexpr/qexpr
   // Then create an empty list that we'll fill
   lval *x = NULL;
   if(!strcmp(token->tag, ">") || strstr(token->tag, "sexpr")) 
      x = lval_sexpr();
   if(strstr(token->tag, "qexpr"))
      x = lval_qexpr();

   // Fill the list that we previously made using recursion
   int iChild;
   for(iChild = 0; iChild < token->children_num; ++iChild) {
      if( !strcmp(token->children[iChild]->contents, "(") ||
          !strcmp(token->children[iChild]->contents, ")") ||
          !strcmp(token->children[iChild]->contents, "{") ||
          !strcmp(token->children[iChild]->contents, "}") ||
          !strcmp(token->children[iChild]->tag, "regex"))
            continue;
      x = lval_add(x, lval_read(token->children[iChild]));
   }

   return x;
}

lval *lval_add(lval *val, lval *x)
{
   ++(val->count);
   val->cell = realloc(val->cell, sizeof(lval*) * val->count);
   val->cell[val->count - 1] = x;
   return val;
}

/* ---------- Printing lval ---------- */

void lval_print_expr(lval *val, char open, char close)
{
   int iCell;

   putchar(open);
   for(iCell = 0; iCell < val->count; ++iCell) {
      lval_print(val->cell[iCell]);
      if(iCell != (val->count - 1))
         putchar(' ');
   }
   putchar(close);
}

void lval_print(lval *val)
{
   switch(val->type) {
      case LVAL_NUM   : printf("%li", val->num); break;
      case LVAL_SYM   : printf("%s", val->sym); break;
      case LVAL_FUN   : 
         if(val->fun) 
            printf("<function>"); 
         else {
            printf("(\\ "); 
            lval_print(val->formal);
            putchar(' ');
            lval_print(val->body);
            putchar(')');
         }
         break;
      case LVAL_SEXPR : lval_print_expr(val, '(', ')'); break;
      case LVAL_QEXPR : lval_print_expr(val, '{', '}'); break;
      case LVAL_ERR   : printf("Error: %s", val->err); break;
   }
}

void lval_println(lval *val)
{
   lval_print(val);
   putchar('\n');
}

/* ---------- Misc ---------- */

// Extract an element (target) from an S-expression and shift the rest of the 
// list backward
lval *lval_pop(lval *val, int iChild)
{
   lval *target = val->cell[iChild];

   // Make the shift
   memmove( &val->cell[iChild], &val->cell[iChild + 1],
            sizeof(lval*) * (val->count - iChild - 1));

   // Update the list
   --(val->count);
   val->cell = realloc(val->cell, sizeof(lval*) * val->count);

   return target;
}

// Extract an element (target) and delete the rest of the list
lval *lval_take(lval *val, int iChild)
{
   lval *target = lval_pop(val, iChild);
   lval_del(val);
   return target;
}

lval *lval_join(lval *x, lval *y)
{
   int iChild;

   for(iChild = 0; iChild < y->count; ++iChild)
      x = lval_add(x, y->cell[iChild]);

   free(y->cell);
   free(y);

   return x;
}

lval *lval_copy(lval *val)
{
   lval *x = malloc(sizeof(lval));
   x->type = val->type;

   switch(val->type) {
      case LVAL_NUM:
         x->num = val->num;
         break;
      case LVAL_SYM:
         x->sym = malloc(strlen(val->sym) + 1);
         strcpy(x->sym, val->sym);
         break;
      case LVAL_FUN:
         if(val->fun)
            x->fun    = val->fun;
         else {
            x->fun    = NULL;
            x->env    = lenv_copy(val->env);
            x->formal = lval_copy(val->formal);
            x->body   = lval_copy(val->body);
         }
         break;
      case LVAL_SEXPR:
      case LVAL_QEXPR:
         x->count = val->count;
         x->cell = malloc(sizeof(lval*) * x->count);
         for(int iCell = 0; iCell < x->count; ++iCell)
            x->cell[iCell] = lval_copy(val->cell[iCell]);
         break;
      case LVAL_ERR:
         x->err = malloc(strlen(val->err) + 1);
         strcpy(x->err, val->err);
         break;
   }

   return x;
}

lval *lval_call(lenv *env, lval *func, lval *arg)
{
   // If this is a builtin, just apply it
   if(func->fun)
      return func->fun(env, arg);

   int given, total;

   given = arg->count;
   total = func->formal->count;

   while(arg->count) {
      // If there is no more formal argument to bind
      if(func->formal->count == 0) {
         lval_del(arg);
         return lval_err(  "Funtion passed too many arguments : "
                           "got %i (expected %i)", given, total);
      }

      lval *sym = lval_pop(func->formal, 0);
      lval *val = lval_pop(arg, 0);

      lenv_put(func->env, sym, val);

      lval_del(sym);
      lval_del(val);
   }

   lval_del(arg);

   // If all formals have been bound evaluate
   if(func->formal->count == 0) {
      func->env->parent = env;

      return builtin_eval(
               func->env, lval_add(lval_sexpr(), lval_copy(func->body)));
   }
   // Partially evaluated function
   else
      return lval_copy(func);
}

char *ltype_name(int t)
{
   switch(t) {
      case LVAL_NUM   : return "Number";
      case LVAL_SYM   : return "Symbol";
      case LVAL_FUN   : return "Function";
      case LVAL_SEXPR : return "S-Expression";
      case LVAL_QEXPR : return "Q-Expression";
      case LVAL_ERR   : return "Error";
      default         : return "Unknown";
   }
}

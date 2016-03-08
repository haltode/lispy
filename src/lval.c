#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lval.h"

/* ---------- Constructors & Destructors ---------- */

lval *lval_num(long num)
{
   lval *v = malloc(sizeof(lval));
   v->type = LVAL_NUM;
   v->num  = num;
   return v;
}

lval *lval_sym(char *sym)
{
   lval *v = malloc(sizeof(lval));
   v->type = LVAL_SYM;
   v->sym  = malloc(strlen(sym) + 1);
   strcpy(v->sym, sym);
   return v;
}

lval *lval_sexpr(void)
{
   lval *v  = malloc(sizeof(lval));
   v->type  = LVAL_SEXPR;
   v->cell  = NULL;
   v->count = 0;
   return v;
}

lval *lval_err(char *err)
{
   lval *v = malloc(sizeof(lval));
   v->type = LVAL_ERR;
   v->err  = malloc(strlen(err) + 1);
   strcpy(v->err, err);
   return v;
}

void lval_del(lval *v)
{
   // Free everything that the lval is pointing to...
   switch(v->type) {
      case LVAL_NUM:
         break;
      case LVAL_SYM:
         free(v->sym);
         break;
      case LVAL_SEXPR:
         for(int iCell = 0; iCell < v->count; ++iCell)
            lval_del(v->cell[iCell]);
         free(v->cell);
         break;
      case LVAL_ERR:
         free(v->err);
         break;
   }

   // ...before freeing the lval itself
   free(v);
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

   // If this is a root (>) or an sexpr, then create an empty list that we'll fill
   lval *x = NULL;
   if(!strcmp(token->tag, ">") || strstr(token->tag, "sexpr")) 
      x = lval_sexpr();

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

lval *lval_add(lval *v, lval *x)
{
   ++(v->count);
   v->cell = realloc(v->cell, sizeof(lval*) * v->count);
   v->cell[v->count - 1] = x;
   return v;
}

/* ---------- Printing lval ---------- */

void lval_expr_print(lval *v, char open, char close)
{
   int iCell;

   putchar(open);
   for(iCell = 0; iCell < v->count; ++iCell) {
      lval_print(v->cell[iCell]);
      if(iCell != (v->count - 1))
         putchar(' ');
   }
   putchar(close);
}

void lval_print(lval *v)
{
   switch(v->type) {
      case LVAL_NUM   : printf("%li", v->num); break;
      case LVAL_SYM   : printf("%s", v->sym); break;
      case LVAL_SEXPR : lval_expr_print(v, '(', ')'); break;
      case LVAL_ERR   : printf("Error: %s!", v->err); break;
   }
}

void lval_println(lval *v)
{
   lval_print(v);
   putchar('\n');
}

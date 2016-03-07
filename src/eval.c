#include <string.h>
#include <math.h>

#include "eval.h"

lval eval_op(char *op, lval x, lval y)
{
   // Return the lval in case it is an error
   if(x.type == LVAL_ERR) return x;
   if(y.type == LVAL_ERR) return y;

   // Possible operation
   if(!strcmp(op, "+") || !strcmp(op, "add")) return lval_num(x.num + y.num);
   if(!strcmp(op, "-") || !strcmp(op, "sub")) return lval_num(x.num - y.num);
   if(!strcmp(op, "*") || !strcmp(op, "mul")) return lval_num(x.num * y.num);
   if(!strcmp(op, "/") || !strcmp(op, "div"))
      return y.num == 0 
         ? lval_err(LERR_DIV_ZERO)
         : lval_num(x.num / y.num);
   if(!strcmp(op, "%") || !strcmp(op, "mod"))
      return y.num == 0
         ? lval_err(LERR_MOD_ZERO)
         : lval_num(x.num % y.num);
   if(!strcmp(op, "^") || !strcmp(op, "pow")) return lval_num(pow(x.num, y.num));
   if(!strcmp(op, "min")) return x.num < y.num ? lval_num(x.num) : lval_num(y.num);
   if(!strcmp(op, "max")) return x.num > y.num ? lval_num(x.num) : lval_num(y.num);

   return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t *token)
{
   // If it is a number, check it and return it (or raise the error)
   if(strstr(token->tag, "number")) {
      errno = 0;
      long x = strtol(token->contents, NULL, 10);
      return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
   }

   // Expression is either : number or ( op expr ... )
   char *op = token->children[1]->contents;
   lval x   = eval(token->children[2]);

   // Special case when we have : - num
   // We just want to get -num
   if(token->children_num == 4 && 
      (!strcmp(op, "-") || !strcmp(op, "sub")))
      x.num = -x.num;

   int iChild = 3;
   while(strstr(token->children[iChild]->tag, "expr")) {
      x = eval_op(op, x, eval(token->children[iChild]));
      ++iChild;
   }

   return x;
}

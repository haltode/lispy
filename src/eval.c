#include <string.h>
#include <math.h>

#include "eval.h"

long eval_op(char *op, long x, long y)
{
   if(!strcmp(op, "+") || !strcmp(op, "add")) return x + y;
   if(!strcmp(op, "-") || !strcmp(op, "sub")) return x - y;
   if(!strcmp(op, "*") || !strcmp(op, "mul")) return x * y;
   if(!strcmp(op, "/") || !strcmp(op, "div")) return x / y;
   if(!strcmp(op, "%") || !strcmp(op, "mod")) return x % y;
   if(!strcmp(op, "^") || !strcmp(op, "pow")) return pow(x, y);
   if(!strcmp(op, "min")) return (x < y) ? x : y;
   if(!strcmp(op, "max")) return (x > y) ? x : y;

   return 0;
}

long eval(mpc_ast_t *token)
{
   // If it is a number, return it
   if(strstr(token->tag, "number"))
      return atoi(token->contents);

   // Expression is either : number or ( op expr ... )
   char *op = token->children[1]->contents;
   long x   = eval(token->children[2]);

   // Special case when we have : - num
   // We just want to get -num
   if(token->children_num == 4 && 
      (!strcmp(op, "-") || !strcmp(op, "sub")))
      x = -x;

   int iChild = 3;
   while(strstr(token->children[iChild]->tag, "expr")) {
      x = eval_op(op, x, eval(token->children[iChild]));
      ++iChild;
   }

   return x;
}

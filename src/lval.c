#include <stdio.h>

#include "lval.h"

lval lval_num(long num)
{
   lval v;
   v.type = LVAL_NUM;
   v.num = num;
   return v;
}

lval lval_err(int err)
{
   lval v;
   v.type = LVAL_ERR;
   v.err = err;
   return v;
}

void lval_print(lval v)
{
   switch(v.type) {
      case LVAL_NUM:
         printf("%li", v.num);
         break;

      case LVAL_ERR:
         if(v.err == LERR_DIV_ZERO)
            printf("Error: Division by zero!");
         if(v.err == LERR_MOD_ZERO)
            printf("Error: Modulo by zero!");
         if(v.err == LERR_BAD_OP)
            printf("Error: Unknown operator!");
         if(v.err == LERR_BAD_NUM)
            printf("Error: Invalid number!");
         break;

      default:
         break;
   }
}

void lval_println(lval v)
{
   lval_print(v);
   putchar('\n');
}

#include "parser.h"

void init_parsers( mpc_parser_t **Number, mpc_parser_t **Operator,
                   mpc_parser_t **Expr, mpc_parser_t **Lispy)
{
   *Number   = mpc_new("number");
   *Operator = mpc_new("operator");
   *Expr     = mpc_new("expr");
   *Lispy    = mpc_new("lispy");

   mpca_lang(MPCA_LANG_DEFAULT, 
      "                                                                 \
         number   :  /-?[0-9]+/ ;                                       \
         operator :  '+' | '-' | '*' | '/' | '%' | '^' |                \
                     \"add\" | \"sub\" | \"mul\" | \"div\" | \"mod\" |  \
                     \"pow\" |                                          \
                     \"min\" | \"max\";                                 \
         expr     :  <number> | '(' <operator> <expr>+ ')' ;            \
         lispy    :  /^/ <operator> <expr>+ /$/ ;                       \
      ",
      *Number, *Operator, *Expr, *Lispy);
}

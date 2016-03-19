#include "parser.h"

mpc_parser_t *Number;
mpc_parser_t *String;
mpc_parser_t *Comment;
mpc_parser_t *Symbol;
mpc_parser_t *Sexpr;
mpc_parser_t *Qexpr;
mpc_parser_t *Expr;
mpc_parser_t *Lispy;

void init_parsers(void)
{
   Number  = mpc_new("number");
   String  = mpc_new("string");
   Comment = mpc_new("comment");
   Symbol  = mpc_new("symbol");
   Sexpr   = mpc_new("sexpr");
   Qexpr   = mpc_new("qexpr");
   Expr    = mpc_new("expr");
   Lispy   = mpc_new("lispy");

   mpca_lang(MPCA_LANG_DEFAULT, 
      "                                                  \
         number   : /-?[0-9]+/ ;                         \
         string   : /\"(\\\\.|[^\"])*\"/ ;               \
         comment  : /;[^\\r\\n]*/ ;                      \
         symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;   \
         sexpr    : '(' <expr>* ')' ;                    \
         qexpr    : '{' <expr>* '}' ;                    \
         expr     :  <number> | <string>  | <comment> |  \
                     <symbol> | <sexpr>   | <qexpr> ;    \
         lispy    : /^/ <expr>* /$/ ;                    \
      ",
      Number, String, Comment, Symbol, Sexpr, Qexpr, Expr, Lispy);
}

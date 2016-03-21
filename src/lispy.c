#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"
#include "prompt.h"
#include "parser.h"
#include "eval.h"
#include "lval.h"

mpc_parser_t *Number;
mpc_parser_t *String;
mpc_parser_t *Comment;
mpc_parser_t *Symbol;
mpc_parser_t *Sexpr;
mpc_parser_t *Qexpr;
mpc_parser_t *Expr;
mpc_parser_t *Lispy;


int main(int argc, char **argv)
{
   init_parsers();

   lenv *env = lenv_new();
   lenv_add_builtins(env);

   // Load standard library
   lval *lib = lval_add(lval_sexpr(), lval_str("lib/stdlib.lspy"));
   builtin_load(env, lib);

   // Interactive prompt
   if(argc == 1) {
      puts("Lispy Version 0.0.0.1.1");
      puts("Press Ctrl+C to Exit\n");

      while(1) {
         char *input = readline("lispy> ");
         add_history(input);

         // Parse the user input and evaluate it
         mpc_result_t res;
         if(mpc_parse("<stdin>", input, Lispy, &res)) {
            lval *output = lval_eval(env, lval_read(res.output));
            lval_println(output);
            lval_del(output);
            mpc_ast_delete(res.output);
         }
         else {
            mpc_err_print(res.error);
            mpc_err_delete(res.error);
         }

         free(input);
      }
   }
   // Loading scripts from list of files
   else if(argc >= 2) {
      int iArg;
      for(iArg = 1; iArg < argc; ++iArg) {
         lval *arg = lval_add(lval_sexpr(), lval_str(argv[iArg]));

         lval *x = builtin_load(env, arg);
         if(x->type == LVAL_ERR)
            lval_println(x);
         lval_del(x);
      }
   }
   
   lenv_del(env);
   mpc_cleanup(8, Number, String, Comment, Symbol, Sexpr, Qexpr, Expr, Lispy);

   return 0;
}

#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"
#include "prompt.h"
#include "parser.h"
#include "eval.h"
#include "lval.h"

int main(void)
{
   mpc_parser_t *Number, *Symbol, *Sexpr, *Qexpr, *Expr, *Lispy;
   init_parsers(&Number, &Symbol, &Sexpr, &Qexpr, &Expr, &Lispy);

   lenv *env = lenv_new();
   lenv_add_builtins(env);

   puts("Lispy Version 0.0.0.0.8");
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

   lenv_del(env);
   mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

   return 0;
}

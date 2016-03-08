#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "prompt.h"
#include "eval.h"
#include "lval.h"

int main(void)
{
   // Create and init parsers
   mpc_parser_t *Number, *Symbol, *Sexpr, *Expr, *Lispy;
   init_parsers(&Number, &Symbol, &Sexpr, &Expr, &Lispy);

   puts("Lispy Version 0.0.0.0.5");
   puts("Press Ctrl+C to Exit\n");

   while(1) {
      char *input;
      input = readline("lispy> ");

      add_history(input);

      // Parse the user input and evaluate it
      mpc_result_t res;
      if(mpc_parse("<stdin>", input, Lispy, &res)) {
         lval *output = lval_eval(lval_read(res.output));
         lval_println(output);
         lval_del(output);
      }
      else {
         mpc_err_print(res.error);
         mpc_err_delete(res.error);
      }

      free(input);
   }

   // Undefine and delete parsers
   mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);

   return 0;
}

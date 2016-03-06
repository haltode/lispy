#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "prompt.h"
#include "eval.h"

int main(void)
{
   // Create and init parsers
   mpc_parser_t *Number, *Operator, *Expr, *Lispy;
   init_parsers(&Number, &Operator, &Expr, &Lispy);

   puts("Lispy Version 0.0.0.0.3");
   puts("Press Ctrl+C to Exit\n");

   while(1) {
      char *input;
      input = readline("lispy> ");

      add_history(input);

      // Parse the user input and evaluate it
      mpc_result_t res;
      if(mpc_parse("<stdin>", input, Lispy, &res)) {
         printf("%li\n", eval(res.output));
         mpc_ast_delete(res.output);
      }
      else {
         mpc_err_print(res.error);
         mpc_err_delete(res.error);
      }

      free(input);
   }

   // Undefine and delete parsers
   mpc_cleanup(4, Number, Operator, Expr, Lispy);

   return 0;
}

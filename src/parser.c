#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"


/* Windows */
#ifdef _WIN32
#include <string.h>

#define BUFFER_SIZE 2048

static char buffer[BUFFER_SIZE];

char *readline(char *prompt)
{
   fputs(prompt, stdout);
   fgets(buffer, BUFFER_SIZE, stdin);

   char *copy;
   copy = malloc(strlen(buffer) + 1);
   strcpy(copy, buffer);
   copy[strlen(copy) - 1] = '\0';

   return copy;
}

void add_history(char *unused)
{

}

/* Linux and Mac */
#else
#include <editline/readline.h>
#include <histedit.h>
#endif


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
         operator :  '+' | '-' | '*' | '/' | '%' |                      \
                     \"add\" | \"sub\" | \"mul\" | \"div\" | \"mod\" ;  \
         expr     :  <number> | '(' <operator> <expr>+ ')' ;            \
         lispy    :  /^/ <operator> <expr>+ /$/ ;                       \
      ",
      *Number, *Operator, *Expr, *Lispy);
}

int main(void)
{
   /* Create and init parsers */
   mpc_parser_t *Number, *Operator, *Expr, *Lispy;
   init_parsers(&Number, &Operator, &Expr, &Lispy);

   puts("Lispy Version 0.0.0.0.2");
   puts("Press Ctrl+C to Exit\n");

   while(1) {
      char *input;
      input = readline("lispy> ");

      add_history(input);

      /* Parse the user input */
      mpc_result_t r;
      if(mpc_parse("<stdin>", input, Lispy, &r)) {
         mpc_ast_print(r.output);
         mpc_ast_delete(r.output);
      }
      else {
         mpc_err_print(r.error);
         mpc_err_delete(r.error);
      }

      free(input);
   }

   /* Undefine and delete parsers */
   mpc_cleanup(4, Number, Operator, Expr, Lispy);

   return 0;
}

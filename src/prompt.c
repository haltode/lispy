#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <histedit.h>

int main(void)
{
   puts("Lispy Version 0.0.0.0.1");
   puts("Press Ctrl+C to Exit\n");

   while(1) {
      char *input;
      input = readline("lispy> ");

      add_history(input);
      printf("You typed : %s\n", input);

      free(input);
   }

   return 0;
}

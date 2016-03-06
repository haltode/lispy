#include <stdio.h>
#include <stdlib.h>

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

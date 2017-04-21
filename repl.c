#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

int main(int argc, char **argv) {

  puts("Lispy version 0.0.1");
  puts("Press Ctrl-C to quit\n");

  while(1) {
    char *input = readline("lispy > "); //uses malloc

    add_history(input);

    printf("No you're a %s\n", input);

    free(input); //frees memory

  }

  return 0;

}

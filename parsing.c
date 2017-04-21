#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "mpc.h"

int main(int argc, char **argv) {
  //
//MPC parser init - parsers

mpc_parser_t *Number = mpc_new("number");
mpc_parser_t *Expr = mpc_new("expr");
mpc_parser_t *Operator = mpc_new("operator");
mpc_parser_t *Lispy = mpc_new("lispy");

//define language of parsers

mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
    number   : /-?[0-9]+/ ;                             \
    operator : '+' | '-' | '*' | '/' ;                  \
    expr     : <number> | '(' <operator> <expr>+ ')' ;  \
    lispy    : /^/ <operator> <expr>+ /$/ ;             \
    ",
    Number, Operator, Expr, Lispy);


  puts("Lispy version 0.0.1");
  puts("Press Ctrl-C to quit\n");

  while(1) {
    char *input = readline("lispy > "); //uses malloc

    add_history(input);

    printf("No you're a %s\n", input);

    free(input); //frees memory

  }

  mpc_cleanup(4, Number, Operator, Expr, Lispy);

  return 0;

}

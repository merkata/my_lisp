#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "mpc.h"

typedef struct {
  int type;
  long result;
  int error;
} lval;

enum { LVAL_NUM, LVAL_ERR };
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

lval eval(mpc_ast_t *t);
lval eval_op(lval x, char *op, lval y);
void usage(void);
void throw_error(mpc_result_t *r);
void prepare_ast(char *input, char *ast);
lval lval_num(long result);
lval lval_err(int err);
void lval_print(lval v);

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
    char ast[81]; //I don't know how to malloc ;(

    if(strcmp(input, "\n")) {
      continue;
    }

    add_history(input);

    //parse user input
    mpc_result_t r;
    if(strstr(input, "\\h")) {
      usage();
    } else if(strstr(input, "\\a")) {
      prepare_ast(input, ast);
      if(mpc_parse("<stdin>", ast, Lispy, &r)) {
        //parsed successfully
        mpc_ast_print(r.output);
      } else {
        throw_error(&r);
      } 
    } else if(strstr(input, "\\i")) {
      prepare_ast(input, ast);
      if(mpc_parse("<stdin>", ast, Lispy, &r)) {
        //parsed successfully
        mpc_ast_t *inspect = r.output;
        printf("Root AST of tag %s\n", inspect->tag);
        printf("Contents is %s\n", inspect->contents);
        printf("AST has %d children\n", inspect->children_num);
        printf("Descending into first child...\n");
        mpc_ast_t *child0 = inspect->children[0];
        printf("Child AST has tag %s\n", child0->tag);
        printf("Contents of child is %s\n", child0->contents);
        printf("%d children under child letf\n", child0->children_num);
      } else {
        throw_error(&r);
      }
    } else {
      if(mpc_parse("<stdin>", input, Lispy, &r)) {
        lval result = eval(r.output);
        lval_print(result);
        mpc_ast_delete(r.output);
      } else {
        throw_error(&r);
      }
    }

    free(input); //frees memory

  }

  mpc_cleanup(4, Number, Operator, Expr, Lispy);

  return 0;

}

lval eval(mpc_ast_t *t) {
  if(strstr(t->tag, "number")) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }

  char *op = t->children[1]->contents;

  lval x = eval(t->children[2]);

  int i = 3;
  while(strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  return x;
}

lval eval_op(lval x, char *op, lval y) {
  if(strcmp(op, "+") == 0 ) { return lval_num(x.result + y.result); }
  if(strcmp(op, "-") == 0 ) { return lval_num(x.result - y.result); }
  if(strcmp(op, "*") == 0 ) { return lval_num(x.result * y.result); }
  if(strcmp(op, "/") == 0 ) { 
    return y.result == 0
      ? lval_err(LERR_DIV_ZERO)
      : lval_num(x.result / y.result);
  }
  return lval_err(LERR_BAD_OP);
}

void usage(void) {
  printf("Please specify one of following:\n");
  printf("\\h -> prints this nifty help\n");
  printf("\\a <expression> -> prints the AST of an expression\n");
  printf("\\i <expression> -> inspects the AST of an expression\n");
  printf("<expression> -> prints the evaluated AST result\n");
}

void prepare_ast(char *input, char *ast) {
  int i, pos;
  for(i = 3, pos = 0; input[i] != '\0'; i++, pos++) {
    ast[pos] = input[i];
  }
  ast[pos+1] = '\0';
}

void throw_error(mpc_result_t *r) {
  //we did not parse correctly
  mpc_err_print(r->error);
  mpc_err_delete(r->error);
}

lval lval_num(long result) {
  lval v;
  v.result = result;
  v.type = LVAL_NUM;
  return v;
}

lval lval_err(int err) {
  lval v;
  v.error = err;
  v.type = LVAL_ERR;
  return v;
}

void lval_print(lval v) {
  switch(v.type) {
    case LVAL_ERR:
      if(v.error == LERR_DIV_ZERO) {
        printf("Division by zero error!\n");
      }
      if(v.error == LERR_BAD_OP) {
        printf("Bad operator as value!\n");
      }
      if(v.error == LERR_BAD_NUM) {
        printf("Number outside boundaries!\n");
      }
      break;
    case LVAL_NUM:
      printf("Received a value of %li\n", v.result);
      break;
  }
}

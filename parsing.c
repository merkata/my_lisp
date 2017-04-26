#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "mpc.h"

/* main lisp structure - the lval */
typedef struct {
  int type;
  long result;
  /* descriptive errors */
  char *error;
  /* symbol type such as "+" */
  char *sym;
  /* count and pointer to lval */
  int count;
  struct lval **cell;
} lval;

/* lval type */
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };

/* definitions */
void usage(void);
void throw_error(mpc_result_t *r);
void prepare_ast(char *input, char *ast);
lval *lval_num(long result);
lval *lval_err(char *err);
lval *lval_sym(char *sym);
lval *lval_sexpr(void);
lval *lval_read_num(mpc_ast_t *t);
lval *lval_read(mpc_ast_t *t);
lval *lval_add(lval *l, lval *r);
void lval_print(lval *l);
void lval_expr_print(lval *l, char a, char b);
void lval_delete(lval *l);

/* main REPL */
int main(int argc, char **argv) {
  //
  //MPC parser init - parsers

  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Sexpr = mpc_new("sexpr");
  mpc_parser_t *Symbol = mpc_new("symbol");
  mpc_parser_t *Lispy = mpc_new("lispy");

  //define language of parsers

  mpca_lang(MPCA_LANG_DEFAULT,
      "                                                   \
      number   : /-?[0-9]+/ ;                             \
      operator : '+' | '-' | '*' | '/' ;                  \
      sexpr    : '(' <expr>* ')' ;                        \
      expr     : <number> | <symbol> | <sexpr>            \
      lispy    : /^/ <operator> <expr>+ /$/ ;             \
      ",
      Number, Symbol, Sexpr, Expr, Lispy);

  puts("Lispy version 0.0.1");
  puts("Press Ctrl-C to quit\n");

  while(1) {
    char *input = readline("lispy > "); //uses malloc
    char ast[81]; //I don't know how to malloc ;(

    if (strcmp(input, "") == 0) {
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
        lval *result = lval_read(r.output);
        lval_print(result);
        lval_delete(result);
      } else {
        throw_error(&r);
      }
    }

    free(input); //frees memory

  }

  mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);

  return 0;

}

/* helper functions */
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

/* lval constructors */

lval *lval_num(long result) {
  lval *v = malloc(sizeof(lval));
  v->result = result;
  v->type = LVAL_NUM;
  return v;
}

lval *lval_err(char *err) {
  lval *v = malloc(sizeof(lval));
  v->error = malloc(strlen(err) + 1);
  strcpy(v->error, err);
  v->type = LVAL_ERR;
  return v;
}

lval *lval_sym(char *s) {
  lval *v = malloc(sizeof(lval));
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  v->type = LVAL_SYM;
  return v;
}

lval *lval_sexpr(void) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

/* lval deconstructor */

void lval_delete(lval *v) {
  switch(v->type) {
    case LVAL_NUM:
      break;
    case LVAL_ERR:
      free(v->error);
      break;
    case LVAL_SYM:
      free(v->sym);
      break;
    case LVAL_SEXPR:
      for(int i = 0; i < v->count; i++) {
        lval_delete(v->cell[i]);
      }
      free(v->cell);
  }

  free(v);
}

/* lval evaluations */

lval *lval_read_num(mpc_ast_t *t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno == ERANGE
    ? lval_num(x)
    : lval_err("invalid number");
}

lval *lval_read(mpc_ast_t *t) {
  /* return primitive types number and symbol */
  if(strstr(t->tag, "number")) { return lval_num(t->contents); }
  if(strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

  /* when root (>) or s-expr */
  lval *x = NULL;
  if(strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if(strstr(t->tag, "sexpr")) { x = lval_sexpr(); }

  for(int i = 0; i < t->children_num; i++) {
    if(strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if(strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if(strcmp(t->children[i]->tag, "regex") == 0) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }

  return x;
}

lval *lval_add(lval *l, lval *r) {
  l->count++;
  l->cell = realloc(l->cell, sizeof(lval*) * l->count);
  l->cell[l->count - 1] = r;
  return l;
}

/* lval printing functions */

void lval_expr_print(lval *v, char open, char close) {
  putchar(open);
  for(int i = 0; i < v->count; i++ ) {

    /* print the child structure one by one */
    lval_print(v->cell[i]);

  }

  putchar(close);
}

void lval_print(lval *v) {
  switch(v->type) {
    case LVAL_NUM: printf("%li", v->result); break;
    case LVAL_ERR: printf("Error %s", v->error); break;
    case LVAL_SYM: printf("%s", v->sym); break;
    case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
  }
}

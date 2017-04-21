#include <stdio.h>

void hi_world(int);

int main(int argc, char **argv) {

  char line[80]; //line of input
  int count = 1; //print Hi World minimm one time
  const char *errstr; //error string set by strtonum

  printf("Please enter a number to repeat Hi World: ");
                                                       
  fgets(line, sizeof(line), stdin);
  printf("Got line\n%s\n", line);
  sscanf(line, "%d", &count);

  hi_world(count);

  return 0;
}

void hi_world(int count) {

  puts("With while loop");
  while(count > 0) {
    puts("Hello Lisp!");
    count--;
  }

  puts("With for loop");
  for(int i = 0; i < count ; i++) {
    puts("Hello Lisp!");
  }
}

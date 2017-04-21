hello_world: hello_world.c
	cc -I/usr/local/include -std=c99 -Wall -pedantic -Wextra hello_world.c -o hello_world

repl: repl.c
	cc -L/usr/local/lib -I/usr/local/include -std=c99 -Wall -pedantic -Wextra -lreadline repl.c -o repl

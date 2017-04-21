hello_world: hello_world.c
	cc -I/usr/local/include -std=c99 -Wall -pedantic -Wextra hello_world.c -o hello_world

parsing: parsing.c mpc.c
	cc -g -L/usr/local/lib -I/usr/local/include -std=c99 -Wall -pedantic -Wextra -lm -lreadline mpc.c parsing.c -o parsing

main: src/main.c src/store.c src/store.h
	cc src/main.c src/store.c -Wall -Wextra -std=c11 -o copycached

repl-legacy:
	cc repl-legacy/main.c -Wall -Wextra -std=c11 -o repl

main: src/main.c src/store.c src/store.h
	cc src/main.c src/store.c -Wall -Wextra -std=c11 -D_BSD_SOURCE -fsanitize=address,undefined -g -o copycached

repl-legacy:
	cc repl-legacy/main.c -Wall -Wextra -std=c11 -D_BSD_SOURCE -o repl

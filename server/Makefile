CC= gcc
CFLAGS= -Wall -Wextra -g

db: db.c
	$(CC) $(CFLAGS) db.c -o db -lsqlite3

PHONY: clean

clean:
	rm *.o
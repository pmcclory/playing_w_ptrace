all: strace

CFLAGS := -Werror -Wall -Wextra

strace: strace.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf strace

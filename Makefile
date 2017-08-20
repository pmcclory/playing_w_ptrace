all: strace

LDFLAGS := -laudit
CFLAGS := -Werror -Wall -Wextra

strace: strace.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

clean:
	rm -rf strace

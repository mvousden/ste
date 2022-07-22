CFLAGS = -O3 -Wall -Wextra -pedantic -std=c89

all: ste ste.so string_helpers_test

ste: ste_executable.c ste.c string_helpers.c ste.h string_helpers.h
	${CC} ${CFLAGS} $(filter %.c, $^) -o $@

ste.so: ste.c string_helpers.c ste.h string_helpers.h
	${CC} ${CFLAGS} -shared -fPIC -o $@ $(filter %.c, $^)

string_helpers_test: string_helpers_test.c string_helpers.c string_helpers.h
	${CC} ${CFLAGS} $(filter %.c, $^) -o $@

clean:
	rm --force ste ste.so string_helpers_test

.PHONY: all clean

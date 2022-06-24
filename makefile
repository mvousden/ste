CFLAGS = -O3 -Wall -Wextra -pedantic -std=c89
LFLAGS = -lbsd

all: ste ste.so

ste: ste_executable.c ste.c ste.h
	${CC} ${CFLAGS} $(filter %.c, $^) -o $@ ${LFLAGS}

ste.so: ste.c ste.h
	${CC} ${CFLAGS} -shared -fPIC -o $@ $(filter %.c, $^) ${LFLAGS}

clean:
	rm --force ste ste.so

.PHONY: all clean

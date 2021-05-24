CFLAGS = -O3 -Wall -Wextra -pedantic -std=c89
LFLAGS = -lbsd

all: templater templater.so

templater: template_executable.c template.c template.h
	${CC} ${CFLAGS} ${LFLAGS} $(filter %.c, $^) -o $@

templater.so: template.c template.h
	${CC} ${CFLAGS} ${LFLAGS} -shared -fPIC -o $@ $(filter %.c, $^)

clean:
	rm --force templater templater.so

.PHONY: all clean

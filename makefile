CFLAGS = -O3 -Wall -Wextra -pedantic -std=c89

all: templater templater.so

templater: template_executable.c template.c template.h
	${CC} ${CFLAGS} $(filter %.c, $^) -o $@

templater.so: template.c template.h
	${CC} ${CFLAGS} -shared -fPIC -o $@ $(filter %.c, $^)

clean:
	rm --force templater templater.so

.PHONY: all clean

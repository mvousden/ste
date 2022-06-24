#include <bsd/string.h>
#include <errno.h>
#include <libgen.h>  /* POSIX */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
int template_files(const char*, const char*, char**, char**, const unsigned);
int template(FILE*, FILE*, const char*, char**, char**, const unsigned);

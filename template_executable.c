#include "template.h"

/* e.g. ./template "input_file" "output_directory/output_file" */

int main(const int argc, const char** argv)
{
    /* Complain about bad arguments. */
    if (argc != 3)
    {
        fprintf(stderr,
                "Argument 1: Input template. Argument 2: Destination. No "
                "more, no less.\n");
        return 1;
    }

    /* Go go go */
    return template_files(argv[1], argv[2]);
}

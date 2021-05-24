#include "template.h"

/* Call as:
 *
 *  ./template "input_file" "output_directory/output_file" "handle=value"
 *
 * For as many handle=values as you like. */

int main(const int argc, char** argv)
{
    const int argOffset = 3;

    char** valStrs;
    char** valVals;
    size_t valIndex;

    unsigned numVals;
    char* handle;
    size_t handleLength;
    char* value;
    size_t valLength;

    int rc;

    const char* errMsg = "Argument 1: Input template. Argument 2: "
        "Destination. Arguments 3+: handle=value. Whitespace matters.\n";

    /* Complain about bad arguments. */
    if (argc < argOffset)
    {
        fprintf(stderr, errMsg);
        return 1;
    }

    /* Allocate and define handle and value maps. */
    numVals = argc - argOffset;
    valStrs = (char**) malloc(numVals * sizeof(char*));
    valVals = (char**) malloc(numVals * sizeof(char*));
    for (valIndex = 0; valIndex < numVals; valIndex++)
    {
        /* Grab the handle and value from the input argument. */
        handle = strtok(argv[valIndex + argOffset], "=");
        value = strtok(NULL, "=");
        if (handle == NULL || value == NULL)
        {
            fprintf(stderr, errMsg);
            return 1;
        }

        /* Copy contents into structures. */
        handleLength = strlen(handle) + 1;
        valStrs[valIndex] = (char*) malloc(handleLength * sizeof(char));
        strlcpy(valStrs[valIndex], handle, handleLength);

        valLength = strlen(value) + 1;
        valVals[valIndex] = (char*) malloc(valLength * sizeof(char));
        strlcpy(valVals[valIndex], value, valLength);
    }

    /* Go go go */
    rc = template_files(argv[1], argv[2], valStrs, valVals, numVals);

    /* Cleanup */
    for (valIndex = 0; valIndex < numVals; valIndex++)
    {
        free(valStrs[valIndex]);
        free(valVals[valIndex]);
    }
    free(valStrs);
    free(valVals);

    return rc;
}

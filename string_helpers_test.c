#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_helpers.h"

int min(int first, int second){return first < second ? first : second;}

int main()
{
    /* We're going to make some applepie. */
    char* destination;
    int rcCpy;
    int rcCpyExpected;
    int rcCat;
    int rcCatExpected;
    const char* base = "apple";
    int baseLength = 5;
    const char* source = "pie";
    int sourceLength = 3;
    unsigned bufSize;
    int fail = 0;  /* bool-like */
    int failCount = 0;
    for (bufSize = 1; bufSize < 10; bufSize++)
    {
        /* Copy and cat */
        destination = (char*) malloc(sizeof(char) * bufSize);
        rcCpy = ste_strlcpy(destination, base, bufSize);
        rcCat = ste_strlcat(destination, source, bufSize);

        /* Checking return codes and destination buffer. */
        switch(bufSize)
        {
        case 1:
            fail = strcmp(destination, "");
            break;
        case 2:
            fail = strcmp(destination, "a");
            break;
        case 3:
            fail = strcmp(destination, "ap");
            break;
        case 4:
            fail = strcmp(destination, "app");
            break;
        case 5:
            fail = strcmp(destination, "appl");
            break;
        case 6:
            fail = strcmp(destination, "apple");
            break;
        case 7:
            fail = strcmp(destination, "applep");
            break;
        case 8:
            fail = strcmp(destination, "applepi");
            break;
        case 9:
            fail = strcmp(destination, "applepie");
            break;
        }
        rcCpyExpected = baseLength;
        rcCatExpected = min(baseLength + sourceLength,
                            sourceLength + (int)bufSize - 1);
        fail = rcCpy != rcCpyExpected || rcCat != rcCatExpected;

        /* Punishment for transgression. */
        if (fail)
        {
            fprintf(stderr,
                   "Failed with buffer size %u.\n"
                   " - Copy returned %u (expected %u).\n"
                   " - Cat returned %u (expected %u).\n"
                   " - Buffer contains '%s'.\n",
                   bufSize,
                   rcCpy, rcCpyExpected,
                   rcCat, rcCatExpected,
                   destination);
            failCount++;
        }
        free(destination);
    }
    return failCount;
}

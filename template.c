/* There's very little in here that protects the user, so be careful! */

#include "template.h"

#define MOUSTACHE_OPEN_CODE 123  /* '{', you'll need two of them */
#define MOUSTACHE_CLOSE_CODE 125  /* '}', you'll need two of them */
#define MOUSTACHE_BUFFER_SIZE 1000  /* Scream if you want to go faster! */

/* Simple wrapper around template to handle file opening and closing. inPath
 * and outPath may be string literal. */
int template_files(char* inPath, char* outPath)
{
    /* File objects go here, one for each path provided. */
    FILE* inFile;
    FILE* outFile;

    char* inPathCopy;  /* For directory extraction */
    char* dir;  /* Directory will go here */
    int rc;  /* Returncode */

    /* Open the input and output files. */
    inFile = fopen(inPath, "rb+");
    if (inFile == NULL)
    {
        fprintf(stderr,
                "Error opening input file '%s': %s.\n",
                inPath, strerror(errno));
        return 1;
    }

    outFile = fopen(outPath, "wb");
    if (outFile == NULL)
    {
        fprintf(stderr,
                "Error opening output file '%s': %s.\n",
                outPath, strerror(errno));
        fclose(inFile);
        return 1;
    }

    /* Grab the directory containing the input file. If inPath is a string
     * literal, we'll segmentation fault here, so copy the buffer somewhere we
     * can modify. POSIX! */
    inPathCopy = (char*) malloc(sizeof(char*) * (strlen(inPath) + 1));
    strcpy(inPathCopy, inPath);
    dir = dirname(inPathCopy);

    /* Use templating engine. */
    rc = template(inFile, outFile, dir);

    /* Cleanup. */
    free(inPathCopy);
    fclose(inFile);
    fclose(outFile);
    return rc;
}

/* This function performs templating on the file at "inFile", and writes it to
 * file at "outFile". Both character arrays must be null-terminated.
 *
 * Also accepts a character array "dir", which is the base directory for all
 * template files, and must fit with each template in MOUSTACHE_BUFFER_SIZE.
 *
 * If there are no moustaches in the file at "inFile", the file is simply
 * copied to "outFile".
 *
 * inFile must be opened in read-binary mode, and outFile must be opened in
 * write-binary mode for this to work as intended (but by all means play
 * around).
 *
 * You can even move the read/write pointers to parse certain areas of files -
 * this is exploited when recursing moustaches.
 *
 * Returns 0 if no errors occured, and errno (non-zero) otherwise. */
int template(FILE* inFile, FILE* outFile, char* dir)
{
    /* Characters, read from inFile. */
    int thisChar;
    int previousChar;

    /* Moustache mode */
    int moustacheMode;  /* Are we in moustache mode? */
    char moustacheBuffer[MOUSTACHE_BUFFER_SIZE];  /* Holds moustache
                                                   * contents. (NB: Not the
                                                   * contents of the file in
                                                   * the moustache!) */
    int moustacheIndex;  /* How deep are we into the moustache? */

    /* For recursion; this function calls itself to resolve nested
     * moustaches. */
    FILE* middleFile;
    char* middleDir;

    /* Reporting (holds errno values) */
    int error;

    /* Initialisation */
    moustacheIndex = 0;
    moustacheMode = 0;
    error = 0;
    previousChar = 0;
    thisChar = fgetc(inFile);  /* The first of many (hopefully). */

    /* Read until we hit EOF in the input file. */
    while (thisChar != EOF)
    {
        /* Are we currently in moustache mode? */
        if (moustacheMode == 1)
        {
            /* Has the moustache ended? */
            if (thisChar == MOUSTACHE_CLOSE_CODE &&
                previousChar == MOUSTACHE_CLOSE_CODE)
            {
                /* If so, exit moustache mode, and remove the stray curly brace
                 * from the buffer. */
                moustacheMode = 0;
                moustacheBuffer[moustacheIndex - 1] = 0;

                /* Open the contents of the next-level moustache. */
                middleFile = fopen(moustacheBuffer, "rb+");
                if (middleFile == NULL)
                {
                    fprintf(stderr, "Error opening nested file '%s': %s.\n",
                            moustacheBuffer, strerror(errno));
                    return errno;
                }

                /* Grab the directory. POSIX! */
                middleDir = dirname(moustacheBuffer);

                /* Recurse, propagating any errors. */
                error = template(middleFile, outFile, middleDir);
                fclose(middleFile);
                if (error != 0) return error;
            }

            /* Otherwise, store the character in the moustache buffer. */
            moustacheBuffer[moustacheIndex++] = thisChar;
        }

        else
        {
            /* Has a moustache started? */
            if (thisChar == MOUSTACHE_OPEN_CODE &&
                previousChar == MOUSTACHE_OPEN_CODE)
            {
                /* Enter moustache mode. */
                moustacheMode = 1;

                /* Clear the moustache buffer, putting the directory at the
                 * start. */
                for (moustacheIndex = 0;
                     moustacheIndex < MOUSTACHE_BUFFER_SIZE;
                     moustacheBuffer[moustacheIndex++] = 0);
                strcpy(moustacheBuffer, dir);
                strcat(moustacheBuffer, "/");
                moustacheIndex = strlen(dir) + 1;

                /* If so, remove the previous curly brace by moving the write
                 * pointer backwards by one. */
                fseek(outFile, -1, SEEK_CUR);
            }

            /* Otherwise, write the character to output file. */
            else
            {
                if (fputc(thisChar, outFile) == EOF)
                {
                    fprintf(stderr, "Error writing to output file.\n");
                    return errno;
                }
            }
        }

        /* Next loop setup. */
        previousChar = thisChar;
        thisChar = fgetc(inFile);
    }

    return 0; /* If we made it all the way here, we're pretty safe. */
}

#undef MOUSTACHE_OPEN_CODE
#undef MOUSTACHE_CLOSE_CODE
#undef MOUSTACHE_BUFFER_SIZE

#include "template.h"

#define MOUSTACHE_OPEN_CODE 123  /* '{', you'll need two of them. */
#define MOUSTACHE_CLOSE_CODE 125  /* '}', you'll need two of them. */
#define FILE_CODE 102  /* 'f' */
#define VALUE_CODE 118  /* 'v' */
#define CODE_DELIMITER 58  /* ':', used to delimit all input codes. */

/* Base size for the moustache buffer. The buffer itself is elastic - this is
 * an increment to make sure we don't realloc on every character read. */
#define MOUSTACHE_BUFFER_BASE 100

/* Simple wrapper around template to handle file opening and closing. */
int template_files(const char* inPath, const char* outPath,
                   char** valStrs, char** valVals, const unsigned numVals)
{
    /* File objects go here, one for each path provided. */
    FILE* inFile;
    FILE* outFile;

    char* inPathCopy;  /* For directory extraction */
    size_t inPathLength;
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
    inPathLength = strlen(inPath) + 1;
    inPathCopy = (char*) malloc(sizeof(char) * inPathLength);
    if (inPathCopy == NULL)
    {
        fprintf(stderr, "Error allocating memory for the input path '%s'.",
                inPath);
        return 1;
    }
    strlcpy(inPathCopy, inPath, inPathLength);
    dir = dirname(inPathCopy);

    /* Use templating engine. */
    rc = template(inFile, outFile, dir, valStrs, valVals, numVals);

    /* Cleanup. */
    free(inPathCopy);
    fclose(inFile);
    fclose(outFile);
    return rc;
}

/* This function performs templating on the file at "inFile", and writes it to
 * file at "outFile".
 *
 * Arguments:
 *
 * - inFile: Where to read from, open in read-binary mode.
 *
 * - outFile: Where to write to, open in write-binary mode.
 *
 * - dir: Null-terminated string describing the base directory for the input
 *   file.
 *
 * - valHandles: Array of null-terminated strings for user-defined value
 *   handles. Duplicate entries are ignored, but not removed.
 *
 * - valVals: Array of null-terminated strings containing the values for each
 *   handle in "valHandles".
 *
 * - numVals: The number of defined handles. The number of values is assumed
 *   equal to the number of handles.
 *
 * If there are no moustaches in the file at "inFile", the file is simply
 * copied to "outFile".
 *
 * You can even move the read/write pointers to parse certain areas of files -
 * this is exploited when recursing moustaches.
 *
 * Returns 0 if no errors occured, and errno or other non-zero otherwise. */
int template(FILE* inFile, FILE* outFile, const char* dir,
             char** valStrs, char** valVals, const unsigned numVals)
{
    /* Characters, read from inFile. */
    int thisChar;
    int previousChars[3];  /* It's a queue. */

    /* Moustache mode */
    char moustacheMode;  /* Which moustache mode are we in, if any? */
    size_t moustacheIndex;  /* How deep are we into the moustache? */

    /* For dealing with value moustaches. */
    char* valueHandle;
    size_t valueIndex;

    /* While in moustache mode, a buffer is needed to hold moustache
     * contents. This buffer grows and shrinks as necessary to hold moustache
     * contents. */
    char* moustacheBuffer;
    const size_t moustacheBaseSize = sizeof(char*) * (MOUSTACHE_BUFFER_BASE);
    size_t moustacheCurrentSize;

    /* For recursion; this function calls itself to resolve nested
     * moustaches. */
    FILE* middleFile;
    char* middleDir;
    int error;

    /* Initialisation */
    moustacheMode = 0;
    previousChars[0] = 0;
    previousChars[1] = 0;
    previousChars[2] = 0;
    thisChar = fgetc(inFile);  /* The first of many (hopefully). */

    /* Set up the moustache buffer. */
    moustacheCurrentSize = moustacheBaseSize;
    moustacheBuffer = (char*) malloc(strlen(dir) + 2 + moustacheBaseSize);
    if (moustacheBuffer == NULL)
    {
        fprintf(stderr, "Error allocating memory for a moustache. Exiting.\n");
        return 1;
    }
    for (moustacheIndex = 0; moustacheIndex < moustacheCurrentSize;
         moustacheBuffer[moustacheIndex++] = 0);

    /* Read until we hit EOF in the input file. */
    while (thisChar != EOF)
    {
        /* Are we currently in a moustache mode? */
        if (moustacheMode != 0)
        {
            /* Has the moustache ended? */
            if (thisChar == MOUSTACHE_CLOSE_CODE &&
                previousChars[2] == MOUSTACHE_CLOSE_CODE)
            {
                /* If so, remove the stray curly brace from the buffer. */
                moustacheBuffer[moustacheIndex - 1] = 0;
                fseek(outFile, -1, SEEK_CUR);

                /* Handle a file moustache. */
                if (moustacheMode == FILE_CODE)
                {
                    /* Open the contents of the next-level moustache. */
                    middleFile = fopen(moustacheBuffer, "rb+");
                    if (middleFile == NULL)
                    {
                        fprintf(stderr,
                                "Error opening nested file '%s': %s.\n",
                                moustacheBuffer, strerror(errno));
                        return errno;
                    }

                    /* Grab the directory. POSIX! */
                    middleDir = dirname(moustacheBuffer);

                    /* Recurse, propagating any errors. */
                    error = template(middleFile, outFile, middleDir,
                                     valStrs, valVals, numVals);
                    fclose(middleFile);
                    if (error != 0) return error;
                }

                /* Handle a value moustache. */
                else if (moustacheMode == VALUE_CODE)
                {
                    /* Figure out which value handle we've grabbed. */
                    valueHandle = NULL;
                    for (valueIndex = 0; valueIndex < numVals; valueIndex++)
                    {
                        if (strncmp(moustacheBuffer, valStrs[valueIndex],
                                    strlen(valStrs[valueIndex])) == 0)
                        {
                            valueHandle = valStrs[valueIndex];
                            break;
                        }
                    }

                    /* If it's not a defined handle, complain. */
                    if (valueHandle == NULL)
                    {
                        fprintf(stderr, "No value defined with handle '%s'. "
                                "Exiting.\n", moustacheBuffer);
                        return 1;
                    }

                    /* Place the mapped value into the output file. */
                    fflush(outFile);
                    fputs(valVals[valueIndex], outFile);
                    fflush(outFile);
                }

                /* We can't actually get here, because a moustache with an
                 * invalid code will just get ignored, but we'll be safe
                 * anyway. */
                else
                {
                    fprintf(stderr, "Invalid moustache code '%c'. Exiting.\n",
                            moustacheMode);
                    return 1;
                }

                /* Exit moustache mode. */
                moustacheMode = 0;
            }

            /* Otherwise, store the character in the moustache buffer. */
            if (moustacheIndex % moustacheCurrentSize == 0)
            {
                moustacheBuffer = \
                    realloc(moustacheBuffer,
                            moustacheBaseSize + moustacheCurrentSize);
                if (moustacheBuffer == NULL)
                {
                    fprintf(stderr, "Error expanding the moustache buffer. "
                            "Exiting.\n");
                    return 1;
                }
            }
            moustacheBuffer[moustacheIndex++] = thisChar;
        }

        else  /* We're not in a moustache mode, then. */
        {
            /* Has a file moustache started? */
            if (thisChar == CODE_DELIMITER &&
                previousChars[2] == FILE_CODE &&
                previousChars[1] == MOUSTACHE_OPEN_CODE &&
                previousChars[0] == MOUSTACHE_OPEN_CODE)
            {
                /* Enter moustache mode. */
                moustacheMode = FILE_CODE;

                /* Clear the moustache buffer, putting the directory at the
                 * start. Also resize the moustache buffer appropriately. */
                moustacheBuffer = realloc(moustacheBuffer,
                                          strlen(dir) + 2 + moustacheBaseSize);
                if (moustacheBuffer == NULL)
                {
                    fprintf(stderr, "Error compressing the moustache buffer. "
                                    "Exiting.\n");
                    return 1;
                }
                moustacheCurrentSize = moustacheBaseSize;
                for (moustacheIndex = 0; moustacheIndex < moustacheCurrentSize;
                     moustacheBuffer[moustacheIndex++] = 0);
                strlcpy(moustacheBuffer, dir, moustacheCurrentSize);
                strlcat(moustacheBuffer, "/", moustacheCurrentSize);
                moustacheIndex = strlen(dir) + 1;

                /* Remove the previous three characters ('{f:') by moving the
                 * write pointer backwards by three. */
                fseek(outFile, -3, SEEK_CUR);
            }

            /* Has a value moustache started? */
            if (thisChar == CODE_DELIMITER &&
                previousChars[2] == VALUE_CODE &&
                previousChars[1] == MOUSTACHE_OPEN_CODE &&
                previousChars[0] == MOUSTACHE_OPEN_CODE)
            {
                /* Enter moustache mode. */
                moustacheMode = VALUE_CODE;

                /* Clear and resize the moustache buffer. */
                moustacheBuffer = realloc(moustacheBuffer,
                                          strlen(dir) + 2 + moustacheBaseSize);
                if (moustacheBuffer == NULL)
                {
                    fprintf(stderr, "Error compressing the moustache buffer. "
                                    "Exiting.\n");
                    return 1;
                }
                moustacheCurrentSize = moustacheBaseSize;
                moustacheIndex = moustacheCurrentSize;
                do
                {
                    moustacheIndex--;
                    moustacheBuffer[moustacheIndex] = 0;
                }
                while (moustacheIndex > 0);

                /* Remove the previous three characters ('v:') by moving the
                 * write pointer backwards by three. */
                fseek(outFile, -2, SEEK_CUR);
            }

            /* Otherwise, write the character to output file. */
            else
            {
                if (fputc(thisChar, outFile) == EOF)
                {
                    fprintf(stderr, "Error writing to output file.\n");
                    return errno;
                }
                fflush(outFile);
            }
        }

        /* Next loop setup. */
        previousChars[0] = previousChars[1];
        previousChars[1] = previousChars[2];
        previousChars[2] = thisChar;
        thisChar = fgetc(inFile);
    }

    free(moustacheBuffer);
    return 0; /* If we made it all the way here, we're pretty safe. */
}

#undef MOUSTACHE_OPEN_CODE
#undef MOUSTACHE_CLOSE_CODE
#undef FILE_CODE
#undef CODE_DELIMITER
#undef MOUSTACHE_BUFFER_SIZE

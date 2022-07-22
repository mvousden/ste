#include "string_helpers.h"

/* These are probably very similar to the BSD version... */

/* Appends the null-terminated string `source` to the null-terminated string
 * `destination`, ensuring that the concatenated string doesn't exceed the
 * buffer. Only copies until either the buffer is (almost) full, or a null
 * terminator is reached. These strings must not overlap.
 *
 * Returns the initial length of destination, and the length of source. */
size_t ste_strlcat(char* const destination, const char* const source,
                   const size_t size)
{
    ptrdiff_t destLength;

    /* Find the breakpoint, where destination ends. */
    char* breakpoint = destination;
    if (*breakpoint != 0) while (breakpoint++[1] != 0);

    /* Return the length of the destination string, without the null
     * terminator (before the copy), added to the length of the source
     * string. */
    destLength = breakpoint - destination;
    return destLength +
        ste_strlcpy(breakpoint, source, size - destLength);
}

/* Copies characters from the `source` string to the `destination` string,
 * ensuring a null-terminated copy that doesn't exceed the buffer. Only copies
 * until either the buffer is (almost) full, or a null terminator is
 * reached. These strings must not overlap.
 *
 * Returns the length of source. */
size_t ste_strlcpy(char* const destination, const char* const source,
                   const size_t size)
{
    const char* end = destination + size;  /* Convenience */

    /* These will move */
    char* target = destination;
    const char* thisChar = source;

    /* Copy characters into target, sensibly. */
    while (*thisChar != 0 && target < end) *target++ = *thisChar++;

    /* Tack on a /0, sensibly. */
    if (target < end) *target = 0;
    else if (size > 0) target[-1] = 0;  /* Handles the size=0 case. */

    /* Return the length of source, by taking our pointer to the end of the
     * source string and doing some pointer arith. Cheaper than doing a strlen,
     * since we're already part of the way through. */
    while (*thisChar != 0) thisChar++;
    return thisChar - source;
}

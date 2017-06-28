#ifndef _WORKER_H
#define _WORKER_H

#include <stdio.h>

/** Do work for request.  request is assumed to contain the following
 *  fields with each field terminated by a single NUL '\0' character:
 *
 *   nArgs:      # of arguments which follow (not including itself).
 *   moduleName: name of the module containing is-word-char definition.
 *   maxNWords:  max number of words to be output.
 *   fileNames:  file-names (as many as left-over from nArgs value).
 *
 * Up to maxNWords are written to out in the following format:
 *    A word count
 *    A single space
 *    A NUL-terminated word.
 * If an error is detected, the error is written to out preceeded
 * by a negative count.
 */
void do_work(const char *request, FILE *out);

#endif //ifndef _WORKER_H

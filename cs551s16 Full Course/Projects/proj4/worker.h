#ifndef _WORKER_H
#define _WORKER_H

#include "data-buffer.h"

#include <stddef.h>
#include <stdio.h>

struct Worker;

/** Create worker for request.  request is assumed to contain the following
 *  fields with each field terminated by a single NUL '\0' character:
 *
 *   nArgs:      # of arguments which follow (not including itself).
 *   maxNWords:  max number of words to be output.
 *   fileNames:  file-names (as many as left-over from nArgs value).
 *
 * If an error is detected, the error is written to error and a NULL
 * is returned.
 */
struct Worker *new_worker(const char *request, struct DataBuffer *error);

/** Free all resources used by worker. */
void free_worker(struct Worker *worker);

/** Write chars of response from worker into dataBuffer at start of
 *  buffer.  Returns # of chars written.  Returns 0 if no more
 *  response.
 */
size_t next_response(struct Worker *worker, struct DataBuffer *dataBuffer);

#endif //ifndef _WORKER_H

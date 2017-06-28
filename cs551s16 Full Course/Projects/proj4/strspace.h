#ifndef _STRSPACE_H
#define _STRSPACE_H

/** Provide permanent storage for intern'd strings */

typedef struct StrSpaceImpl StrSpace;

/** Create a new string-space */
StrSpace *new_strspace(void);

/** Free all resources used by strspace. */
void free_strspace(StrSpace *strspace);

/** Intern NUL-terminated string in strspace and return pointer to
 *  intern'd value.
 */
const char *intern_strspace(StrSpace *strspace, const char *string);

#endif //ifndef _STRSPACE_H

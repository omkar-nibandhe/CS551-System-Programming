#ifndef _ERRORS_H
#define _ERRORS_H

/* Print a error message on stderr as per printf-style fmt and
 * optional arguments.  If fmt ends with ':', follow with
 * a space, strerror(errno), followed by a newline.
 */
void error(const char *fmt, ...);

/* Print a error message on stderr as per printf-style fmt and
 * optional arguments.  If fmt ends with ':', follow with a space,
 * strerror(errno), followed by a newline.  Terminate the program.
 */
void fatal(const char *fmt, ...) __attribute__ ((noreturn));

/* Return # of errors seen so far. */
int getErrorCount(void);

#endif //ifndef _ERRORS_H

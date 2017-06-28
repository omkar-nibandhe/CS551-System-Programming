#ifndef _UNIT_TEST_H
#define _UNIT_TEST_H

#include <stdio.h>

/** Module for unit test.  Call UNIT_TEST(expr) to verify that expr
 *  is true.
 */

/** Implementation function called by UNIT_TEST() macro; do not call
 *  directly
 */
void unit_test(const char *fileName, int lineNumber, const char *functionName,
               int exprValue, const char *expr);

/** Return # of unit tests performed so far */
int num_total_unit_test(void);

/** Return # of unit test failures encountered so far */
int num_fail_unit_test(void);

/** If there are unit test failures, print a report on out. */
void report_unit_test(FILE *out);

/** Reset unit-test counts */
void reset_unit_test(void);

#define UNIT_TEST(expr)                         \
  do { \
    unit_test(__FILE__, __LINE__, __func__, (expr), #expr);     \
  } while(0)

#endif //ifndef _UNIT_TEST_H

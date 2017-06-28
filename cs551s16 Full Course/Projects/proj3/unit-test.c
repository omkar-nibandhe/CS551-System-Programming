#include "unit-test.h"

#include <stdio.h>


/** Module for unit test.  Call UNIT_TEST(expr) to verify that expr
 *  is true.
 */

static int num_tests = 0;
static int num_failures = 0;

/** Implementation function called by UNIT_TEST() macro; do not call
 *  directly
 */
void
unit_test(const char *fileName, int lineNumber, const char *functionName,
          int exprValue, const char *expr)
{
  if (!exprValue) {
    fprintf(stderr, "%s:%d: %s(): %s failed\n", fileName, lineNumber,
            functionName, expr);
    num_failures++;
  }
  num_tests++;
}

/** Return # of unit tests performed so far */
int num_total_unit_test(void) { return num_tests; }

/** Return # of unit test failures encountered so far */
int num_fail_unit_test(void) { return num_failures; }

/** If there are unit test failures, print a report on out. */
void report_unit_test(FILE *out) {
  if (num_failures > 0) {
    fprintf(out, "%d/%d failures\n", num_failures, num_tests);
  }
}

/** Reset unit-test counts */
void reset_unit_test(void) { num_tests = num_failures = 0; }

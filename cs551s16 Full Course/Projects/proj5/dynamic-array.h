#ifndef _DYNAMIC_ARRAY_H
#define _DYNAMIC_ARRAY_H

#include "arrayi.h"
#include "compare-fn.h"

#include <stddef.h>


/** Create a new dynamic array for elements having size elementSize,
 *  with default size parameters.  If optionsP is NULL, then there
 *  is no limit on array size and no compare function.
 */
ArrayI *new_dynamic_array(size_t elementSize, const ArrayOptions *optionsP);

/** Create a new dynamic array for elements having size elementSize,
 *  with each having size 2**log2RowSize and number of rowInc rows added
 *  whenever number of rows is sufficient.  If optionsP is NULL, then there
 *  is no limit on array size and no compare function.
 */
ArrayI *
new_sized_dynamic_array(size_t elementSize, size_t log2RowSize, size_t rowInc,
                        const ArrayOptions *optionsP);

#endif //ifndef _DYNAMIC_ARRAY_H

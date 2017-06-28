#ifndef _INTERFACES_H
#define _INTERFACES_H

/** An interface object is exposed as a pointer to a struct, the first
 *  field of which is a FUNCTIONS_I pointer to a struct of
 *  function pointers for the functions in that interface.  It is
 *  intended that the struct of function pointers be statically
 *  setup in the code which implements the interface.  The rest
 *  of the interface object will contain the object state but
 *  that is totally hidden from the interface clients.
 *
 *  This file hides the details calling of an interface function
 *  within the CALL_I() macro.
 */

#define FUNCTIONS_I fns

//Uses gcc ## feature to take care of trailing comma problem when no
//args are provided for ... .
//If this proves to be a problem, then define a special case
//CALL_I0 macro which does not used __VA_ARGS__
#define CALL_I(fnName, objectI, ...) \
  (objectI)->FUNCTIONS_I->fnName(objectI, ##__VA_ARGS__)

#endif //ifndef _INTERFACES_H

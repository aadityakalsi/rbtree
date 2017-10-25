/*! rbt.cpp */

#include "defs.h"

#include <rbtree/rbtree.hpp>

#ifdef RBTREE_C_API
#  define VER_DEF_FOUND 1
#else
#  define VER_DEF_FOUND 0
#endif

void api_macro(void)
{
    testThat(1 == VER_DEF_FOUND);
}

setupSuite(exports)
{
    addTest(api_macro);
}

/////////////////////////////////////////////////////////////////////////////////////

using namespace containers;

void rbt0(void)
{
    rbtree<int> t;
}

setupSuite(rbt)
{
    addTest(rbt0);
}

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
    testThat(sizeof(t) == 4*sizeof(void*));
    testThat(t.size() == 0);
    testThat(t.empty());
}

void rbt1(void)
{
    rbtree<int> t;
    // queries
    testThat(t.contains(0) == false);
    testThat(t.contains(-1) == false);
    testThat(t.contains(1) == false);
    // add
    testThat(t.insert(0) == true);
    testThat(t.size() == 1);
    testThat(t.contains(0) == true);
    testThat(t.contains(-1) == false);
    testThat(t.contains(1) == false);
    // add
    testThat(t.insert(-1) == true);
    testThat(t.size() == 2);
    testThat(t.contains(0) == true);
    testThat(t.contains(-1) == true);
    testThat(t.contains(1) == false);
    // add
    testThat(t.insert(1) == true);
    testThat(t.size() == 3);
    testThat(t.contains(0) == true);
    testThat(t.contains(-1) == true);
    testThat(t.contains(1) == true);
    // add-repeat
    testThat(t.insert(0) == false);
    testThat(t.size() == 3);
    testThat(t.contains(0) == true);
    testThat(t.contains(-1) == true);
    testThat(t.contains(1) == true);
    // add-repeat
    testThat(t.insert(-1) == false);
    testThat(t.size() == 3);
    testThat(t.contains(0) == true);
    testThat(t.contains(-1) == true);
    testThat(t.contains(1) == true);
    // add-repeat
    testThat(t.insert(1) == false);
    testThat(t.size() == 3);
    testThat(t.contains(0) == true);
    testThat(t.contains(-1) == true);
    testThat(t.contains(1) == true);
}

setupSuite(rbt)
{
    addTest(rbt0);
    addTest(rbt1);
}

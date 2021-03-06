/*! rbt.cpp */

#include "defs.h"

#include <rbtree/rbtree.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <set>

#include <string>
#include <vector>

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
    testThat(sizeof(t) == 3*sizeof(void*));
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
    // clear
    t.clear();
    testThat(t.size() == 0);
    testThat(t.empty());
}

void rbt2(void)
{
    rbtree<int> t;
    const int N = 1000;
    for (int i = 0; i < N; ++i) {
        testThat(t.contains(i) == false);
        testThat(t.size() == i);
        testThat(t.insert(i) == true);
        testThat(t.size() == (i+1));
        testThat(t.contains(i) == true);
        for (int j = 0; j < N; ++j) {
            testThat(t.contains(j) == (j <= i));
        }
    }
}

namespace {

template<class T>
void print_time_taken(T a, T b)
{
    auto diff = b - a;
    std::cout << std::chrono::duration<double, std::milli>(diff).count() << " : ";
}

#ifdef NDEBUG
const size_t PERFN_DEFL = 10000;
#else
const size_t PERFN_DEFL = 1000;
#endif

const size_t PERFN = std::getenv("N") ? std::atoi(std::getenv("N")) : PERFN_DEFL;

} // namespace

void rbt3_time_int(void)
{
    rbtree<int> t;
    const int N = PERFN;
    auto a = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        testThat(t.contains(i) == false);
        testThat(t.size() == i);
        testThat(t.insert(i) == true);
        testThat(t.size() == (i+1));
        testThat(t.contains(i) == true);
    }
    auto b = std::chrono::high_resolution_clock::now();
    // t.print();
    std::cout << "rbtree<int>: ";
    print_time_taken(a, b);
}

void stdset3_time_int(void)
{
    std::set<int> t;
    const int N = PERFN;
    auto end = t.end();
    auto a = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        testThat(t.find(i) == end);
        testThat(t.size() == i);
        testThat(t.insert(i).second == true);
        testThat(t.size() == (i+1));
        testThat(t.find(i) != end);
    }
    auto b = std::chrono::high_resolution_clock::now();
    std::cout << "std::set<int>: ";
    print_time_taken(a, b);
}

void rbt3_time_size(void)
{
    rbtree<size_t> t;
    const size_t N = PERFN;
    auto a = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < N; ++i) {
        testThat(t.contains(i) == false);
        testThat(t.size() == i);
        testThat(t.insert(i) == true);
        testThat(t.size() == (i+1));
        testThat(t.contains(i) == true);
    }
    auto b = std::chrono::high_resolution_clock::now();
    // t.print();
    std::cout << "rbtree<size_t>: ";
    print_time_taken(a, b);
}

void stdset3_time_size(void)
{
    std::set<size_t> t;
    const size_t N = PERFN;
    auto end = t.end();
    auto a = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < N; ++i) {
        testThat(t.find(i) == end);
        testThat(t.size() == i);
        testThat(t.insert(i).second == true);
        testThat(t.size() == (i+1));
        testThat(t.find(i) != end);
    }
    auto b = std::chrono::high_resolution_clock::now();
    std::cout << "std::set<size_t>: ";
    print_time_taken(a, b);
}

void rbt3_time_string_move(void)
{
    rbtree<std::string> t;
    const size_t N = PERFN;
    std::vector<std::string> strs;
    for (size_t i = 0; i < N; ++i) {
        strs.push_back(std::to_string(i));
    }
    auto cp = strs;
    auto a = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < N; ++i) {
        testThat(t.contains(strs[i]) == false);
        testThat(t.size() == i);
        testThat(t.insert(std::move(strs[i])) == true);
        testThat(t.size() == (i+1));
        testThat(t.contains(cp[i]) == true);
    }
    auto b = std::chrono::high_resolution_clock::now();
    // t.print();
    std::cout << "rbtree<std::string>: ";
    print_time_taken(a, b);
}

void stdset3_time_string_move(void)
{
    std::set<std::string> t;
    const size_t N = PERFN;
    std::vector<std::string> strs;
    for (size_t i = 0; i < N; ++i) {
        strs.push_back(std::to_string(i));
    }
    auto cp = strs;
    auto a = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < N; ++i) {
        testThat(t.find(strs[i]) == t.end());
        testThat(t.size() == i);
        testThat(t.insert(std::move(strs[i])).second == true);
        testThat(t.size() == (i+1));
        testThat(t.find(cp[i]) != t.end());
    }
    auto b = std::chrono::high_resolution_clock::now();
    // t.print();
    std::cout << "std::set<std::string>: ";
    print_time_taken(a, b);
}

//////////////////////////////////////////

setupSuite(rbt)
{
    addTest(rbt0);
    addTest(rbt1);
    addTest(rbt2);
    addTest(stdset3_time_int);
    addTest(rbt3_time_int);
    addTest(stdset3_time_size);
    addTest(rbt3_time_size);
    addTest(stdset3_time_string_move);
    addTest(rbt3_time_string_move);
}

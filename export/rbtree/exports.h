/*! exports.h */

#ifndef _RBTREE_VERSION_H_
#define _RBTREE_VERSION_H_

#if defined(USE_RBTREE_STATIC)
#  define RBTREE_API
#elif defined(_WIN32) && !defined(__GCC__)
#  ifdef BUILDING_RBTREE_SHARED
#    define RBTREE_API __declspec(dllexport)
#  else
#    define RBTREE_API __declspec(dllimport)
#  endif
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#else
#  ifdef BUILDING_RBTREE_SHARED
#    define RBTREE_API __attribute__ ((visibility ("default")))
#  else
#    define RBTREE_API 
#  endif
#endif

#if defined(__cplusplus)
#  define RBTREE_EXTERN_C extern "C"
#  define RBTREE_C_API RBTREE_EXTERN_C RBTREE_API
#else
#  define RBTREE_EXTERN_C
#  define RBTREE_C_API RBTREE_API
#endif

#endif/*_RBTREE_VERSION_H_*/

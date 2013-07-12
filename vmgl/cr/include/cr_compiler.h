/**
 * Compiler-related definitions, etc.
 */

#ifndef CR_COMPILER_H
#define CR_COMPILER_H 1


/**
 * Function inlining
 */
#if defined(__GNUC__)
#  define INLINE __inline__
#elif defined(__MSC__)
#  define INLINE __inline
#elif defined(_MSC_VER)
#  define INLINE __inline
#elif defined(__ICL)
#  define INLINE __inline
#else
#  define INLINE
#endif


/**
 * For global vars in shared libs
 */
#define DLLDATA


/**
 * For functions called via the public API.
 * XXX CR_APIENTRY could probably replace all the other *_APIENTRY defines.
 */
#define CR_APIENTRY


#endif /* CR_COMPILER_H */

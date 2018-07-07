/*
 * $Source: r:/source/lib/RCS/defines.h,v $
 * $Revision: 1.4 $
 * $Date: 1999/05/09 23:17:23 $
 * $Locker:  $
 *
 *	Nice shortcuts
 *
 * $Log: defines.h,v $
 * Revision 1.4  1999/05/09 23:17:23  vitus
 * - added MIN(), MAX()
 *
 * Revision 1.3  1998/10/30 01:27:57  vitus
 * - UNREFERENCED() for non-EMX compilers, too (empty)
 *
 * Revision 1.2  1997/09/22 23:20:05  vitus
 * define PRIVATE to empty string in non-debug versions
 *
 * Revision 1.1  1996/09/02 22:08:19  vitus
 * Initial revision
 */


#if !defined(PRIVATE)
# if !defined(NDEBUG)
#  define PRIVATE
#  define PUBLIC
# else
#  define PRIVATE	static
#  define PUBLIC
# endif
#endif


#if defined(__GNUC__)
# define UNREFERENCED(var) static void *const use_##var = (&use_##var, &var, 0)
# define USE_OS2_TOOLKIT_HEADERS

#else
# define UNREFERENCED(var)
#endif


#if !defined(MIN)
# define MIN(a,b)	((a)>(b)?(b):(a))
#endif

#if !defined(MAX)
# define MAX(a,b)	((a)>(b)?(a):(b))
#endif

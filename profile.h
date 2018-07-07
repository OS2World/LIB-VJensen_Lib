/*
 * $Source: r:/source/lib/RCS/profile.h,v $
 * $Revision: 1.3 $
 * $Date: 2000/05/14 01:26:05 $
 * $Locker:  $
 *
 * 	Interface to profile.c
 *
 * $Log: profile.h,v $
 * Revision 1.3  2000/05/14 01:26:05  vitus
 * - added ProfileReadNext()
 *
 * Revision 1.2  1997/03/23 15:36:48  vitus
 * Added ProfileDelete()
 * Changed ProfileRead()
 *
 * Revision 1.1  1996/03/17 17:31:10  vitus
 * Initial revision
 * ----------------------------------------
 * This code is given to the Public Domain.
 */

extern ULONG	ProfileOpen(HAB const hab,char const * const filename);

extern ULONG	ProfileClose(HAB const hab);

extern ULONG	ProfileWrite(char const * const app,
			     char const * const key,
			     unsigned const bufsiz,
			     void const * const buf);

extern ULONG	ProfileRead(char const * const appname,
			    char const * const key,
			    unsigned * const size,
			    void * const buf);

extern ULONG	ProfileReadNext(char const * const appname,
				char * const key,
				unsigned * const size,
				void * const buf);

extern ULONG	ProfileDelete(char const * const app,
			      char const * const key);

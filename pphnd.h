/*
 * $Source: r:/source/lib/RCS/pphnd.h,v $
 * $Revision: 1.2 $
 * $Date: 2000/05/15 00:58:50 $
 * $Locker:  $
 *
 *	Interface to pphnd.c
 *	Presentation Parameter Handling
 *
 * $Log: pphnd.h,v $
 * Revision 1.2  2000/05/15 00:58:50  vitus
 * - PPSubclassWindow: removed 'data' parameters and added 'title' which is
 *   used as app key when writing to the profile.
 *
 * Revision 1.1  2000/05/14 01:25:23  vitus
 * Initial revision
 * ----------------------------------------
 * This code is given to the Public Domain.
 */



extern BOOL	PPSubclassWindow(HWND hwnd, PCSZ title);

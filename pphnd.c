/*
 * $Source: r:/source/lib/RCS/pphnd.c,v $
 * $Revision: 1.3 $
 * $Date: 2000/05/15 00:57:49 $
 * $Locker:  $
 *
 *	Generalized handling of presentation parameter
 *	changes.
 *
 * $Log: pphnd.c,v $
 * Revision 1.3  2000/05/15 00:57:49  vitus
 * - changed storage to use a given app key and the window identifier as subkey.
 *   All presentation parameters are now saved combined in that storage.
 *
 * Revision 1.2  2000/05/14 01:57:34  vitus
 * - commented
 *
 * Revision 1.1  2000/05/14 01:24:55  vitus
 * Initial revision
 * ----------------------------------------
 * This code is given to the Public Domain.
 */
static char const vcid[]="$Id: pphnd.c,v 1.3 2000/05/15 00:57:49 vitus Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_WIN
#include <os2.h>

#include "defines.h"
#include "verbose.h"
#include "profile.h"				/* profile handling */

#include "pphnd.h"



#define MAX_ATTR_SIZE	260			/* for single presparam */
#define MAX_SAVED_PRESP	32768



typedef struct _SUBCLASSINFO {
    PFNWP	pfnOldWp;
    HWND	hwnd;
    PCSZ	apptitle;
} SUBCLASSINFO, * PSUBCLASSINFO;





#define ADIFF(a,b)	((PUCHAR)(a)-(PUCHAR)(b))




/*# ----------------------------------------------------------------------
 * AdjustPresParam(hwnd, PresParam, data, datalen)
 *
 * PARAMETER
 *	hwnd			affected window
 *	PresParam		pres parameter changed
 *	data, datalen		well, ???
 *
 * RETURNS
 *	TRUE			all OK
 *	FALSE			some error occured (see WinGetLastError)
 *
 * DESCRIPTION
 *	This funcktion should be called whenever a presentation parameter
 *	was changed.  It will query the changed paramter and store it
 *	in the profile.
 *
 * REMARKS
 */
PRIVATE BOOL
AdjustPresParam(HWND hwnd, PCSZ const title, ULONG PresParam)
{
    BOOL	result = FALSE;
    PUCHAR const	attr = malloc(MAX_ATTR_SIZE);
    PPRESPARAMS const	saved = malloc(MAX_SAVED_PRESP);

    do
    {
	ULONG	cb, ul, rc;
	unsigned u;
	char	key[9];

	if( attr == NULL  ||  saved == NULL )
	    break;

	cb = WinQueryPresParam(hwnd, PresParam, 0, NULL,
			       MAX_ATTR_SIZE, attr, QPF_NOINHERIT);
	if( cb == 0 )
	    break;

	ul = WinQueryWindowUShort(hwnd, QWS_ID);
	sprintf(key, "%lX", ul);

	u = MAX_SAVED_PRESP;
	rc = ProfileRead(title, key, &u, saved);
	if( rc != 0 )
	{
	    /* No presentation parameters so far. */

	    saved->cb = ADIFF(saved->aparam[0].ab,&saved->aparam[0]) + cb;
	    saved->aparam[0].id = PresParam;
	    saved->aparam[0].cb = cb;
	    memcpy(saved->aparam[0].ab, attr, cb);
	}
	else
	{
	    PPARAM	p = &saved->aparam[0];
	    BOOL	found = FALSE;

	    while( ADIFF(p,&saved->aparam[0]) < saved->cb )
	    {
		if( p->id == PresParam )
		{
		    if( p->cb == cb  &&  memcmp(p->ab, attr, cb) == 0 )
		    {
			found = TRUE;		/* not changed: nothing to do */
		    }
		    else
		    {
			/* There is already a record for 'PresParam'.  Replace
			 * record data with new value. */

			if( p->cb != cb )
			{
			    memmove(&p->ab[cb],
				    &p->ab[p->cb],
				    saved->cb
				    - ADIFF(&p->ab[p->cb],&saved->aparam[0]));
			    saved->cb += cb - p->cb;
			    p->cb = cb;
			}
			memcpy(p->ab, attr, cb);
		    }
		    break;
		}
		p = (PPARAM)&p->ab[p->cb];
	    }
	    if( !found )
	    {
		/* 'p' already points to a free record behind all others. */

		p->id = PresParam;
		p->cb = cb;
		memcpy(p->ab, attr, cb);
		saved->cb += 2 * sizeof(ULONG) + cb;
	    }
	}
	ProfileWrite(title, key, saved->cb+sizeof(ULONG), saved);
	result = TRUE;
    }
    while( 0 );

    free(saved);
    free(attr);
    return result;
}




/*# ----------------------------------------------------------------------
 * PPSubclassProc(hwnd, msg, mp1, mp2)
 *
 * PARAMETER
 *	hwnd,msg,mp1,mp2	same as subclassed window proc
 *
 * RETURNS
 *	result of subclassed window proc
 *
 * DESCRIPTION
 *	This is the window procedure used to subclass a window.  It
 *	will handle WM_PRESPARAMCHANGED (to save the value) and
 *	WM_DESTROY (to deinstall), but call the previous window
 *	procedure for any real processing.
 *
 * REMARKS
 */
PRIVATE MRESULT EXPENTRY
PPSubclassProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PSUBCLASSINFO   sInfo = (PSUBCLASSINFO)WinQueryWindowULong(hwnd, QWL_USER);
    PFNWP const     pfnOldProc = sInfo->pfnOldWp;

    if( msg == WM_PRESPARAMCHANGED )
	AdjustPresParam(hwnd, sInfo->apptitle, (ULONG)mp1);
    else if( msg == WM_DESTROY )
    {
	free(sInfo); 
	WinSubclassWindow(hwnd, pfnOldProc);
    }
    return pfnOldProc(hwnd, msg, mp1, mp2);
}




/*# ----------------------------------------------------------------------
 * PPSubclassWindow(hwnd, datalen, data)
 *
 * PARAMETER
 *	hwnd		window to subclass
 *	datalen,data	still no idea what this could be used for
 *
 * RETURNS
 *	TRUE		OK, subclassed the window
 *	FALSE		some error occurred (see WinGetLastError)
 *
 * DESCRIPTION
 *	This routine restores all stored presentation parameters
 *	and subclasses 'hwnd' afterwards to get hold of all further
 *	changes.
 *
 * REMARKS
 */
PUBLIC BOOL
PPSubclassWindow(HWND hwnd, PCSZ title)
{
    BOOL		result = FALSE;
    PSUBCLASSINFO const	NewRec = (PSUBCLASSINFO)calloc(1, sizeof(SUBCLASSINFO));
    ULONG const		ul = WinQueryWindowUShort(hwnd, QWS_ID);
    unsigned		u;
    PPRESPARAMS		saved = NULL;
    char		key[9];
    PPARAM		p;
    ULONG		rc;


    do
    {
	if( NewRec == NULL )
	    break;				/* we have problems... */

	if( hwnd == NULLHANDLE )
	{
	    free(NewRec);
	    break;				/* caller has problems... */
	}


	NewRec->hwnd = hwnd;
	NewRec->apptitle = title;
	result = TRUE;


	/* Apply all saved presentation parameters to window. */

	sprintf(key, "%lx", ul);

	u = MAX_SAVED_PRESP;
	saved = malloc(u);
	if( saved == NULL )
	    break;

	rc = ProfileRead(title, key, &u, saved);
	if( rc != 0 )
	    break;

	p = &saved->aparam[0];
	while( ADIFF(p,&saved->aparam[0]) < saved->cb )
	{
	    WinSetPresParam(hwnd, p->id, p->cb, p->ab);
	    p = (PPARAM)&p->ab[p->cb];
	}
    }
    while( 0 );
    free(saved);

    if( result == TRUE )
    {
	WinSetWindowULong(hwnd, QWL_USER, (ULONG)NewRec);
	NewRec->pfnOldWp = WinSubclassWindow(hwnd, PPSubclassProc);
    }
    return result;
}

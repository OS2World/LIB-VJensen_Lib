/*
 * $Source: r:/source/lib/RCS/profile.c,v $
 * $Revision: 2.1 $
 * $Date: 2000/05/14 01:25:51 $
 * $Locker:  $
 *
 *	Function to handle all profile settings.
 *
 * Idea:
 * - Program settings are stored in a program related ini-file seperate
 *	from the system ini's.
 * - All settings which should be recorded in the ini-file should
 *	be stored immediately when occuring. Ie: window size changes
 *	stored when window resizes (during W_SIZE) not when application
 *	terminates.
 * - It's no idea to access the ini-file each time this happens. The
 *	ini-file should be written when the application is said to
 *	do so (during WM_SAVEAPPLICATION).
 *
 * Solution:
 *	- Build here a cache to save the changes and flush this cache
 *	  during WM_SAVEAPPLICATION.
 * 	- Work with the open, close, read, write model.
 * 	- Allow only a single ini-file (to skip handles).
 * 	- There is only one generic data type.
 *
 * $Log: profile.c,v $
 * Revision 2.1  2000/05/14 01:25:51  vitus
 * - implemented ProfileReadNext()
 *
 * Revision 1.4  2000/04/20 23:45:59  vitus
 * - changed Verbose() calls to new format
 *
 * Revision 1.3  1999/06/20 17:13:56  vitus
 * - minor corrections to keep compiler happy.  Well, not all warnings are
 *   gone.  But it's getting better...
 *
 * Revision 1.2  1997/03/23 16:09:47  vitus
 * Implemented ProfileDelete()
 * Changed ProfileRead() - returns size
 * Commented
 *
 * Revision 1.1  1996/03/17 17:31:32  vitus
 * Initial revision
 * ----------------------------------------
 * This code is given to the Public Domain.
 */
static char vcid[]="$Id: profile.c,v 2.1 2000/05/14 01:25:51 vitus Exp $";


#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define INCL_WIN
#define INCL_DOS
#include <os2.h>

#include "defines.h"
#include "verbose.h"
#include "profile.h"



#define	MAX_VALUE	8192			/* of any key */


typedef struct _SETTING {
    struct _SETTING	*next;
    int			 changed;		/* since last write */
    char		*app;
    char		*key;
    unsigned		 len;
    void		*data;
} SETTING;




PRIVATE HMTX	 hmtxSetting =	0;
PRIVATE SETTING *pSetting =	NULL;
PRIVATE char	*pszIniFile =	NULL;




/* **********************************************************************
 * Local routines
 * ******************************************************************* */



/*# ----------------------------------------------------------------------
 * CALL
 *	YankSetting(hab,hd,p)
 * PARAMETER
 *	hab		anchor block
 *	hd		handle of INI files
 *	p		setting to save
 * RETURNS
 *	0		OK
 * GLOBAL
 *	none
 * DESPRIPTION
 *	Flushes all changed settings in tree to disk and
 *	deletes memory structures.
 * REMARKS
 *	Recursion!
 */
PRIVATE ULONG
YankSetting(HAB const hab,HINI const hd,SETTING * const p)
{
    assert( hd != 0 );
    assert( p != NULL );

    if( p->next != NULL )
	YankSetting(hab, hd, p->next);

    if( p->changed != 0 )
    {
	BOOL bool;

	bool = PrfWriteProfileData(hd, p->app, p->key, p->data, p->len);
	if( bool == FALSE )
	    Verbose(1, "Profile", "PrfWriteProfileData fails - error %#x",
		    WinGetLastError(hab));
    }

    free(p->app), free(p->key), free(p->data);
    free(p);
    return 0;
}




/*# ----------------------------------------------------------------------
 * CALL
 *	NextWord(cp)
 * PARAMETER
 *	cp	pointer to list
 * RETURNS
 *	pointer to next word
 * GLOBAL
 *	none
 * DESPRIPTION
 *	Returns the next word out of a '\0' seperated list. If the
 *	returned pointer points to a '\0' the list has edded.
 * REMARKS
 *	See structure of environment.
 */
PRIVATE char *
NextWord(char *cp)
{
    for(; *cp != '\0'; ++cp )
	;
    return ++cp;
}






/* **********************************************************************
 * **** Exported routines ***********************************************
 * ******************************************************************* */


/*# ----------------------------------------------------------------------
 * CALL
 *	ProfileOpen(hab,filename)
 * PARAMETER
 *	hab		anchor block
 *	filename	name of INI file
 * RETURNS
 *	0		OK
 *	/0		failed
 * GLOBAL
 *	pSetting
 * DESPRIPTION
 *	Initialize access to profile by reading the whole
 *	contents to memory.
 * REMARKS
 *	Should this routine test 'filename' for correctness?
 *	Available directory?
 */
PUBLIC ULONG
ProfileOpen(HAB const hab,char const * const filename)
{
    APIRET rc;

    assert( filename != NULL );
    if( pszIniFile != NULL )
    {
	Verbose(1, "Profile", "OpenProfile: %s already open", pszIniFile);
	return (ULONG)-2;			/* only _one_ ini-file! */
    }
    pszIniFile = strdup( filename );
    if( pszIniFile == NULL )
    {
	Verbose(1, "Profile", "OpenProfile: memory problem");
	return (ULONG)-1;
    }

    rc = DosCreateMutexSem( NULL, &hmtxSetting, 0, FALSE );

    {
	HINI const hini = PrfOpenProfile( hab, (PSZ)filename );
	BOOL	bool;
	char	applist[8192];			/* name buffer */
	ULONG	appcnt = sizeof(applist);

	bool = PrfQueryProfileData( hini, NULL, NULL, applist, &appcnt );
	if( bool == TRUE )			/* OK? */
	{
	    char * ap = &applist[0];
	    for(; *ap != '\0'; ap = NextWord(ap) )
	    {
		char	keylist[8192];
		ULONG	keycnt = sizeof(keylist);

		bool = PrfQueryProfileData( hini, ap, NULL, keylist, &keycnt );
		if( bool == TRUE )
		{
		    char *kp = &keylist[0];
		    for(; *kp != '\0'; kp = NextWord(kp) )
		    {
			char  value[MAX_VALUE];
			ULONG size = sizeof(value);

			bool = PrfQueryProfileData( hini, ap, kp, value, &size );
			if( bool == TRUE )
			{
			    SETTING * p = malloc( sizeof(SETTING) );
			    memset( p, 0, sizeof(SETTING) );
			    p->app = strdup(ap);
			    p->key = strdup(kp);
			    p->data = malloc(size);
			    p->len = size;
			    memcpy( p->data, value, size );
			    p->changed = 0;
			    p->next = pSetting;
			    pSetting = p;
			}
		    }
		}
	    }
	}
	PrfCloseProfile( hini );
    }

    return rc;
}




/*# ----------------------------------------------------------------------
 * CALL
 *	ProfileClose(hab)
 * PARAMETER
 *	hab		anchor block
 * RETURNS
 *	0		OK
 *	/0		failed
 * GLOBAL
 *	pszIniFile
 * DESPRIPTION
 *	Flushes all changes to disk, frees resources.
 * REMARKS
 */
PUBLIC ULONG
ProfileClose(HAB const hab)
{
    SETTING	*p = pSetting;
    HINI const	hd = PrfOpenProfile( hab, pszIniFile );
    BOOL	bool;

    if( hd == NULLHANDLE )
    {
	ULONG error = WinGetLastError( hab );
	Verbose(1, "Profile", "PrfOpenProfile fails - error %#x", error);
	return error;
    }

    /* Write all saved data to disk */

    if( p != NULL )
    {
	YankSetting( hab, hd, p );
	p = NULL;
    }

    /* Close ini-file */

    bool = PrfCloseProfile( hd );
    free( pszIniFile ),	pszIniFile = NULL;
    DosCloseMutexSem( hmtxSetting ),	hmtxSetting = 0;

    if( bool == FALSE )
    {
	ULONG error = WinGetLastError( hab );
	Verbose(1, "Profile", "PrfCloseProfile fails - error %#x", error);
	return error;
    }
    return 0;
}




/*# ----------------------------------------------------------------------
 * CALL
 *	ProfileWrite(app,key,bufsiz,buf)
 * PARAMETER
 *	app		"application" key
 *	key		"second" key
 *	bufsiz		size of data to save
 *	buf		data to save
 * RETURNS
 *	0		OK
 *	/0		failed
 * GLOBAL
 *	pSetting	root of setting tree
 * DESPRIPTION
 *	Adds new node to settings tree or updates an already
 *	available with new data.
 * REMARKS
 *	Use ProfileRead() before so you're not overwriting
 *	valuable data!
 */
PUBLIC ULONG
ProfileWrite(char const * const app,char const * const key,
	     unsigned const bufsiz,void const * const buf)
{
    SETTING * p;

    assert( app != NULL );
    assert( key != NULL );
    assert( buf != NULL );
    assert( bufsiz != 0 );

    DosRequestMutexSem( hmtxSetting, (ULONG)SEM_INDEFINITE_WAIT );
    for( p = pSetting; p != NULL ; p = p->next )
    {
	if( strcmp(app, p->app) == 0  &&  strcmp(key, p->key) == NULL )
	{
	    if( bufsiz == p->len  &&  memcmp(buf, p->data, bufsiz) == 0 )
		;
	    else
		free( p->data ), p->data = NULL;
	    break;
	}
    }
    if( p == NULL )				/* not already in table? */
    {
	p = malloc( sizeof(SETTING) );
	p->next = pSetting;
	p->app = strdup( app );
	p->key = strdup( key );
	p->data = NULL;
	pSetting = p;
    }
    if( p->data == NULL )
    {
	p->changed = 1;
	p->data = malloc( bufsiz );
	p->len = bufsiz;
	memcpy( p->data, buf, bufsiz );
	Verbose(3, "Profile", "Saved key %s in profile", key);
    }
    DosReleaseMutexSem( hmtxSetting );

    return 0;
}




/*# ----------------------------------------------------------------------
 * ProfileRead(app,key,bufsiz,buf)
 *
 * PARAMETER
 *	app		"application" key
 *	key		"second" key
 *	*size		size of buffer to hold data, will
 *			be updated to size of entry (even
 *			if error -2 is returned)
 *	buf		place data here (OK to say NULL)
 * RETURNS
 *	0		OK
 *	-1UL		profile contains no data for this setting
 *	-2UL		buffer too small
 * GLOBAL
 *	pSetting	root of settings tree
 * DESPRIPTION
 * REMARKS
 */
PUBLIC ULONG
ProfileRead(char const * const app,char const * const key,
	    unsigned * const size,void * const buf)
{
    SETTING const * p;
    ULONG	rc = (ULONG)-1;

    assert( app != NULL );
    assert( key != NULL );

    DosRequestMutexSem( hmtxSetting, (ULONG)SEM_INDEFINITE_WAIT );
    for( p = pSetting; p != NULL ; p = p->next )
    {
	if( strcmp(app, p->app) == 0  &&  strcmp(key, p->key) == NULL )
	{
	    if( p->len > *size )
	    {
		*size = p->len;
		rc = (ULONG)-2;
	    }
	    else
	    {
		if( buf != NULL )
		    memcpy( buf, p->data, p->len );
		*size = p->len;
		rc = 0;
	    }
	    break;
	}
    }
    DosReleaseMutexSem( hmtxSetting );

    return rc;
}




/*# ----------------------------------------------------------------------
 * ProfileReadNext(app,key,bufsiz,buf)
 *
 * PARAMETER
 *	app		"application" key
 *	key		"second" key (or NULL)
 *	*size		size of buffer to hold data, will
 *			be updated to size of entry (even
 *			if error -2 is returned)
 *	buf		place data here (OK to say NULL)
 *
 * RETURNS
 *	0		OK
 *	-1UL		profile contains no next key
 *	-2UL		buffer too small
 *
 * GLOBAL
 *	pSetting	root of settings tree
 *
 * DESPRIPTION
 *	This function will walk through the tree starting at 'pSetting'
 *	and find the entry for 'app' one behind 'key'.  If 'key'
 *	is NULL the very first entry for 'app' is considered to
 *	be the next.
 *	The found entry is returned in 'buf' (up to 'bufsiz' bytes)
 *	and 'key' (updated to enable searching for the next entry).
 *
 * REMARKS
 *	The tree isn't always sorted!
 */
PUBLIC ULONG
ProfileReadNext(char const * const app,char * const key,
		unsigned * const bufsiz,void * const buf)
{
    SETTING const * p;
    ULONG	rc = (ULONG)-1;
    BOOL	found_first = FALSE;

    assert( app != NULL );

    DosRequestMutexSem(hmtxSetting, (ULONG)SEM_INDEFINITE_WAIT);
    for( p = pSetting; p != NULL ; p = p->next )
    {
	if( strcmp(app, p->app) == 0 )
	{
	    if( key[0] == '\0'  ||  found_first == TRUE )
	    {
		strcpy(key, p->key);
		if( p->len > *bufsiz )
		{
		    *bufsiz = p->len;
		    rc = (ULONG)-2;
		}
		else
		{
		    if( buf != NULL )
			memcpy(buf, p->data, p->len);
		    *bufsiz = p->len;
		    rc = 0;
		}
		break;
	    }
	    else if( strcmp(key, p->key) == NULL )
		found_first = TRUE;
	}
    }
    DosReleaseMutexSem(hmtxSetting);

    return rc;
}




/*# ----------------------------------------------------------------------
 * CALL
 *	ProfileDelete(app,key)
 * PARAMETER
 *	app		"application" key
 *	key		"second" key
 * RETURNS
 *	0		OK
 *	/0		failed
 * GLOBAL
 *	pSetting	root of setting tree
 * DESPRIPTION
 *	Removes node in settings tree but _not_ in INI file.
 * REMARKS
 *	Use ProfileRead() before so you're not overwriting
 *	valuable data!
 */
PUBLIC ULONG
ProfileDelete(char const * const app,char const * const key)
{
    SETTING * p, * prev;

    assert( app != NULL );
    assert( key != NULL );

    DosRequestMutexSem( hmtxSetting, (ULONG)SEM_INDEFINITE_WAIT );
    for( prev = NULL, p = pSetting; p != NULL ; prev = p, p = p->next )
    {
	if( strcmp(app, p->app) == 0  &&  strcmp(key, p->key) == NULL )
	{
	    free( p->data );
	    if( prev != NULL )
		prev->next = p->next;
	    else
		pSetting = p->next;
	    free( p );
	    break;
	}
    }
    DosReleaseMutexSem( hmtxSetting );

    return 0;
}

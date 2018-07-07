/*
 * $Source: r:/source/lib/RCS/verbose.c,v $
 * $Revision: 1.10 $
 * $Date: 1999/11/04 21:12:39 $
 * $Locker:  $
 *
 *	Output error/warning/verbose strings to stream.
 *
 * $Log: verbose.c,v $
 * Revision 1.10  1999/11/04 21:12:39  vitus
 * - Verbose: always to stderr
 *
 * Revision 1.9  1998/06/04 06:31:32  vitus
 * - corrected strftime() call
 * - seperated file and screen logging (added SetDisplaylevel)
 *
 * Revision 1.8  1997/01/22 00:41:46  vitus
 * Loglevel now unsigned
 * References verbose.h, not overbose.h (?)
 *
 * Revision 1.7  1996/09/10 00:36:58  vitus
 * Changed to HFILE usage
 *
 * Revision 1.6  1996/05/12 01:21:55  vitus
 * Changed timer to 24hour display
 *
 * Revision 1.5  1996/05/11 00:56:35  vitus
 * Added support of GNU C compiler
 *
 * Revision 1.4  1996/03/23 23:42:29  vitus
 * Moved newline to end of format string
 *
 * Revision 1.3  1996/03/13 23:35:48  vitus
 * Changed to binkley-style output
 *
 * Revision 1.2  1996/03/13 21:09:11  vitus
 * Removed initialisation of fpLogfile if IBM CSet++
 *
 * Revision 1.1  1996/01/27 02:53:24  vitus
 * Initial revision
 */
static char const id[]="$Id: verbose.c,v 1.10 1999/11/04 21:12:39 vitus Exp $";

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INCL_DOS
#include <os2.h>

#include "verbose.h"


static HFILE	hLogfile = 0;			/* 0 is impossible */
static unsigned	iDisplaylevel = 1;		/* only fatal errors */
static unsigned	iLoglevel = 1;			/* only fatal errors */

static char const achLevel[] = "!*+:# ";



void
Verbose(unsigned level,char const * module,char const * fmt,...)
{
    APIRET rc;

    if( level <= iLoglevel  &&  hLogfile != 0 )
    {
	va_list	argp;
	size_t	cb = strlen(fmt)+strlen(module)+200;
	char	*str = malloc(cb);
	char	*buf = malloc(LOGBUFSIZ);
	ULONG	written;
	time_t	now = 0;
	struct tm * timep;

	if( level >= strlen(achLevel) )
	    str[0] = ' ';
	else
	    str[0] = achLevel[level];

	time(&now);
	timep = localtime(&now);
	strftime(&str[1], 200-3, " %d %b %H:%M:%S ", timep);

	strcat(str, module);
	strcat(str, " ");
	strcat(str, fmt);			/* append format string */
	strcat(str, "\r\n");			/* append newline */

	va_start(argp,fmt);
	vsprintf(buf, str, argp);
	rc = DosWrite(hLogfile, buf, strlen(buf), &written);
	va_end(argp);

	free(buf);
	free(str);
    }
    if( level <= iDisplaylevel )
    {
	va_list	argp;
	size_t	cb = strlen(fmt)+strlen(module)+200;
	char	*str = malloc(cb);

	strcpy(str, "\r\n");
	strcat(str, fmt);			/* append format string */

	va_start(argp,fmt);
	vfprintf(stderr, str, argp),		fflush(stderr);
	va_end(argp);

	free(str);
    }

    return;
}




HFILE
SetLogfile(HFILE new)
{
    HFILE old = hLogfile;
    hLogfile = new;
    return old;
}




unsigned
SetLoglevel(unsigned new)
{
    unsigned	old = iLoglevel;

    iLoglevel = new;
    return old;
}




unsigned
SetDisplaylevel(unsigned new)
{
    unsigned	old = iDisplaylevel;

    iDisplaylevel = new;
    return old;
}

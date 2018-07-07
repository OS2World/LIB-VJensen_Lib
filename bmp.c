/*
 * $Source$
 * $Revision$
 * $Date$
 * $Locker$
 *
 *	Bitmap laden
 *	Einschr„nkungen:
 *	- BMP
 *	- 24 Bit
 *
 *	Original: siehe www.edm2.com
 *
 * $Log$
 * ----------------------------------------
 */
#pragma strings(readonly)
static char const vcid[]="$Id$";

#include <stdlib.h>
#include <io.h>
#include <fcntl.h>


#include "bmp.h"


int
LoadBmp(char const * filename, PIMAGE image)
{
    int		hd;
    long	fsize;
    char *	buffer;
    int		i;

    if( image == NULL )
	return 1;				/* na, wolln wir nicht? */

    hd = open(filename, O_BINARY|O_RDONLY);
    if( hd == -1 )
	return 2;				/* Datei fehlt? */

    /* Ist das wirklich eine 24Bit Bitmap?  Dann máten
     * es ja 3 Bytes pro Pixel sein... */

    fsize = filelength(hd);
    if( fsize % 3 != 0 )
    {
	close(hd);
	return 3;				/* falsches Format */
    }
    image->colors = 24;
    image->cx = ;

    buffer = malloc(fsize);
    if( buffer == NULL )
    {
	close(hd);
	return NULL;				/* huch? kein Speicher mehr??? */
    }

    i = read(hd, buffer, fsize);
    if( i != fsize )
    {
	close(hd);
	free(buffer);
	return NULL;				/* Dateisystem kaputt??? */
    }

    close(hd);


#if 0
    /* Reset data back to RGB from BGR */

    for( i = 0; i < fsize; i += 3 )
    {
	buffer[i] = buffer[22+i];
	buffer[i+1] = buffer[21+i];
	buffer[i+2] = buffer[20+i];
    }
#endif

    return buffer;
}

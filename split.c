/*
 * $Source: e:/source/tools/split.c,v $
 * $Revision: 1.1 $
 * $Date: 1996/01/13 14:23:09 $
 * $Author: vitus $
 *
 * Split binary file to tiny parts.
 *
 * $Log: split.c,v $
 * Revision 1.1  1996/01/13 14:23:09  vitus
 * Initial revision
 *
 */
static char vcid[]="$Id: split.c,v 1.1 1996/01/13 14:23:09 vitus Exp $";

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


char		 szPrgName[] = 	"split";
unsigned long	 ulPartSize =	710*1024L;
FILE		*fpStatus;
FILE		*fpInput;

int		 fVerbose =	1;		/* 0: quiet (return error code)
						   1: display error messages
						   2: display logo, ... */





void Verbose(int level,char *fmt,...)
{
    va_list arglist;

    /* If below actual level display it
     */
    if( level <= fVerbose )
    {
	va_start( arglist, fmt );
	vfprintf( fpStatus, fmt, arglist );
	fflush( fpStatus );
	va_end( arglist );
    }
    return;
}


void help(void)
{
    Verbose(0,"no help available\n");
}



int examine_args(int argc,char *argv[])
{
    while( argc > 1  &&  argv[1][0] == '-' )
    {
	switch( argv[1][1] )
	{
	  case 'q':
	  case 'Q':
	    fVerbose = 0;
	    break;

	  case 'v':
	  case 'V':
	    fVerbose = 2;
	    break;

	  case '?':
	  case 'h':
	  case 'H':
	    help();
	    break;

	  case 'c':
	  case 'C':
	    sscanf( &argv[1][2], "%lu", &ulPartSize );
	    Verbose(2,"generating files %lu bytes in size\n",ulPartSize);
	    break;

	  default:
	    Verbose(2,"unknown parameter \'%c\' (%s -? for help)\n",
		    argv[1][1],szPrgName);
	    return 1;
	}
	--argc;
	++argv;
    }
    if( argc != 2 )
    {
	Verbose(1,"missing file name (%s -? for help)\n",szPrgName);
	return 1;
    }
    if( strcmp(argv[1], "-") == 0 )
    {
	fpInput = stdin;
	Verbose(2,"using stdin");
    }
    else
    {
	Verbose(2,"input file is %s\n",argv[1]);
	if( (fpInput=fopen(argv[1], "rb")) == NULL )
	{
	    Verbose(1,"error opening %s (errno %u)\n",argv[1],errno);
	    return 2;
	}
    }
    return 0;
}




int SplitFile(FILE *in,unsigned long part)
{
    unsigned long  size;
    size_t	cb = 0;
    int		instance, rc;
    char	*buf;
    
    if( (buf=malloc(part)) == NULL )
    {
	Verbose(1,"memory problem\n");
	return 2;
    }
    if( (rc=fseek(in, 0, SEEK_END)) != 0 )
    {
	Verbose(1,"fseek - rc %u\n",rc);
	return 2;
    }
    size = (unsigned long)ftell( in );
    Verbose(2,"original file is %lu bytes\n",size);
    if( (rc=fseek(in, 0, SEEK_SET)) != 0 )		/* rewind */
    {
	Verbose(1,"fseek - rc %u\n",rc);
	return 2;
    }

    for( instance = 0; size != 0; ++instance, size -= cb )
    {
	char name[_MAX_PATH];
	FILE *out;

	sprintf( name, "x%03d", instance );
	Verbose(2,"creating %s\t",name);
	if( (out=fopen(name, "wb")) == NULL )
	{
	    Verbose(1,"error opening %s (errno %u)\n",name,errno);
	    free( buf );
	    return -1;
	}
	cb = fread( buf, 1, part, in );
	Verbose(2,"copying %u bytes, %lu remaining\n",cb,size);
	fwrite( buf, 1, cb, out );
	fclose( out );
    }

    free( buf );
    return 0;
}




int main(int argc,char *argv[])
{
    int rc;

    fpStatus = stderr;
    if( (rc=examine_args(argc, argv)) != 0 )
	return rc;

    return SplitFile( fpInput, ulPartSize );
}

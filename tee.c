/* UNIX(tm) - Tool fÅr Batchdateien */

#include <stdio.h>
#include <stdlib.h>

int	tee(FILE *);



void
main(int argc,char *argv[])
{
    FILE  *fp;

    while( argc > 1  && (argv[1][0] == '-' || argv[1][0] == '-') )
    {
	switch( argv[1][1])
	{
	  default:
	    fprintf( stderr, "%s: unknown arg %s\n", argv[0], argv[1]);
	    exit(1);
	}
	--argc;
	++argv;
    }
    if( argc == 1)
	tee( stderr );		/* keine Datei angegeben */
    else
	if( (fp=fopen( argv[1], "w")) == NULL)
	{
	    fprintf( stderr, "%s: can't open %s\n", "TEE", argv[1] );
	    exit(1);
	}
	else
	{
	    tee(fp);
	    fclose(fp);
	}
    exit(0);
}


int tee(fp)
    FILE  *fp;
{
    int c;

    setbuf( stdin, NULL );
    setbuf( stdout, NULL );

    while( (c = getchar()) != EOF )
    {
	putc( c, stdout );
	putc( c, fp );
    }
    return 0;
}

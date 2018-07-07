/*
****	OBS:	this program executes a warm boot!
****
****	lt. OS2PROG	fidoecho
*/

#include <stdlib.h>

#define INCL_DOSFILEMGR
#include <os2.h>


main()
{
    APIRET rc;
    HFILE hf;
    ULONG dummy;

    rc = DosOpen("DOS$", &hf, &dummy, 0L, FILE_NORMAL, FILE_OPEN,
		 OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE |
		 OPEN_FLAGS_FAIL_ON_ERROR, 0L);

    if( !rc )
    {
	DosShutdown(0L);
	DosDevIOCtl(NULL, NULL, 0xab, 0xd5, hf);
	DosClose(hf);
    }
    else
	printf("DOS.SYS not installed (sys%04u)\n", rc);

    exit(rc);
}


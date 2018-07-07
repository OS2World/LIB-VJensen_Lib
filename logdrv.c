/*
 * $Source: r:/source/lib/RCS/logdrv.c,v $
 * $Revision: 1.2 $
 * $Date: 1999/11/11 21:48:58 $
 * $Locker:  $
 *
 *	Routines to directly access logical devices (drives).
 *
 * $Log: logdrv.c,v $
 * Revision 1.2  1999/11/11 21:48:58  vitus
 * - implemented LDrvLockCD(), LDrvUnlockCD(), LDrvEjectCD()
 * - local structure to hold handle and flags
 *
 * Revision 1.1  1999/11/04 00:39:43  vitus
 * Initial revision
 * ----------------------------------------
 * This code is Copyright Vitus Jensen 1999
 */
static char const vcid[]="$Id: logdrv.c,v 1.2 1999/11/11 21:48:58 vitus Exp $";

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_ERRORS
#include <os2.h>
#define FSC_SECTORIO	0x9014			/* new, not in toolkit */

#include "defines.h"
#include "logdrv.h"



#define SECTORSIZE	512
#define MAX_DEVICES	32


typedef struct _DEVTAB {
    BOOL		open;
    BOOL		smode;			/* TRUE: sector mode */
    HFILE		hd;			/* from DosOpen() */
} DEVTAB, * PDEVTAB;

PRIVATE DEVTAB	DevTab[MAX_DEVICES] = {0};





PRIVATE PDEVTAB
Allocate(void)
{
    int	i;

    for( i = 0; i < MAX_DEVICES; ++i )
	if( !DevTab[i].open )
	{
	    DevTab[i].open = TRUE;
	    return &DevTab[i];
	}
    return NULL;
}

PRIVATE void
Free(PDEVTAB dt)
{
    assert( dt->open == TRUE );
    memset(dt, 0, sizeof(*dt));
}






/* **********************************************************************
 * **** Public Entries **************************************************
 * ******************************************************************* */

PUBLIC APIRET
LDrvEnum(PULONG count)
{
    return -1;					/* to be implemented */
}




/*# ----------------------------------------------------------------------
 * LDrvOpen(idx,handle)
 *
 * PARAMETER
 *	idx		index of logical drive (A: = 0)
 *	handle		place for return value
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Opens a logical drive in DENY_WRITE mode.
 *
 * REMARKS
 */
PUBLIC APIRET
LDrvOpen(ULONG idx,PULONG handle)
{
    APIRET	rc;
    PDEVTAB const dt = Allocate();

    if( dt == NULL )
	return ERROR_TOO_MANY_OPEN_FILES;

    {
	char	str[3];
	ULONG	action_taken;			/* what did OS/2? */
	ULONG	new_attr = 0;			/* attr. if created (ignored) */
	ULONG	open_flag;
	ULONG	open_mode = OPEN_FLAGS_FAIL_ON_ERROR;
	ULONG	new_size = 0;			/* new size of file (ignored) */


	open_flag = OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS;
	open_mode |= OPEN_FLAGS_NO_CACHE|OPEN_FLAGS_RANDOMSEQUENTIAL
	    |OPEN_SHARE_DENYWRITE|OPEN_ACCESS_READONLY
	    | OPEN_FLAGS_DASD;

	sprintf(str, "%c:", 'A'+idx);
	rc = DosOpen(str, &dt->hd, &action_taken,
		     new_size, new_attr, open_flag, open_mode, 0);
    }
    if( rc == 0 )
    {
	ULONG	parm = 0xDEADFACE;
	ULONG	parmsize = sizeof(parm);
	ULONG	datasize = 0;
	ULONG   function = FSC_SECTORIO;

	rc = DosFSCtl(NULL, datasize, &datasize, &parm, parmsize, &parmsize,
		      function, NULL, dt->hd, 1);
	if( rc == 0 )
	    dt->smode = TRUE;
	rc = 0;

	*handle = (ULONG)dt;
    }
    return rc;
}




/*# ----------------------------------------------------------------------
 * LDrvClose(handle)
 *
 * PARAMETER
 *	handle		from LDrvOpen()
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Inverses the effekt of LDrvOpen().
 *
 * REMARKS
 */
PUBLIC APIRET
LDrvClose(ULONG handle)
{
    APIRET	rc;
    PDEVTAB	dt = (PDEVTAB)handle;

    rc = LDrvUnlockIO(handle);			/* ignore error */
    rc = DosClose(dt->hd);
    if( rc == 0 )
	Free(dt);
    return rc;
}




/*# ----------------------------------------------------------------------
 * LDrvQuerySize(handle,seccnt)
 *
 * PARAMETER
 *	handle		from LDrvOpen()
 *	seccnt		room for return value
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Returns count of sectors (numbered 0 - *seccnt-1).
 *
 * REMARKS
 */
PUBLIC APIRET
LDrvQuerySize(ULONG handle,PULONG seccnt)
{
    APIRET	rc;
    PDEVTAB	dt = (PDEVTAB)handle;
    FILESTATUS3	fsts3ConfigInfo = {{0}};       /* Buffer for file information */
    ULONG       ulBufSize = sizeof(FILESTATUS3); /* Size of above buffer */


    rc = DosQueryFileInfo(dt->hd, FIL_STANDARD, &fsts3ConfigInfo, ulBufSize);
    if( rc == 0 )
    {
	if( dt->smode )
	    *seccnt = fsts3ConfigInfo.cbFile;
	else
	    *seccnt = fsts3ConfigInfo.cbFile / SECTORSIZE;
    }
    return rc;
}




/*# ----------------------------------------------------------------------
 * LDrvLockIO(handle)
 *
 * PARAMETER
 *	handle		from LDrvOpen()
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	"This function locks a drive and is used to exclude I/O by another
 *	process on the volume in the drive.  The Lock Drive function succeeds
 *	only if there is one file handle open on the volume in the drive (that
 *	is, the file handle on which this function is issued)."
 *
 * REMARKS
 */
PUBLIC APIRET
LDrvLockIO(ULONG handle)
{
    APIRET	rc;
    PDEVTAB	dt = (PDEVTAB)handle;

    UCHAR	parm_packet = 0;
    UCHAR	data_packet = 0;
    ULONG	parm_size = 1;
    ULONG	data_size = 1;

    rc = DosDevIOCtl(dt->hd, 8, 0,
		     &parm_packet, parm_size, &parm_size,
		     &data_packet, data_size, &data_size);
    return rc;
}




/*# ----------------------------------------------------------------------
 * LDrvUnlockIO(handle)
 *
 * PARAMETER
 *	handle		from LDrvOpen()
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Inverses LDrvLockIO().
 *
 * REMARKS
 */
PUBLIC APIRET
LDrvUnlockIO(ULONG handle)
{
    APIRET	rc;
    PDEVTAB	dt = (PDEVTAB)handle;

    UCHAR	parm_packet = 0;
    UCHAR	data_packet = 0;
    ULONG	parm_size = 1;
    ULONG	data_size = 1;

    rc = DosDevIOCtl(dt->hd, 8, 1,
		     &parm_packet, parm_size, &parm_size,
		     &data_packet, data_size, &data_size);
    return rc;
}




/*# ----------------------------------------------------------------------
 * LDrvRead(handle,secno,seccnt,data)
 *
 * PARAMETER
 *	handle
 *	secno
 *	seccnt
 *	data
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Reads contents of one or several sectors.
 *
 * REMARKS
 */
PUBLIC APIRET
LDrvRead(ULONG handle,ULONG secno,USHORT seccnt,PVOID data)
{
    APIRET	rc;
    PDEVTAB	dt = (PDEVTAB)handle;

    do
    {
	ULONG	ul;

	if( dt->smode == FALSE )
	{
	    secno *= SECTORSIZE;
	    seccnt *= SECTORSIZE;
	}
	rc = DosSetFilePtr(dt->hd, (LONG)secno, FILE_BEGIN, &ul);
	if( rc != 0 )
	    break;
	if( ul != secno )
	{
	    rc = -1ul;
	    break;
	}

	rc = DosRead(dt->hd, data, seccnt, &ul);
	if( rc != 0 )
	    break;
	if( ul != seccnt )
	{
	    rc = -1ul;
	    break;
	}
    }
    while( 0 );

    return rc;
}




PUBLIC APIRET
LDrvWrite(ULONG handle,ULONG secno,USHORT seccnt,PVOID data)
{
    APIRET	rc;
    PDEVTAB	dt = (PDEVTAB)handle;

    do
    {
	ULONG	ul;

	if( dt->smode == FALSE )
	{
	    secno *= SECTORSIZE;
	    seccnt *= SECTORSIZE;
	}
	rc = DosSetFilePtr(dt->hd, (LONG)secno, FILE_BEGIN, &ul);
	if( rc != 0 )
	    break;
	if( ul != secno )
	{
	    rc = -1ul;
	    break;
	}

	rc = DosWrite(dt->hd, data, seccnt, &ul);
	if( rc != 0 )
	    break;
	if( ul != seccnt )
	{
	    rc = -1ul;
	    break;
	}
    }
    while( 0 );

    return rc;
}




/*# ----------------------------------------------------------------------
 * LDrvLockCD(handle)
 *
 * PARAMETER
 *	handle		from LDrvOpen()
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Locks the CDROM so that it can't be removed.
 *
 * REMARKS
 */
PUBLIC APIRET
LDrvLockCD(ULONG handle)
{
    APIRET	rc;
    PDEVTAB	dt = (PDEVTAB)handle;

    UCHAR	parm[5];
    ULONG	parmlen = sizeof(parm);


    memcpy(parm, "CD01", 4);
    parm[4] = 1;
    rc = DosDevIOCtl(dt->hd, IOCTL_CDROMDISK, CDROMDISK_LOCKUNLOCKDOOR,
		     &parm, parmlen, &parmlen, NULL, 0, NULL);
    return rc;
}




/*# ----------------------------------------------------------------------
 * LDrvUnlockCD(handle)
 *
 * PARAMETER
 *	handle		from ???
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Reverse the effect of LDrvLockMedia().
 *
 * REMARKS
 */
PUBLIC APIRET
LDrvUnlockCD(ULONG handle)
{
    APIRET	rc;
    PDEVTAB	dt = (PDEVTAB)handle;

    UCHAR	parm[5];
    ULONG	parmlen = sizeof(parm);


    memcpy(parm, "CD01", 4);
    parm[4] = 0;
    rc = DosDevIOCtl(dt->hd, IOCTL_CDROMDISK, CDROMDISK_LOCKUNLOCKDOOR,
		     &parm, parmlen, &parmlen, NULL, 0, NULL);
    return rc;
}





/*# ----------------------------------------------------------------------
 * LDrvEjectCD(handle)
 *
 * PARAMETER
 *	handle		from LDrvOpen()
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Guess!
 *
 * REMARKS
 */
PUBLIC APIRET
LDrvEjectCD(ULONG handle)
{
    APIRET	rc;
    PDEVTAB	dt = (PDEVTAB)handle;

    UCHAR	parm[4];
    ULONG	parmlen = sizeof(parm);


    memcpy(parm, "CD01", 4);
    rc = DosDevIOCtl(dt->hd, IOCTL_CDROMDISK, CDROMDISK_EJECTDISK,
		     &parm, parmlen, &parmlen, NULL, 0, NULL);
    return rc;
}

/*
 * $Source: r:/source/lib/RCS/physdsk.c,v $
 * $Revision: 1.9 $
 * $Date: 1999/11/11 21:49:41 $
 * $Locker:  $
 *
 *	Routines to directly access physical devices (disk),
 *	*NOT* logical partitions!
 *
 * $Log: physdsk.c,v $
 * Revision 1.9  1999/11/11 21:49:41  vitus
 * - renamed PDskLock/PDskUnlock to PDskLockIO/PDskUnlockIO
 *
 * Revision 1.8  1998/12/12 18:47:40  vitus
 * - PDskOpen: failures in PDSK_GETPHYSDEVICEPARAMS should close
 *   the allocated handle, corrected
 *
 * Revision 1.7  1998/12/11 03:09:46  vitus
 * - PDskOpen korrigiert: 'open' wird schon von Allocate() gesetzt, entfernt.
 *
 * Revision 1.6  1998/07/30 09:26:36  vitus
 * - relocated source, #includes updated
 *
 * Revision 1.5  1998/05/20 01:16:23  vitus
 * - includes physdsk.h (instead of proto.h)
 *
 * Revision 1.4  1998/04/29 01:53:02  vitus
 * - handle meaning now index in local table
 *
 * Revision 1.3  1998/04/01 01:10:56  vitus
 * - now uses DevTab structure and returns pointer to it as handle
 *
 * Revision 1.2  1998/03/14 14:06:19  vitus
 * - added PDskLock, PdskUnlock
 *
 * Revision 1.1  1998/03/11 02:34:43  vitus
 * Initial revision
 * ----------------------------------------
 * This code is given to the public domain
 */
static char const id[]="$Id: physdsk.c,v 1.9 1999/11/11 21:49:41 vitus Exp $";

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_ERRORS
#include <os2.h>

#include "defines.h"
#include "physdsk.h"


#define SECTORSIZE	512
#define MAX_DEVICES	32



typedef struct _DEVTAB {
    USHORT		open;
    USHORT		hd;			/* from GET_IOCTLHANDLE */
    DEVICEPARAMETERBLOCK layout;		/* layout of disk */
} DEVTAB, * PDEVTAB;

PRIVATE DEVTAB	DevTab[MAX_DEVICES] = {0};





PRIVATE PDEVTAB
Allocate(void)
{
    int	i;

    for( i = 0; i < MAX_DEVICES; ++i )
	if( !DevTab[i].open )
	{
	    DevTab[i].open = 1;
	    return &DevTab[i];
	}
    return NULL;
}

PRIVATE void
Free(PDEVTAB dt)
{
    assert( dt->open != 0 );
    memset(dt, 0, sizeof(*dt));
}




/*# ----------------------------------------------------------------------
 * CALL
 *	PDskEnum(count)
 * PARAMETER
 *	count		returns count
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Returns count of installed hard disks.
 *
 * REMARKS
 */
PUBLIC APIRET
PDskEnum(PULONG count)
{
    APIRET	rc;
    USHORT	us;

    rc = DosPhysicalDisk(INFO_COUNT_PARTITIONABLE_DISKS,
			 &us, sizeof(us),
			 NULL, 0);
    if( rc == 0 )
	*count = us;
    return rc;
}




/*# ----------------------------------------------------------------------
 * CALL
 *	PDskOpen(idx,handle)
 * PARAMETER
 *	idx		disk index (0 - N-1)
 *	handle		returns handle
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Enables access to a hard disk.
 *
 * REMARKS
 */
PUBLIC APIRET
PDskOpen(ULONG idx,PULONG handle)
{
    APIRET	rc;
    char	str[20];
    PDEVTAB	dt = Allocate();

    if( dt == NULL )
	return ERROR_TOO_MANY_OPEN_FILES;

    sprintf(str, "%lu:", idx+1);
    rc = DosPhysicalDisk(INFO_GETIOCTLHANDLE,
			 &dt->hd, sizeof(dt->hd),
			 str, strlen(str)+1);
    if( rc == 0 )
    {
	UCHAR	ucParm = 0;
	ULONG	ulParmLen = sizeof(ucParm);
	ULONG	ulDataLen = sizeof(dt->layout);

	rc = DosDevIOCtl(dt->hd, IOCTL_PHYSICALDISK, PDSK_GETPHYSDEVICEPARAMS,
			 &ucParm, ulParmLen, &ulParmLen,
			 &dt->layout, ulDataLen, &ulDataLen);
	if( rc != 0 )
	    PDskClose((ULONG)dt);
	else
	    *handle = (ULONG)dt;
    }
    return rc;
}




/*# ----------------------------------------------------------------------
 * CALL
 *	PDskClose(handle)
 * PARAMETER
 *	handle		from PDskOpen()
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Ends hard disk access.
 *
 * REMARKS
 */
PUBLIC APIRET
PDskClose(ULONG handle)
{
    APIRET	rc;
    PDEVTAB	dt = (PDEVTAB)handle;

    rc = DosPhysicalDisk(INFO_FREEIOCTLHANDLE,
			 NULL, 0,
			 &dt->hd, sizeof(dt->hd));
    Free(dt);
    return rc;
}




/*# ----------------------------------------------------------------------
 * CALL
 *	PDskQueryParam(handle,dp)
 * PARAMETER
 *	handle		from PDskOpen()
 *	dp		returns structure
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Returns OS/2 structure about physical parameters (plates, etc.)
 *	of this hard disk unit.
 *
 * REMARKS
 */
PUBLIC APIRET
PDskQueryParam(ULONG handle,PDEVICEPARAMETERBLOCK dp)
{
    PDEVTAB	dt = (PDEVTAB)handle;

    memcpy(dp, &dt->layout, sizeof(dt->layout));
    return 0;
}




/*# ----------------------------------------------------------------------
 * CALL
 *	PDskLockIO(handle)
 * PARAMETER
 *	handle		from PDskOpen()
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Locks the hard disk unit so that no other process
 *	may write to it.
 *
 * REMARKS
 */
PUBLIC APIRET
PDskLockIO(ULONG handle)
{
    APIRET	rc;
    PDEVTAB	dt = (PDEVTAB)handle;
    UCHAR	ucParm = 0;
    ULONG	ulParmLen = sizeof(ucParm);
    UCHAR	aucData[31];
    ULONG	ulDataLen = sizeof(aucData);

    memset(aucData, 0, sizeof(aucData));
    rc = DosDevIOCtl(dt->hd, IOCTL_PHYSICALDISK, PDSK_LOCKPHYSDRIVE,
		     &ucParm, ulParmLen, &ulParmLen,
		     aucData, ulDataLen, &ulDataLen);
    return rc;
}




/*# ----------------------------------------------------------------------
 * CALL
 *	PDskUnlockIO(handle)
 * PARAMETER
 *	handle		from PDskOpen()
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Reverse the effect of PDskLockIO().
 *
 * REMARKS
 */
PUBLIC APIRET
PDskUnlockIO(ULONG handle)
{
    APIRET	rc;
    PDEVTAB	dt = (PDEVTAB)handle;
    UCHAR	ucParm = 0;
    ULONG	ulParmLen = sizeof(ucParm);
    UCHAR	aucData[31];
    ULONG	ulDataLen = sizeof(aucData);

    memset(aucData, 0, sizeof(aucData));
    rc = DosDevIOCtl(dt->hd, IOCTL_PHYSICALDISK, PDSK_UNLOCKPHYSDRIVE,
		     &ucParm, ulParmLen, &ulParmLen,
		     aucData, ulDataLen, &ulDataLen);
    return rc;
}




/*# ----------------------------------------------------------------------
 * CALL
 *	PDskRead(handle,secno,seccnt,data)
 * PARAMETER
 *	handle		from PDskOpen()
 *	secno		sector no (0 - N-1)
 *	seccnt		how may sectors
 *	data		place data here
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Reads one or more sectors from disk into memory.
 *
 * REMARKS
 *	Be carefull when reading more than a single sector: they shouldn't
 *	span more than a track.
 */
PUBLIC APIRET
PDskRead(ULONG handle,ULONG secno,USHORT seccnt,PVOID data)
{
    APIRET	rc;
    USHORT	i;
    PDEVTAB	dt = (PDEVTAB)handle;
    ULONG	ulParmLen = sizeof(TRACKLAYOUT) + (seccnt - 1) * 4;
    PTRACKLAYOUT	tl = malloc(ulParmLen);
    ULONG	ulDataLen = seccnt * SECTORSIZE;
    USHORT	sector;

    if( tl == NULL )
	return ERROR_NOT_ENOUGH_MEMORY;

    tl->bCommand = 0;
    sector = secno % dt->layout.cSectorsPerTrack + 1;
    tl->usHead = (secno / dt->layout.cSectorsPerTrack) % dt->layout.cHeads;
    tl->usCylinder = secno / (dt->layout.cSectorsPerTrack
			      * ((ULONG)dt->layout.cHeads));
    tl->usFirstSector = 0;			/* index in track table? */
    tl->cSectors = seccnt;
    for( i = 0; i < seccnt; ++i )
    {
	tl->TrackTable[i].usSectorNumber = sector + i;
	tl->TrackTable[i].usSectorSize = SECTORSIZE;
    }
    rc = DosDevIOCtl(dt->hd, IOCTL_PHYSICALDISK, PDSK_READPHYSTRACK,
		     tl, ulParmLen, &ulParmLen,
		     data, ulDataLen, &ulDataLen);

    free(tl);
    return rc;
}




/*# ----------------------------------------------------------------------
 * CALL
 *	PDskWrite(handle,secno,seccnt,data)
 * PARAMETER
 *	handle		from PDskOpen()
 *	secno		sector no (0 - N-1)
 *	seccnt		how may sectors
 *	data		place data here
 *
 * RETURNS
 *	APIRET
 *
 * DESCRIPTION
 *	Writes one or more sectors to disk.
 *
 * REMARKS
 *	Be carefull when writing more than a single sector: they shouldn't
 *	span more than a track.
 */
PUBLIC APIRET
PDskWrite(ULONG handle,ULONG secno,USHORT seccnt,PVOID data)
{
    APIRET	rc;
    USHORT	i;
    PDEVTAB	dt = (PDEVTAB)handle;
    ULONG	ulParmLen = sizeof(TRACKLAYOUT) + (seccnt - 1) * 4;
    PTRACKLAYOUT	tl = malloc(ulParmLen);
    ULONG	ulDataLen = seccnt * SECTORSIZE;
    USHORT	sector;

    if( tl == NULL )
	return ERROR_NOT_ENOUGH_MEMORY;

    tl->bCommand = 0;
    sector = secno % dt->layout.cSectorsPerTrack + 1;
    tl->usHead = (secno / dt->layout.cSectorsPerTrack) % dt->layout.cHeads;
    tl->usCylinder = secno / (dt->layout.cSectorsPerTrack
			      * ((ULONG)dt->layout.cHeads));
    tl->usFirstSector = 0;			/* index in track table? */
    tl->cSectors = seccnt;
    for( i = 0; i < seccnt; ++i )
    {
	tl->TrackTable[i].usSectorNumber = sector + i;
	tl->TrackTable[i].usSectorSize = SECTORSIZE;
    }
    rc = DosDevIOCtl(dt->hd, IOCTL_PHYSICALDISK, PDSK_WRITEPHYSTRACK,
		     tl, ulParmLen, &ulParmLen,
		     data, ulDataLen, &ulDataLen);
    free(tl);
    return rc;
}

/*
 * $Source: r:/source/lib/RCS/physdsk.h,v $
 * $Revision: 1.2 $
 * $Date: 1999/11/11 20:11:42 $
 * $Locker:  $
 *
 *	Interface to physdsk.c
 *
 * $Log: physdsk.h,v $
 * Revision 1.2  1999/11/11 20:11:42  vitus
 * - renamed PDskLock/PDskUnlock to PDskLockIO/PDskUnlockIO
 *
 * Revision 1.1  1998/04/29 02:00:16  vitus
 * Initial revision
 * ----------------------------------------
 * This code is Copyright Vitus Jensen 1998
 */

extern APIRET	PDskEnum(PULONG count);
extern APIRET	PDskOpen(ULONG idx,PULONG handle);
extern APIRET	PDskClose(ULONG handle);

extern APIRET	PDskQueryParam(ULONG handle,PDEVICEPARAMETERBLOCK dp);
extern APIRET	PDskLockIO(ULONG handle);
extern APIRET	PDskUnlockIO(ULONG handle);
extern APIRET	PDskRead(ULONG handle,ULONG secno,USHORT seccnt,PVOID data);
extern APIRET	PDskWrite(ULONG handle,ULONG secno,USHORT seccnt,PVOID data);

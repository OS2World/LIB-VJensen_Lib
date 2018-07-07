/*
 * $Source: r:/source/lib/RCS/logdrv.h,v $
 * $Revision: 1.2 $
 * $Date: 1999/11/11 21:47:43 $
 * $Locker:  $
 *
 *	Interface to logdsk.c
 *
 * $Log: logdrv.h,v $
 * Revision 1.2  1999/11/11 21:47:43  vitus
 * - added	LDrvLockCD(), LDrvUnlockCD(), LDrvEjectCD()
 *
 * Revision 1.1  1999/11/03 22:20:57  vitus
 * Initial revision
 * ----------------------------------------
 * This code is Copyright Vitus Jensen 1999
 */

extern APIRET	LDrvEnum(PULONG count);
extern APIRET	LDrvOpen(ULONG idx,PULONG handle);
extern APIRET	LDrvClose(ULONG handle);

extern APIRET	LDrvQuerySize(ULONG handle,PULONG seccnt);
extern APIRET	LDrvLockIO(ULONG handle);
extern APIRET	LDrvUnlockIO(ULONG handle);
extern APIRET	LDrvRead(ULONG handle,ULONG secno,USHORT seccnt,PVOID data);
extern APIRET	LDrvWrite(ULONG handle,ULONG secno,USHORT seccnt,PVOID data);

extern APIRET	LDrvLockCD(ULONG handle);
extern APIRET	LDrvUnlockCD(ULONG handle);
extern APIRET	LDrvEjectCD(ULONG handle);

#ifndef __PMTools_H
#  define __PMTools_H

#  include <os2def.h>

#  ifdef __cplusplus
   extern "C" {
#  endif

   extern void     CenterWindow( HWND hwnd );
   extern HWND     CreateHelp( HWND hwndFrame, PSZ pszFileName, HMODULE hmod, USHORT usTableID, PSZ pszWindowTitle );
   extern void     DestroyHelp( HWND hwndHelp );
   extern void     SetButtonIcon( HWND hwndButton, HPOINTER hicon );
   extern HWND     LaunchApplication( HWND hwnd, PSZ pszProgram, PSZ pszOptions, PSZ pszDirectory );
   extern HMTX     OneInstance( PSZ pszSemName, BOOL fSwitch );
   extern HWND     LoadMenu( HWND hwnd,HMODULE hmod, USHORT usResID );
   extern HPOINTER LoadIcon( HWND hwnd, HMODULE hmod, USHORT usResID );
   extern HACCEL   LoadAccelerator( HWND hwnd, HMODULE hmod, USHORT usResID );

#  ifdef __cplusplus
   }
#  endif

#endif /* __PMTools_H */


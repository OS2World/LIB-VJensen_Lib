#include <stdlib.h>
#include <string.h>

#define  INCL_DOS
#define  INCL_DOSERRORS /* how come this isn'd included by INCL_DOS ?! */
#define  INCL_PM

#include <os2.h>

/**
 * Center a window within its parent window.
 *
 * Entry:
 *    
 *    The handle of the window to center.
 *
 * Exit:
 *
 *    Nothing.
 */
void
CenterWindow( HWND hwnd )
/***********************/
{
   RECTL rclParent;
   RECTL rclWindow;

   WinQueryWindowRect( hwnd, &hwndWindow );
   WinQueryWindowRect( WinQueryWindow( hwnd, QW_PARENT ), &rclParent );

   rclWindow.xLeft   = ( rclParent.xRight - rclWindow.xRight ) / 2;
   rclWindow.yBottom = ( rclParent.yTop   - rclWindow.yTop   ) / 2;

   WinSetWindowPos( hwnd, NULLHANDLE, rclWindow.xLeft, rclWindow.yBottom,
                    0, 0, SWP_MOVE );
}

/**
 * Create a help instance and associate it with a window.
 *
 * Entry:
 *
 *    hwndFrame      - The window handle.
 *    pszFileName    - The pathname of the help file.
 *                     If NULL or empty, will build a helpfile name
 *                     using the path and root file name of the executable.
 *    hmod           - The handle of the module containing the help table.
 *                     NULLHANDLE == current.
 *    usTableID      - The resource ID of the help table.
 *    pszWindowTitle - The tile for the help window (may be NULL of empty ).
 *
 * Exit:
 *
 *    The help window's handle on success.
 */
HWND
CreateHelp( HWND    hwndFrame,
            PSZ     pszFileName,
            HMODULE hmod,
            USHORT  usTableID,
            PSZ     pszWindowTitle )
/**********************************/
{
   PPIB     ppib;
   PTIB     ptib;
   HELPINIT hi;
   PSZ      pszExt;
   CHAR     szName[ CCHMAXPATH ];
   HWND     hwndHelp;

   if (( pszFileName == NULL ) || ( *pszFileName ))
   {
      DosGetInfoBlocks( &ptib, &ppib );

      DosQueryModuleName( ppib->pib_hmte, sizeof( szName ), szName );

      pszExt = strrchr( szName, '.' );
      if ( pszExt )
      {
         strcpy( pszExt, ".hlp" );
      }
      else
      {
         strcat( szName, ".hlp" );
      }

      pszFileName = szName;
   }

   hi.cb                       = sizeof( HELPINIT );
   hi.ulReturnCode             = 0;
   hi.pszTutorialName          = NULL;
   hi.phtHelpTable             = ( PHELPTABLE ) MAKELONG( usTableID, 0xffff );
   hi.hmodHelpTableModule      = hmod;
   hi.hmodAccelActionBarModule = NULLHANDLE;
   hi.idAccelTable             = 0;
   hi.idActionBar              = 0;
   hi.pszHelpWindowTitle       = pszWindowTitle;
   hi.fShowPanelId             = CMIC_HIDE_PANEL_ID;
   hi.pszHelpLibraryName       = pszFileName;

   hwndHelp = WinCreateHelpInstance( WinQueryAnchorBlock( hwndFrame ), &hi );
   if ( hwndHelp != NULLHANDLE )
   {
      WinAssociateHelpInstance( hwndHelp, hwndFrame );
      return ( hwndHelp );
   }
   else
   {
      return ( NULLHANDLE );
   }
}

/**
 * Dispose of a help table instance.
 *
 * Entry:
 *
 *    hwndHelp - A help table handle from "CreateHelp()."
 *
 * Exit:
 *
 *    Nothing.
 */
void
DestroyHelp( HWND hwndHelp )
/**************************/
{
   if ( hwndHelp != NULLHANDLE )
   {
      WinAssociateHelpInstance( NULLHANDLE, hwnd );
      WinDestroyHelpInstance( hwndHelp );
   }
}

/**
 * Change the icon used on an icon-button control.
 *
 * Entry:
 *
 *    hwndButton - The button handle.
 *    hicon      - The icons handle.
 *
 * Exit:
 *
 *    Nothing.
 */
void
SetButtonIcon( HWND     hwndButton,
               HPOINTER hicon      )
/**********************************/
{
   WNDPARAMS wp;
   BTNCDATA  bd;

   wp.fsStatus  = WPM_CTLDATA;
   wp.cbCtlData = sizeof( BTNCDATA );
   wp.pCtlData  = &bd;

   WinSendMsg( hwndButton, WM_QUERYWINDOWPARAMS, MPFROMP( &wp ), MPVOID );
   bd.hImage = hicon;
   WinSendMsg( hwndButton, WM_SETWINDOWPARAMS, MPFROMP( &wp ), MPVOID );
}

/**
 * Launch ( spawn ) another application.
 *
 * Entry:
 *
 *    hwnd           - The current window's handle.
 *    pszProgram     - The pathname of the program to launch.
 *    pszOptions     - Command-line options (if any) to pass to the spawned app.
 *    pszDirectory   - The path to launch the app from; i.e., this will be
 *                     the current directory at the time of launch.
 *
 * Exit:
 * 
 *    A handle to the launched app if successful.
 *
 *    NOTE:
 *
 *       The launching window ( hwnd ) will receive a WM_APPTERMINATENOTIFY
 *       message if still running when the spawned app terminates.
 *
 */
HWND
LaunchApplication( HWND    hwnd,
                   PSZ     pszProgram,
                   PSZ     pszOptions,
                   PSZ     pszDirectory )
/***************************************/
{
   PROGDETAILS pd = { 0 };

   pd.Length                        = sizeof( pd );
   pd.progt.progc                   = PROG_DEFAULT;
   pd.pszExecutable                 = pszProgram;
   pd.pszStartupDir                 = pszDirectory;
   pd.pszParameters                 = pszOptions;
   pd.swpInitial.hwndInsertBehind   = HWND_TOP;
   pd.swpInitial.fl                 = SWP_ACTIVATE | SWP_SHOW;
   pd.swpInitial.hwnd               = hwnd;

   return ( WinStartApp( hwnd, &pd, NULL, NULL, SAF_INSTALLEDCMDLINE ));
}

/**
 * Prevent more than one instance of an application from running.
 *
 * Entry:
 *
 *    pszSemName  - A unique semaphore name to identify this application.
 *                  This is a shared semaphore, so the name MAY NOT be
 *                  NULL or empty. As with all OS/2 semaphores, the
 *                  name must begin with "\SEM32\."
 *
 *                  E.g.: "\\SEM32\\MYAPP"
 *
 *    fSwitch     - If TRUE, and another instance of the app is found
 *                  to be running, we'll use the task manager to
 *                  switch to the running version. Otherwise just
 *                  return.
 *
 * Exit:
 *
 *    A mutex semaphore handle if this is the only instance of
 *    the app running, otherwise, NULLHANDLE, indicating that
 *    another copy exists.
 */
HMTX
OneInstance( PSZ  pszSemName,
             BOOL fSwitch    )
/****************************/
{
   HMTX hmtx;

   if ( DosCreateMutexSem( pszSemName, &hmtx, DC_SEM_SHARED, TRUE ) == NO_ERROR )
   {
      return ( hmtx );
   }

   hmtx = NULLHANDLE;

   if ( DosOpenMutexSem( pszSemName, &hmtx ) == NO_ERROR )
   {
      if ( fSwitch )
      {
         PID     pid = 0;
         TID     tid = 0;
         ULONG   ulJunk;

         if ( DosQueryMutexSem( hmtx, &pid, &tid, &ulJunk ) == NO_ERROR )
         {
            HSWITCH hswitch;

            hswitch = WinQuerySwitchHandle( NULLHANDLE, pid );
            if ( hswitch != NULLHANDLE )
            {
               WinSwitchToProgram( hswitch );
            }
         }
      }

      DosCloseMutexSem( hmtx );
   }

   return ( NULLHANDLE );
}

/**
 * Load a menu.
 *
 * Entry:
 *
 *    hwnd     - The parent/owner of the menu. May be HWND_OBJECT.
 *    hmod     - The module containing the menu resource. NULLHANDLE == current.
 *    usResID  - The resource I.D.
 *
 * Exit:
 *
 *    A handle to the menu. If the parent/owner was NOT HWND_OBJECT,
 *    this routine will attempt to associate the loaded menu with
 *    the current window.
 */
HWND
LoadMenu( HWND    hwnd,
          HMODULE hmod,
          USHORT  usResID )
/*************************/
{
   HWND hmenu;

   hmenu = WinLoadMenu( hwnd, hmod, usResID );
   if (( hmenu != NULLHANDLE ) && ( hwnd != HWND_OBJ ))
   {
      WinSendMsg( hwnd, WM_UPDATEFRAME, MPFROMLONG( FID_MENU ), MPVOID );
   }

   return ( hmenu );
}

/**
 * Load an icon and make it the current window's.
 *
 * Entry:
 *
 *    hwnd     - The application's frame window handle.
 *    hmod     - The module containing the resource - NULLHANDLE == current.
 *    usResID  - The resource I.D.
 *
 * Exit:
 *
 *    A handle to the icon. The routine will "attach" the icon to the window
 *    if it can.
 */
HPOINTER
LoadIcon( HWND    hwnd,
          HMODULE hmod,
          USHORT  usResID )
/*************************/
{
   HPOINTER hicon;

   hicon = WinLoadPointer( HWND_DESKTOP, hmod, usResID );
   if ( hicon != NULLHANDLE )
   {
      WinSendMsg( hwnd, WM_SETICON, MPFROMLONG( hicon ), MPVOID );
   }

   return ( hicon );
}

/**
 * Load an accelerator table and connect it to the window.
 *
 * Entry:
 *
 *    hwnd     - The application's frame window handle.
 *    hmod     - The module containing the resource - NULLHANDLE == current.
 *    usResID  - The resource I.D.
 *
 * Exit:
 *
 *    A handle to the accelerator table. The routine will "attach" the 
 *    table to the window if it can.
 */
HACCEL
LoadAccelerator( HWND    hwnd,
                 HMODULE hmod,
                 USHORT  usResID )
/********************************/
{
   HACCEL haccel;
   HAB    hab;

   hab    = WinQueryAnchorBlock( hwnd );
   haccel = WinLoadAccelTable( hab, hmod, usResID );
   if ( haccel != NULLHANDLE )
   {
      WinSetAccelTable( hab, haccel, hwnd );
   }

   return ( haccel );
}




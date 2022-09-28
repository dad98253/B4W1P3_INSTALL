/*----------------------------------------------------------------------------*\
|   install.c - A template for a Windows application installer 		          |
|                                                                              |
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
|                                                                              |
|   i n c l u d e   f i l e s                                                  |
|                                                                              |
\*----------------------------------------------------------------------------*/

#include "wslib\sulib.h"
#include "wslib\ws.h"
#include "wslib\progdde.h"
#include "wslib\gauge.h"
#include "install.h"

/*----------------------------------------------------------------------------*\
|                                                                              |
|   g l o b a l   v a r i a b l e s                                            |
|                                                                              |
\*----------------------------------------------------------------------------*/

char	szCaption[] = "Bible for Windows Setup";
char    szNull[]    = "";
HWND    hwndWS;
HANDLE	hInstWS;
BOOL	fMono;
PSTR    szText;
char    szString[MAXSTR];
int	dyChar;
int	dxChar;
WORD	capsScreen;
WORD	fExit;
char    bufTmp[MAXSTR];
PSTR	pErrMsg;
long int liTotalSize;
BOOL bCheckForVirus=1;


/*----------------------------------------------------------------------------*\
|                                                                              |
|   f u n c t i o n   d e f i n i t i o n s                                    |
|                                                                              |
\*----------------------------------------------------------------------------*/

LONG FAR PASCAL AppWndProc (HWND hwnd, unsigned uiMessage, WORD wParam, LONG lParam);
void PRIVATE infDlgFixup(HWND hwnd);
LONG NEAR PASCAL AppCommand(HWND hwnd, unsigned msg, WORD wParam, LONG lParam);
BOOL FAR PASCAL AppExit(HWND hDlg, unsigned uiMessage, WORD wParam, LONG lParam);
BOOL PRIVATE fnCheckDiskSpace(void);

/*----------------------------------------------------------------------------*\
|   AppInit( hInst, hPrev)                                                     |
|                                                                              |
|   Description:                                                               |
|	This procedure is called when the application is first loaded into         |
|	memory.  It performs any initialization tasks that need to be done only     |
|  for the first instance of the application.                                  |
|                                                                              |
|   Arguments:                                                                 |
|	hInstance	instance handle of current instance                             |
|	hPrev		instance handle of previous instance                               |
|                                                                              |
|   Returns:                                                                   |
|	TRUE if successful, FALSE if not                                            |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL AppInit(HANDLE hInst,HANDLE hPrev,WORD sw,LPSTR szCmdLine)
{
    WNDCLASS    cls;
    int         dx,dy;
    char        ach[80];
    HMENU       hmenu;
    HDC         hdc;
    OFSTRUCT	os;
    char        szAppName[MAXSTR];
    char        buf[MAXSTR];
    RECT        rc;
    TEXTMETRIC  tm;
    PINF	pinf;

#ifdef DEBUG
    dprintf("%C");
#endif
    bCheckForVirus = GetProfileInt("Bible4W", "bCheckForVirus", 0 );

    /* Save the instance handle for the DialogBox */
    hInstWS = hInst;

    /* Display the hourglass cursor */
    wsStartWait();

    if (OpenFile(wsLoadSz(IDS_INFNAME,NULL), &os, OF_EXIST) == -1) {
        wsEndWait();
	MessageBox(NULL, wsLoadSz(IDS_NOINF, NULL), szCaption, MB_OK | MB_ICONEXCLAMATION);
	return FALSE;
    }

//    GlobalCompact((LONG)(-1));	  // help infOpen() succeed

    pinf = infOpen(os.szPathName);

    wsEndWait();

    WinAssert(pinf);
    if (!pinf) {
	MessageBox(NULL, wsLoadSz(IDS_NOINFMEM, NULL), szCaption, MB_OK | MB_ICONEXCLAMATION);
	return FALSE;
    }
    
    hdc  = GetDC(NULL);
    fMono = GetDeviceCaps(hdc, NUMCOLORS) == 2;
    capsScreen = GetDeviceCaps(hdc, RASTERCAPS);
    GetTextMetrics(hdc,&tm);
    dyChar = tm.tmHeight;
    dxChar = tm.tmAveCharWidth;
    ReleaseDC(NULL,hdc);

    if (!hPrev) {

       /* Register a class for the main application window */

        cls.hCursor        = LoadCursor(NULL,IDC_ARROW);
        cls.hIcon          = NULL; //LoadIcon(hInst,MAKEINTATOM(ID_APP));
        cls.lpszMenuName   = NULL;
        cls.lpszClassName  = MAKEINTATOM(ID_APP);
        cls.hbrBackground  = (HBRUSH)COLOR_WINDOW + 1;
        cls.hInstance      = hInst;
        cls.style          = CS_BYTEALIGNCLIENT | CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
        cls.lpfnWndProc    = AppWndProc;
        cls.cbWndExtra     = 0;
        cls.cbClsExtra     = 0;

        if (!RegisterClass(&cls))
	    return FALSE;
    }

    hwndWS = CreateWindow (MAKEINTATOM(ID_APP),     // Class name
                            szAppName,              // Caption
                            WS_OVERLAPPEDWINDOW,    // Style bits
                            0,0,0,0,                // Position, size
                            (HWND)NULL,             // Parent window (no parent)
                            (HMENU)NULL,            // use class menu
                            (HANDLE)hInst,          // handle to window instance
                            (LPSTR)NULL             // no params to pass on
                            );


    /* Create the "stext" class used in progress dialogs */
    if (!ControlInit(hPrev,hInst))
    	return FALSE;

    if (!ProInit(hPrev,hInst))
    	return FALSE;

    if (!ddeInit(hInst, hPrev))
    	return FALSE;

    fExit = 0;

/*
 *  szSetupInf   is the directory or file name where SETUP.INF can be found
 *
 *  szSetupPath  is the directory to which application files will be copied        
 *
 *  szDiskPath   is the directory where the root of the setup disks are
 *
 */

    DosCwd(szDiskPath);
    lstrcpy(szSetupInf,szDiskPath);

    return TRUE;
}

/*----------------------------------------------------------------------------*\
|   WinMain( hInst, hPrev, lpszCmdLine, cmdShow )                              |
|                                                                              |
|   Description:                                                               |
|                                                                              |
|   Arguments:                                                                 |
|	hInst		    instance handle of this instance of the application            |
|	hPrev		    instance handle of previous instance. NULL if this is the      |
|                   instance of the application                                |
|  szCmdLine    null-terminated command line                                   |
|  cmdShow      specifies how the application's window is initially displayed  |
|                                                                              |
|   Returns:                                                                   |
|       The exit code as specified in the WM_QUIT message.                     |
|                                                                              |
\*----------------------------------------------------------------------------*/
int PASCAL WinMain(HANDLE hInst, HANDLE hPrev, LPSTR szCmdLine, WORD sw)
{
    char    szLocalTmp[MAXSTR];

    /* Call the initialization procedure */

    if (!AppInit(hInst,hPrev,sw,szCmdLine))
        return FALSE;

    /* Loop until the user specifies a destination with sufficent disk space */

    while(TRUE) {
       if (! fDialog("WINSETUP", hwndWS, wsInstallDlg) ) {
          DestroyWindow(hwndWS);         // Kill the appllication, right now!
          return TRUE;
       }
       if (! fnCheckDiskSpace() )
          fnOkMsgBox(IDS_NEEDROOM);
       else
          break;
    }

    ProOpen(hwndWS, NULL);

    /* Build groups only if all the copy tasks are successful. */

    if ( wsCopy(wsLoadSz(IDS_WINCOPY,szLocalTmp)) ) {
       wsProgman(hwndWS);

       /* Inform user that the copy process is complete. */
       MessageBox(NULL,wsLoadSz(IDS_FINISHED,NULL),szCaption,MB_OK);
       return TRUE;
    }
    return FALSE;
}

/* BOOL PRIVATE fnCheckDiskSpace(void);
 *
 * This function checks disk space on the specified destination drive
 * against the minimum disk space requirement specified in the .INF file.
 *
 * Arguments: None.
 *
 * Returns:
 *
 *   Boolean value:
 *   TRUE if the current destination provides sufficent disk space.
 *   FALSE if the current destination does not provide sufficient disk space.
 *
 */
BOOL PRIVATE fnCheckDiskSpace(void)
{
   char    szLocalTmp[MAXSTR];

   infLookup(wsLoadSz(IDS_DISKSPACE,NULL),szLocalTmp);

   if ( DosDiskFreeSpace(szSetupPath[0] - '@') >=
			  ( liTotalSize = atoi(szLocalTmp) ) )
      return TRUE;
   else
      return FALSE;

}

/*----------------------------------------------------------------------------*\
|   AppWndProc( hwnd, uiMessage, wParam, lParam )                              |
|                                                                              |
|   Description:                                                               |
|       The window proc for the application's main window.  This processes all |
|       of the parent window's messages.                                       |
|                                                                              |
|   Arguments:                                                                 |
|       hwnd            window handle for the window			                   |
|       uiMessage       message number                                         |
|       wParam          message-dependent                                      |
|       lParam          message-dependent                                      |
|                                                                              |
|   Returns:                                                                   |
|       0 if processed, nonzero if ignored                                     |
|                                                                              |
\*----------------------------------------------------------------------------*/
LONG FAR PASCAL AppWndProc(hwnd, msg, wParam, lParam)
    HWND     hwnd;
    unsigned msg;
    WORD     wParam;
    long     lParam;
{
    PAINTSTRUCT ps;
    BOOL        f;
    HDC         hdc;

    switch (msg) {
        case WM_COMMAND:
            return AppCommand(hwnd,msg,wParam,lParam);

	case WM_DESTROY:
	    PostQuitMessage(0);
	    break;

    }
    return DefWindowProc(hwnd,msg,wParam,lParam);
}

/* AppCommand (hwnd, msg, wParam, lParam)
 *
 * This function handles commands passed to main application windows.
 *
 */
LONG NEAR PASCAL AppCommand (hwnd, msg, wParam, lParam)
    HWND     hwnd;
    unsigned msg;
    WORD     wParam;
    long     lParam;
{
    char szTmpStr[256];

    switch(wParam)
    {
	case ID_EXITSETUP:
            if ( fnErrorMsg(IDS_EXITMSG) ) {
               ProClose();
               DestroyWindow(hwndWS);
            }
            break;
    }
    return 0L;
}

/* fnErrorMsg(int ID_Error_Message);
 *
 * This function displays a system-modal message box and
 * lets user choose yes or no in response to the message box.
 *
 * Arguments:
 *
 *    ID_Error_Message       Resource ID of message to be displayed in the
 *                           message box.
 * Returns:
 *
 *    TRUE if the user chooses the Yes button.
 *    FALSE if the user chooses the No button.
 *
 */
BOOL PUBLIC fnErrorMsg(ID_Error_Message)
int    ID_Error_Message;
{
   char   szTmpStr[256];

   wsLoadSz(ID_Error_Message,szTmpStr);

   if ( MessageBox(NULL,szTmpStr,wsLoadSz(IDS_EXITCAPTION,NULL),STD_EXIT_MSGBOX) == IDYES )
      return TRUE;
   else
      return FALSE;
}

/* fnOkMsgBox(int ID_Error_Message);
 *
 * This function displays a system-modal message box and
 * lets user choose yes or no in response to the message box.
 *
 * Arguments:
 *
 *    ID_Error_Message       Resource ID of message to be displayed in the
 *                           message box.
 * Returns:
 *
 *    Nothing.
 *
 */
VOID PUBLIC fnOkMsgBox(ID_Error_Message)
int    ID_Error_Message;
{
   char   szTmpStr[256];

   wsLoadSz(ID_Error_Message,szTmpStr);

   MessageBox(NULL,szTmpStr,wsLoadSz(IDS_EXITCAPTION,NULL),STD_OK_MSGBOX);
}

/*----------------------------------------------------------------------------*\
|   fDialog(id,hwnd,fpfn)                                                      |
|                                                                              |
|   Description:                                                               |
|	This function displays a dialog box and returns the exit code.              |
|	The function passed will have a proc instance made for it.                  |
|                                                                              |
|	This also handles special keyboard input by calling CheckSpecialKeys().     |
|                                                                              |
|                                                                              |
|   Arguments:                                                                 |
|	id		resource id of dialog to display                                      |
|	hwnd		parent window of dialog                                            |
|	fpfn		dialog message function                                            |
|                                                                              |
|   Returns:                                                                   |
|	exit code of dialog (what was passed to EndDialog)                          |
|                                                                              |
\*----------------------------------------------------------------------------*/
int FAR fDialog(LPSTR id, HWND hwnd, FARPROC fpfn)
{
    int   result = 0;
    FARPROC lpfn;
//    LPSTR lpstemp;

    if ( ( lpfn = MakeProcInstance ( fpfn , hInstWS ) ) == NULL ) goto ERR_EXIT;
//    lpstemp = MAKEINTRESOURCE(id);
//    result = DialogBox(hInstWS, lpstemp , hwnd, lpfn);
    result = DialogBox(hInstWS, id , hwnd, lpfn);
    FreeProcInstance(lpfn);

ERR_EXIT:
    return result;
}

/*----------------------------------------------------------------------------*\
|   wsDlgInit(hDlg)                                                            |
|                                                                              |
|   Handle the init message for a dialog box.                                  |
|                                                                              |
\*----------------------------------------------------------------------------*/
void PUBLIC wsDlgInit(HWND hDlg)
{
    RECT rc;

    /*
     *   Center the dialog.
     */
    GetWindowRect(hDlg,&rc);
    SetWindowPos(hDlg,NULL,
        (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
        0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

    infDlgFixup(hDlg);

    // enable hwndWS to allow keyboard input

    if (hwndWS == GetParent(hDlg))
        EnableWindow(hwndWS,TRUE);
}

/*----------------------------------------------------------------------------*\
|   infDlgFixup()                                                              |
|                                                                              |
|   If the dialog caption text or the text of any control has the form         |
|                                                                              |
|	                     section.text                                           |
|                                                                              |
|   then this function changes the text to the text found in APPSETUP.INF.	    |
|                                                                              |
\*----------------------------------------------------------------------------*/
void PRIVATE infDlgFixup(HWND hwnd)
{

    if (GetWindowText(hwnd,bufTmp,sizeof(bufTmp)))
    {
        if (infLookup(bufTmp,bufTmp))
            SetWindowText(hwnd,bufTmp);
    }

    hwnd = GetWindow(hwnd,GW_CHILD);
    while (hwnd)
    {
	infDlgFixup(hwnd);
	hwnd = GetWindow(hwnd,GW_HWNDNEXT);
    }
}

/*----------------------------------------------------------------------------*\
|   wsStartWait()                                                              |
|                                                                              |
|   Displays an hourglass cursor.                                              |
|                                                                              |
\*----------------------------------------------------------------------------*/
void PUBLIC wsStartWait()
{
        SetCursor(LoadCursor(NULL,MAKEINTATOM(IDC_WAIT)));
}

/*----------------------------------------------------------------------------*\
|   wsEndWait()                                                                |
|                                                                              |
|   Restores the cursor to its original shape.                                 |
|                                                                              |
\*----------------------------------------------------------------------------*/
void PUBLIC wsEndWait()
{

        SetCursor(LoadCursor(NULL,MAKEINTATOM(IDC_ARROW)));
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  PRIV text control that uses ExtTextOut(), thus avoiding flicker.         */
/*                                                                           */
/*---------------------------------------------------------------------------*/

LONG FAR PASCAL fnText( hwnd, msg, wParam, lParam )
    HWND hwnd;
    unsigned msg;
    WORD wParam;
    LONG lParam;
{
    PAINTSTRUCT ps;
    RECT rc;
    char        ach[128];
    int  len;

    switch (msg)
    {
    case WM_SETTEXT:
        DefWindowProc(hwnd, msg, wParam, lParam);
        InvalidateRect(hwnd,NULL,FALSE);
        UpdateWindow(hwnd);
        return 0L;

    case WM_ERASEBKGND:
        return 0L;

    case WM_PAINT:
        BeginPaint(hwnd, &ps);
        GetClientRect(hwnd,&rc);

        len = GetWindowText(hwnd,ach,sizeof(ach));
        SetBkColor(ps.hdc,GetSysColor(COLOR_WINDOW));
        SetTextColor(ps.hdc,GetSysColor(COLOR_WINDOWTEXT));
        ExtTextOut(ps.hdc,0,0,ETO_OPAQUE,&rc,ach,len,NULL);

        EndPaint(hwnd, &ps);
        return 0L;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/*----------------------------------------------------------------------------*\
|   wsLoadSz()                                                                 |
|                                                                              |
|   Description:                                                               |
|       Loads a string from our resource file                                  |
|                                                                              |
|       ids     ID of string to load                                           |
|       pch     buffer in which to place the string. If pch is NULL, the       |
|               string will be placed in a global buffer.                      |
|                                                                              |
|   Returns:                                                                   |
|       near pointer to the loaded string                                      |
|                                                                              |
\*----------------------------------------------------------------------------*/
PSTR PUBLIC wsLoadSz(int ids, PSTR pch)
{
    if (pch == NULL)
	pch = bufTmp;

    LoadString(hInstWS,ids,pch,MAXSTR);

    return pch;
}

/*----------------------------------------------------------------------------*\
|   wsInstallDlg( hDlg, uiMessage, wParam, lParam )                            |
|                                                                              |
|   Returns:                                                                   |
|       TRUE if message has been processed, else FALSE                         |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL EXPORT wsInstallDlg( hDlg, uiMessage, wParam, lParam )
    HWND     hDlg;
    unsigned uiMessage;
    WORD     wParam;
    long     lParam;
{
    WORD    fOptions;
    HWND    hEditFld;
    char    szTmpBuf[MAXSTR];

    switch (uiMessage)
    {
	case WM_SYSCOMMAND:		// suppress taskman
	    if (wParam == SC_TASKLIST)
	    	return TRUE;
	    break;

	case WM_COMMAND:
	    switch (wParam)
	    {
                case ID_OK:
                    GetDlgItemText(hDlg,ID_EDIT1,szSetupPath,MAXPATHLEN);
                    EndDialog(hDlg, TRUE);
		    break;

                case ID_CANCEL:
		    // make this the same as pressing F3
                    if ( fnErrorMsg(IDS_EXITMSG) )
	               EndDialog(hDlg, FALSE);
                    break;
	    }
	    return TRUE;

        /*  On init we need to:
         *     - Put default text in edit field.
         *     - Set focus to edit field.
         *     - Place cursor at end of edit field text.
         */

	case WM_INITDIALOG:
            wsDlgInit(hDlg);
            hEditFld = GetDlgItem(hDlg,ID_EDIT1);
            infLookup(wsLoadSz(IDS_DEFAULT_DIR,NULL),szTmpBuf);
            SetDlgItemText(hDlg,ID_EDIT1,szTmpBuf);
            SetFocus(hEditFld);
            SendMessage(hEditFld,EM_SETSEL,0,MAKELONG(lstrlen(szTmpBuf),lstrlen(szTmpBuf)));
	    return FALSE;
    }
    return FALSE;
}

/*----------------------------------------------------------------------------*\
|   wsErrorDlg( hDlg, uiMessage, wParam, lParam )			       |
|                                                                              |
|   Arguments:                                                                 |
|       hDlg            window handle of About dialog's window                   |
|       uiMessage       message number                                         |
|       wParam          message-dependent                                      |
|       lParam          message-dependent                                      |
|                                                                              |
|   Returns:                                                                   |
|       TRUE if message has been processed, else FALSE                         |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL FAR PASCAL wsErrorDlg(HWND hDlg, unsigned uiMessage, WORD wParam, long lParam)
{
    switch (uiMessage)
    {
	case WM_COMMAND:
	    switch (wParam)
	    {
		case ID_RETRY:
                    EndDialog(hDlg,FC_RETRY);
		    break;

		case ID_ABORT:
		    EndDialog(hDlg,FC_ABORT);
		    break;

		case ID_IGNORE:
		    EndDialog(hDlg,FC_IGNORE);
		    break;
	    }
	    return TRUE;

        case WM_INITDIALOG:
	    SetDlgItemText(hDlg, ID_STATUS1, pErrMsg);
	    wsDlgInit(hDlg);
            return TRUE;
    }
    return FALSE;
}

#ifdef DEBUG
/* FAR _Assert(char*,int);
 *
 * Called as a result of an assertion failure. Will print out an error
 * dialog containing the file and line number at which the assertion failure
 * occured.
 *
 * ENTRY: Only from an assertion failure.
 * EXIT : Fatal Error (exit to MS-DOS).
 *
 */
int FAR _Assert(PSTR szFile, int iLine)
{
    int     id;
    char    buf[128];

    fsprintf(buf,"%s : %d",szFile,iLine);
    id = MessageBox(NULL,buf,"WinAssert",MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION|MB_DEFBUTTON3|MB_SYSTEMMODAL);
    switch (id)
    {
        case IDABORT:
            DosExit(-1);  /* Terminate application immediately. */
        case IDIGNORE:
            break;
    }
    return 0;
}
#endif

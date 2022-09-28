/*----------------------------------------------------------------------------*\
|   PROGRESS.C -							       |
|	     Text.							       |
|                                                                              |
|   Description:                                                               |
|                                                                              |
|   Notes:                                                                     |
|                                                                              |
|   History:                                                                   |
|	11/01/87 Toddla     Created					       |
|                                                                              |
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
|                                                                              |
|   g e n e r a l   c o n s t a n t s                                          |
|                                                                              |
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
|                                                                              |
|   i n c l u d e   f i l e s                                                  |
|                                                                              |
\*----------------------------------------------------------------------------*/

#include "winenv.h"
#include "ws.h"
#include "..\install.h"
#include "gauge.h"

/*----------------------------------------------------------------------------*\
|                                                                              |
|   g l o b a l   v a r i a b l e s                                            |
|                                                                              |
\*----------------------------------------------------------------------------*/

static HWND	ghWnd = NULL;
static int	iCnt = 0;
static FARPROC  fpxProDlg;
static DWORD    rgbFG;
static DWORD    rgbBG;

#define BAR_RANGE 0
#define BAR_POS   2

#define BAR_SETRANGE  WM_USER+BAR_RANGE
#define BAR_SETPOS    WM_USER+BAR_POS
#define BAR_DELTAPOS  WM_USER+4

#ifndef COLOR_HIGHLIGHT
    #define COLOR_HIGHLIGHT	  (COLOR_APPWORKSPACE + 1)
    #define COLOR_HIGHLIGHTTEXT   (COLOR_APPWORKSPACE + 2)
#endif

#define COLORBG  rgbBG
#define COLORFG  rgbFG

/*----------------------------------------------------------------------------*\
|                                                                              |
|   f u n c t i o n   d e f i n i t i o n s                                    |
|                                                                              |
\*----------------------------------------------------------------------------*/

BOOL FAR PASCAL ProDlgProc(HWND, unsigned, WORD, LONG);
LONG FAR PASCAL ProBarProc(HWND, unsigned, WORD, LONG);

/*----------------------------------------------------------------------------*\
|                                                                              |
|   E x t e r n a l   d e f i n i t i o n s                                    |
|                                                                              |
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
|   ProInit( hPrev,hInst )						       |
|                                                                              |
|   Description:                                                               |
|       This is called when the application is first loaded into               |
|	memory.  It performs all initialization.			       |
|                                                                              |
|   Arguments:                                                                 |
|	hPrev	   instance handle of previous instance 		       |
|	hInst	   instance handle of current instance			       |
|                                                                              |
|   Returns:                                                                   |
|       TRUE if successful, FALSE if not                                       |
|                                                                              |
\*----------------------------------------------------------------------------*/

BOOL PUBLIC ProInit (hPrev,hInst)
    HANDLE hPrev;
    HANDLE hInst;
{
    WNDCLASS   rClass;

    if (!hPrev)
    {
       rClass.hCursor	     = LoadCursor(NULL,IDC_ARROW);
       rClass.hIcon	     = NULL;
       rClass.lpszMenuName   = NULL;
       rClass.lpszClassName  = PRO_CLASS;
       rClass.hbrBackground  = (HBRUSH)COLOR_WINDOW+1;
       rClass.hInstance      = hInst;
       rClass.style	     = CS_HREDRAW | CS_VREDRAW;
       rClass.lpfnWndProc    = ProBarProc;
       rClass.cbClsExtra     = 0;
       rClass.cbWndExtra     = 2*sizeof(WORD);

       if (!RegisterClass(&rClass))
          return FALSE;
    }

    if (fMono)
    {
        rgbBG = RGB(0,0,0);
        rgbFG = RGB(255,255,255);
    }
    else
    {
        rgbBG = RGB(0,0,255);
        rgbFG = RGB(255,255,255);
    }
    return TRUE;
}

void PUBLIC ProClear(HWND hDlg)
{
	if (!hDlg)
		hDlg = ghWnd;

	SetDlgItemText (hDlg, ID_STATUS1, szNull);
	SetDlgItemText (hDlg, ID_STATUS2, szNull);
	SetDlgItemText (hDlg, ID_STATUS3, szNull);
	SetDlgItemText (hDlg, ID_STATUS4, szNull);
}

/*---------------------------------------------------------------------------*\
|   ControlInit( hPrev,hInst )                                                 |
|                                                                              |
|   Description:                                                               |
|       This is called when the application is first loaded into               |
|       memory.  It performs all initialization.                               |
|                                                                              |
|   Arguments:                                                                 |
|       hPrev      instance handle of previous instance                        |
|       hInst      instance handle of current instance                         |
|                                                                              |
|   Returns:                                                                   |
|       TRUE if successful, FALSE if not                                       |
|                                                                              |
\*----------------------------------------------------------------------------*/

BOOL FAR PASCAL ControlInit (hPrev,hInst)
    HANDLE hPrev;
    HANDLE hInst;
{
    WNDCLASS    cls;

    if (!hPrev) {
        cls.hCursor        = LoadCursor(NULL,IDC_ARROW);
        cls.hIcon          = NULL;
        cls.lpszMenuName   = NULL;
        cls.lpszClassName  = "stext";
        cls.hbrBackground  = (HBRUSH)COLOR_WINDOW+1;
        cls.hInstance      = hInst;
        cls.style          = CS_HREDRAW | CS_VREDRAW;
        cls.lpfnWndProc    = fnText;
        cls.cbClsExtra     = 0;
        cls.cbWndExtra     = 0;

        if (! RegisterClass(&cls) )
           return FALSE;
    }
    
    return TRUE;
}

/*----------------------------------------------------------------------------*\
|   ProDlgProc( hWnd, uiMessage, wParam, lParam )			       |
|                                                                              |
|   Description:                                                               |
|	The window proc for the Progress dialog box			       |
|                                                                              |
|   Arguments:                                                                 |
|	hWnd		window handle for the dialog			       |
|       uiMessage       message number                                         |
|       wParam          message-dependent                                      |
|       lParam          message-dependent                                      |
|                                                                              |
|   Returns:                                                                   |
|       0 if processed, nonzero if ignored                                     |
|                                                                              |
\*----------------------------------------------------------------------------*/

BOOL EXPORT ProDlgProc( hDlg, uiMessage, wParam, lParam )
    HWND     hDlg;
    unsigned uiMessage;
    WORD     wParam;
    long     lParam;
{
    switch (uiMessage) {

       case WM_INITDIALOG:
          ProClear(hDlg);
          wsDlgInit(hDlg);
          return TRUE;

       case WM_COMMAND:
          if ( wParam == ID_CANCEL )
             PostMessage(hwndWS,WM_COMMAND,ID_EXITSETUP,0L);
          break;
    }
    return FALSE;
}

/*----------------------------------------------------------------------------*\
|   ProBarProc( hWnd, uiMessage, wParam, lParam )			       |
|                                                                              |
|   Description:                                                               |
|	The window proc for the Progress Bar chart			       |
|                                                                              |
|   Arguments:                                                                 |
|	hWnd		window handle for the dialog			       |
|       uiMessage       message number                                         |
|       wParam          message-dependent                                      |
|       lParam          message-dependent                                      |
|                                                                              |
|   Returns:                                                                   |
|       0 if processed, nonzero if ignored                                     |
|                                                                              |
\*----------------------------------------------------------------------------*/

LONG EXPORT ProBarProc( hWnd, uiMessage, wParam, lParam )
    HWND     hWnd;
    unsigned uiMessage;
    WORD     wParam;
    long     lParam;
{
    PAINTSTRUCT rPS;
    RECT	rc1,rc2;
    WORD	dx,dy,x;
    WORD	iRange,iPos;
    char	ach[30];
    DWORD	dwExtent;

    switch (uiMessage) {
        case WM_CREATE:
	    SetWindowWord (hWnd,BAR_RANGE,100);
	    SetWindowWord (hWnd,BAR_POS,0);
            return 0L;

	case BAR_SETRANGE:
	    SetWindowWord (hWnd,uiMessage-WM_USER,wParam);
	    InvalidateRect (hWnd,NULL,FALSE);
	    UpdateWindow(hWnd);
	    return 0L;

	case BAR_SETPOS:
   //	      SetWindowWord (hWnd,uiMessage-WM_USER,wParam);
	    SetWindowWord (hWnd,BAR_POS,wParam);
	    InvalidateRect (hWnd,NULL,FALSE);
	    UpdateWindow(hWnd);
            return 0L;

	case BAR_DELTAPOS:
	    iPos = GetWindowWord (hWnd,BAR_POS);
	    SetWindowWord (hWnd,BAR_POS,iPos+wParam);
	    InvalidateRect (hWnd,NULL,FALSE);
	    UpdateWindow(hWnd);
            return 0L;

        case WM_PAINT:
	    BeginPaint(hWnd,&rPS);
	    GetClientRect (hWnd,&rc1);
	    FrameRect(rPS.hdc,&rc1,GetStockObject(BLACK_BRUSH));
	    InflateRect(&rc1,-1,-1);
	    rc2 = rc1;
	    iRange = GetWindowWord (hWnd,BAR_RANGE);
            iPos   = GetWindowWord (hWnd,BAR_POS);

            if (iRange <= 0)
                iRange = 1;

	    if (iPos > iRange)	// make sure we don't go past 100%
	    	iPos = iRange;

	    dx = rc1.right;
	    dy = rc1.bottom;
	    x  = (WORD)((DWORD)iPos * dx / iRange) + 1;

	    wsprintf (ach,"%3d%%",(WORD)((DWORD)iPos * 100 / iRange));
	    dwExtent = GetTextExtent (rPS.hdc,ach,4);

	    rc1.right = x;
	    rc2.left  = x;

	    SetBkColor(rPS.hdc,COLORBG);
	    SetTextColor(rPS.hdc,COLORFG);
	    ExtTextOut (rPS.hdc,
		(dx-LOWORD(dwExtent))/2,(dy-HIWORD(dwExtent))/2,
		ETO_OPAQUE | ETO_CLIPPED,
		&rc1,
		ach,4,NULL);

            SetBkColor(rPS.hdc,COLORFG);
            SetTextColor(rPS.hdc,COLORBG);
	    ExtTextOut (rPS.hdc,
		(dx-LOWORD(dwExtent))/2,(dy-HIWORD(dwExtent))/2,
		ETO_OPAQUE | ETO_CLIPPED,
		&rc2,
		ach,4,NULL);

            EndPaint(hWnd,(LPPAINTSTRUCT)&rPS);
	    return 0L;
    }
    return DefWindowProc(hWnd,uiMessage,wParam,lParam);
}

/*----------------------------------------------------------------------------*\
|   ProOpen ()								       |
|                                                                              |
|   Description:                                                               |
|                                                                              |
|   Arguments:                                                                 |
|                                                                              |
|   Returns:                                                                   |
|       0 if processed, nonzero if ignored                                     |
|                                                                              |
\*----------------------------------------------------------------------------*/
HWND PUBLIC ProOpen(HWND hWnd, int id)
{
    if (id == NULL)
       id = DLG_PROGRESS;

    iCnt++;
    if (!ghWnd) {
       fpxProDlg  = MakeProcInstance ((FARPROC)ProDlgProc,hInstWS);
       ghWnd = CreateDialog(hInstWS,MAKEINTRESOURCE(id),hWnd,fpxProDlg);
       WinAssert(ghWnd);
       ShowWindow (ghWnd,SHOW_OPENWINDOW);
       UpdateWindow(ghWnd);
    }
    ProSetBarRange(100);
    ProSetBarPos(0);
    return ghWnd;
}   

/*----------------------------------------------------------------------------*\
|   ProClose () 							       |
|                                                                              |
|   Description:                                                               |
|                                                                              |
|   Arguments:                                                                 |
|                                                                              |
|   Returns:                                                                   |
|       0 if processed, nonzero if ignored                                     |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL PUBLIC ProClose()
{
   iCnt--;
   if (ghWnd && iCnt == 0) {
      DestroyWindow (ghWnd);
      FreeProcInstance (fpxProDlg);
      ghWnd = NULL;
   }
   return TRUE;
}

BOOL PUBLIC ProSetText (int i,LPSTR lpch)
{
    if (ghWnd) {
	SetDlgItemText (ghWnd,i,lpch);
	return TRUE;
    }
    return FALSE;
}

BOOL FAR cdecl ProPrintf (int i, LPSTR lpch, ...)
{
    char ach[200];
    if (ghWnd) {
        wvsprintf(ach, lpch, (LPSTR)(&lpch+1));
	SetDlgItemText(ghWnd, i, ach);
	return TRUE;
    }
    return FALSE;
}

BOOL PUBLIC ProSetBarRange (int i)
{
    if (ghWnd) {
        SendDlgItemMessage(ghWnd,ID_BAR,BAR_SETRANGE,i,0L);
	return TRUE;
    }
    return FALSE;
}

BOOL PUBLIC ProSetBarPos (int i)
{
    if (ghWnd) {
        SendDlgItemMessage(ghWnd,ID_BAR,BAR_SETPOS,i,0L);
	return TRUE;
    }
    return FALSE;
}

BOOL PUBLIC ProDeltaPos (int i)
{
    if (ghWnd) {
        SendDlgItemMessage(ghWnd,ID_BAR,BAR_DELTAPOS,i,0L);
	return TRUE;
    }
    return FALSE;
}

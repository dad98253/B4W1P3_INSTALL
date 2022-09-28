/*----------------------------------------------------------------------------*\
|   frogman.c     Module to Write a FrogMan Config File.                       |
|                                                                              |
|   History:                                                                   |
|	03/09/89 toddla     Created					       |
|                                                                              |
\*----------------------------------------------------------------------------*/

#include "sulib.h"
#include "..\install.h"
#include "ws.h"
#include "progdde.h"
#include "gauge.h"
#include <dde.h>

#ifdef DEBUG
BOOL fDebug = TRUE;
#endif

BOOL	bBuildGroups = FALSE;

BOOL PRIVATE WriteString(int fh, PSTR sz);
void PRIVATE SetProgmanSize( void );
void PUBLIC ActivateMain( void );
BOOL PUBLIC fmClose( void );

char szCreateCmd[] = "[creategroup(%s)]";

typedef struct {
    GROUPDEF    gd;
    int         fh;
}   GRP;

typedef GRP *PGRP;

char	szProgman[]	= "PROGMAN";
HWND	hwndDDE		= NULL;
HWND	hwndServer	= NULL;  // Currently active DDE conversation
BOOL	fInitiate	= FALSE;
BOOL	fAck		= FALSE;
HWND	hWndProgman	= NULL;		// global handel of progman window


BOOL PRIVATE ddeExec(HWND hwnd, LPSTR szCmd);
BOOL PRIVATE ddeWait(HWND hwnd);
HWND PRIVATE ddeSendInitiate(ATOM aApp, ATOM aTopic);
BOOL PRIVATE ddeTerminate (HWND hwnd);

long EXPORT ddeWndProc(HWND hwnd, unsigned uiMessage, WORD wParam, long lParam);

BOOL PRIVATE ddeTerminate (HWND hwnd)
{
    PostMessage (hwnd,WM_DDE_TERMINATE,hwndDDE,0L);
    if (hwnd == hwndServer)
       hwndServer = NULL;
    return TRUE;
}

// build the progman groups

BOOL PUBLIC wsProgman(HWND hwnd)
{
	PINF     pinf, pinfGroup;
	char	 szName[MAXSTR];
	char	 szGroupName[MAXSTR];
	char	 szExe[MAXSTR];
	char	 szPath[MAXSTR];
	char	 buf[MAXSTR];
        char     bufTmp[MAXSTR];
	int      iIcon;
	int	 root_len;
	int	 fh;
	int	 num_groups;


	SetProgmanSize();

	pinf = infFindSection(NULL,wsLoadSz(IDS_PROGMAN_GROUP,NULL));

	WinAssert(pinf);

	if (!pinf)
	    	return FALSE;

#ifdef DEBUG
	if (fDontCopy) {
	   if (!bBuildGroups) {
	      return TRUE;
	   }
	}
#endif

	ProOpen(hwnd, NULL);

        LoadString(hInstWS, IDS_BUILDPMG, bufTmp, MAXSTR);
        ProPrintf(ID_STATUS1, bufTmp);

	num_groups = infLineCount(pinf);

	ProSetBarRange(num_groups);

	// first build all the groups.

        while (pinf) {

		/* get group name and file name */
		infParseField(pinf, 1, szGroupName);

		dprintf(" Group:%s\n", szGroupName);

		ProPrintf(ID_STATUS2, szGroupName);

		// open, delete contents

		if (fmOpen(szGroupName, TRUE)) {

			for (pinfGroup = infFindSection(NULL, szGroupName); pinfGroup; pinfGroup = infNextLine(pinfGroup)) {

				*szExe = 0;

				// get optional icon source file

				infParseField(pinfGroup, 3, szExe);

				// get optional icon index #

				if (infParseField(pinfGroup, 4, buf))
					iIcon = (int)atoi(buf);	// convert to int
				else
					iIcon = 0;		// default icon index

				infParseField(pinfGroup, 1, szName);
				infParseField(pinfGroup, 2, buf);


				GenerateProgmanPath(buf, szPath);

				dprintf(" Item:%s %s %s %d\n", szName, szPath, szExe, iIcon);

				ProPrintf(ID_STATUS3, szName);

				fmAddItem(szName, szPath, *szExe ? (PSTR)szExe : (PSTR)NULL, iIcon);
			}

		}

		pinf = infNextLine(pinf);

		ProDeltaPos(1);
	}

	// now run through the groups again and minimize everthing
	// this does not have a second field (non minimize flag)

	pinf = infFindSection(NULL,wsLoadSz(IDS_PROGMAN_GROUP,NULL));

        while (pinf) {

		/* get group name and file name */
		infParseField(pinf, 1, szGroupName);

		if (!infParseField(pinf, 2, buf) || *buf == 0)
			fmMinimize(szGroupName);

		pinf = infNextLine(pinf);
	}

	ActivateMain();

	fmClose();

	dprintf("Progman has been closed\n");

	ProClose();

}

void PRIVATE SetProgmanSize()
{
	HDC hdc;
	int x1, y1, x2, y2;
	int xScreen, yScreen;
	char	buf[MAXSTR];

	#define MARGIN_SIZE(x)	((x) / 20)

	hdc = GetDC(NULL);
	xScreen = GetDeviceCaps(hdc, HORZRES);
	yScreen = GetDeviceCaps(hdc, VERTRES);
	ReleaseDC(NULL, hdc);

	x1 = MARGIN_SIZE(xScreen);
	y1 = MARGIN_SIZE(yScreen);

	x2 = xScreen - MARGIN_SIZE(xScreen);
	y2 = yScreen - 2*MARGIN_SIZE(yScreen);

	wsprintf(buf, "%d %d %d %d 1", x1, y1, x2, y2);

	WritePrivateProfileString("Settings", "Window", buf, "progman.ini");
}

void PUBLIC ActivateMain()
{
	char	szGroupName[MAXSTR];
	char	buf[MAXSTR];
	PINF	pinf;

	dprintf("ActivateMain()\n");

	// now activate the groups that weren't minimized

	pinf = infFindSection(NULL,wsLoadSz(IDS_PROGMAN_GROUP,NULL));

        while (pinf) {

		infParseField(pinf, 1, szGroupName);

		if (infParseField(pinf, 2, buf)) {
			fmActivate(szGroupName);
			break;
		}

		pinf = infNextLine(pinf);
	}
}

/****************************************************************************
 * generate an exe name for a progman item
 *
 * if on the path do not fully qualify
 * if not on the path fully qualify
 * if this is command.com use COMSPEC var first
 *
 ***************************************************************************/

void PUBLIC GenerateProgmanPath(PSTR filename, PSTR szPath)
{
	LPSTR lp;
	PSTR lpname;
	OFSTRUCT os;

	lpname = FileName(filename);

	// if we can open the unqualified file name then we know
	// it is on the path or in the windows directory.  In that
	// case don't fully qualify the path name.

//	  if (OpenFile(lpname, &os, OF_EXIST) != -1) {
//		  lstrcpy(szPath, lpname);	  // non fully qualified file name
//		  return;
//	  }
//	  else {
	   lstrcpy(szPath,szSetupPath);	// use fully qualified name
           lstrcat(szPath,"\\");
           lstrcat(szPath,lpname);
//	  }
}

LONG PUBLIC atoi(PSTR sz)
{
    LONG n = 0;

    while (ISDIGIT(*sz))
    {
	n *= 10;
	n += *sz - '0';
	sz++;
    }
    return n;
}

/*----------------------------------------------------------------------------*\
|   ddeInit (hInst, hPrev)						       |
|                                                                              |
|   Description:                                                               |
|       This is called when the application is first loaded into               |
|       memory.  It performs all initialization that doesn't need to be done   |
|       once per instance.                                                     |
|                                                                              |
|   Arguments:                                                                 |
|	hPrev	instance handle of previous instance			       |
|	hInst	instance handle of current instance			       |
|                                                                              |
|   Returns:                                                                   |
|       TRUE if successful, FALSE if not                                       |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL PUBLIC ddeInit(HANDLE hInst, HANDLE hPrev)
{
    WNDCLASS rClass;
    if (!hPrev) {
       rClass.hCursor	     = NULL;
       rClass.hIcon	     = NULL;
       rClass.lpszMenuName   = NULL;
       rClass.lpszClassName  = "ddeClass";
       rClass.hbrBackground  = NULL;
       rClass.hInstance      = hInst;
       rClass.style	     = 0;
       rClass.lpfnWndProc    = ddeWndProc;
       rClass.cbClsExtra     = 0;
       rClass.cbWndExtra     = 0;

       if (! RegisterClass(&rClass) )
          return FALSE;
    }
    /*
     * Create a window to handle our DDE mesage trafic
     */
    hwndDDE = CreateWindow("ddeClass", NULL, 0L, 0, 0, 0, 0,
			   (HWND)NULL,	      /* no parent */
			   (HMENU)NULL,       /* use class menu */
			   (HANDLE)hInst,     /* handle to window instance */
			   (LPSTR)NULL	      /* no params to pass on */
			  );
    return (BOOL)hwndDDE;
}

/*----------------------------------------------------------------------------*\
|   ddeWndProc( hWnd, uiMessage, wParam, lParam )			       |
|                                                                              |
|   Description:                                                               |
|                                                                              |
|   Arguments:                                                                 |
|       hWnd            window handle for the parent window                    |
|       uiMessage       message number                                         |
|       wParam          message-dependent                                      |
|       lParam          message-dependent                                      |
|                                                                              |
|   Returns:                                                                   |
|       0 if processed, nonzero if ignored                                     |
|                                                                              |
\*----------------------------------------------------------------------------*/
long EXPORT ddeWndProc(HWND hwnd, unsigned uiMessage, WORD wParam, long lParam )
{
    HANDLE	h;
//    LPDDEDATA	lpData;
    WORD	fRelease;

    switch (uiMessage) {
	case WM_DDE_TERMINATE:
		ddeTerminate (wParam);
		return 0L;

	case WM_DDE_ACK:
		if (fInitiate) {
		   hwndServer = wParam;
		   GlobalDeleteAtom(LOWORD(lParam));
		} else {
		   fAck = (LOWORD(lParam) & 0x8000);
		}
		GlobalDeleteAtom(HIWORD(lParam));
		return 0L;

#if 0

	case WM_DDE_DATA:
		h = LOWORD(lParam);
		lpData = (DDEDATA FAR *)GlobalLock(h);
		fRelease  = lpData->fRelease;
		if (lpData->fAckReq)
		   PostMessage(wParam,WM_DDE_ACK,hwnd,MAKELONG(0x8000,HIWORD(lParam)));
		else
		   GlobalDeleteAtom(HIWORD(lParam));
                GlobalUnlock(h);
                if (ghData)
                    DeleteData(ghData);

                if (fRelease)
                {
                    ghData = h;
                }
                else
                {
                    ghData = CopyData(h);
                }

		return 0L;
#endif

    }
    return DefWindowProc(hwnd,uiMessage,wParam,lParam);
}




HWND PRIVATE ddeSendInitiate(ATOM aApp, ATOM aTopic)
{
    fInitiate = TRUE;
    SendMessage((HWND)-1, WM_DDE_INITIATE, hwndDDE, MAKELONG(aApp, aTopic));
    fInitiate = FALSE;
    return hwndServer;
}

HWND PRIVATE ddeInitiate(LPSTR szApp, LPSTR szTopic)
{
    ATOM   aApp;
    ATOM   aTopic;
    HWND   hwnd;

    aApp    = GlobalAddAtom(szApp);
    aTopic  = GlobalAddAtom(szTopic);

    //	Try to start a conversation with the requested app
    hwnd = ddeSendInitiate(aApp, aTopic);

    // perhaps he is not running, try to exec him
    if (!hwnd) {
		
    	dprintf("WinExec:%ls\n", szApp);
	if (!WinExec(szApp, SW_SHOWNORMAL))
    		return NULL;
	hwnd = ddeSendInitiate(aApp, aTopic);
    }

    GlobalDeleteAtom(aApp);
    GlobalDeleteAtom(aTopic);
    return hwnd;
}


BOOL PRIVATE ddeWait(HWND hwnd)
{
    MSG    rMsg;
    BOOL   fResult;

    LockData(0);
    while (GetMessage(&rMsg, NULL, WM_DDE_FIRST, WM_DDE_LAST)) {
	TranslateMessage(&rMsg);
	DispatchMessage (&rMsg);
	if (rMsg.wParam == hwnd) {   /* DDE message from proper window */
	   switch (rMsg.message) {
	      case WM_DDE_ACK:
		  fResult = fAck;
		  goto exit;

	      case WM_DDE_DATA:
		  fResult = TRUE;
		  goto exit;
	   }
	}
    }
exit:
    UnlockData(0);
    return fResult;
}


BOOL PRIVATE ddeExec(HWND hwnd, LPSTR szCmd)
{
    HANDLE hCmd;
    LPSTR  lpCmd;
    BOOL   bResult = FALSE;

    dprintf("ddeExec:%ls\n", szCmd);

    if (hCmd = GlobalAlloc(GMEM_MOVEABLE|GMEM_SHARE, (LONG)lstrlen(szCmd)+1)) {
    	
       lpCmd = GlobalLock(hCmd);

       if (!lpCmd) {
           dprintf("lock failed!\n");
	   goto ERR_FREE;
       }

       lstrcpy (lpCmd,szCmd);
       GlobalUnlock(hCmd);
       PostMessage(hwnd, WM_DDE_EXECUTE, hwndDDE, MAKELONG(NULL,hCmd));
       bResult = ddeWait(hwnd);

ERR_FREE:
       GlobalFree (hCmd);
    }
    return bResult;
}



BOOL PUBLIC fmActivate(PSTR szGroup)
{
	char buf[MAXSTR];

	WinAssert(hWndProgman);

	wsprintf(buf, szCreateCmd, (LPSTR)szGroup);

	return ddeExec(hWndProgman, buf);
}

BOOL PUBLIC fmMinimize(PSTR szGroup)
{
	char buf[MAXSTR];

	WinAssert(hWndProgman);

	wsprintf(buf, "[showgroup(%s,2)]", (LPSTR)szGroup);	// SW_SHOWMINIMIZED

	return ddeExec(hWndProgman, buf);
}

/*
 *  fmOpen() - Open a existing FrogMan Group File. Or create a new one
 *
 *      szName  - Name of the group to create
 *		  note, if this group already exists it is cleared.
 *
 *  RETURNS:    progman dde window handel
 *
 */
HWND PUBLIC fmOpen(PSTR szName, BOOL fDelete)
{
	char buf[MAXSTR];

	if (!hWndProgman) {

		if (!(hWndProgman = ddeInitiate(szProgman, szProgman)))
			return NULL;

		// don't let bozo close progman
		BringWindowToTop(hWndProgman);
		ShowWindow(hWndProgman, SW_RESTORE);

		EnableWindow(hWndProgman, FALSE);
	}

	if (fDelete) {
		wsprintf(buf, "[deletegroup(%s)]", (LPSTR)szName);
		ddeExec(hWndProgman, buf);
	}


	wsprintf(buf, szCreateCmd, (LPSTR)szName);
	ddeExec(hWndProgman, buf);

	return hWndProgman;
}

/*
 *  fmClose() - Close a Group file opened with fmOpen()
 *              The header will be updated to reflect any canges made.
 *
 *      fh      - File handle to Group file
 *
 *  RETURNS:    TRUE if successful FALSE otherwise
 *
 */

BOOL PUBLIC fmClose()
{
    WinAssert(hWndProgman);

    EnableWindow(hWndProgman, TRUE);		// don't let bozo close progman

    ddeExec(hWndProgman, "[exitprogman(1)]");	// close save state

    hwndServer = hWndProgman = NULL;

    return TRUE;
}

/*
 *  fmAddItem() - Append a new item to a Group file opened with fmOpen()
 *
 *      szName          - Name of item to add.
 *      szCmd           - Item's Command line
 *	szExe		- EXE file to grab icon from
 *			  if NULL then get icon from szCmd
 *	iIcon		- icon # in szExe to get
 *
 *  RETURNS:    TRUE if successful FALSE otherwise
 *
 */
BOOL PUBLIC fmAddItem(PSTR szName, PSTR szCmd, PSTR szExe, int iIcon)
{

    #define EXEC_STR_SIZE 200
    char buf[EXEC_STR_SIZE];

    WinAssert(szName[0]);
    WinAssert(szCmd[0]);

    WinAssert((lstrlen(szCmd) + lstrlen(szName) + lstrlen(szExe) + 10) < EXEC_STR_SIZE);

    wsprintf(buf, "[additem(%s,\"%s\",%s,%d)]", (LPSTR)szCmd, (LPSTR)szName, (LPSTR)szExe, iIcon);

    WinAssert(hWndProgman);

    return ddeExec(hWndProgman, buf);
}

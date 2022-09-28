/*----------------------------------------------------------------------------*\
|   ws.h  - Windows Setup						       |
|                                                                              |
|   History:                                                                   |
|	03/09/89 toddla     Created					       |
|                                                                              |
\*----------------------------------------------------------------------------*/

#include "wprintf.h"

#define XMS_SPEC_VER	0x0200	// we require an XMS spec version of this or higher
#define XMS_DRIVER_VER	0x0260	// and a driver version of this or higher

extern char	bufTmp[];
extern HWND	hwndWS;
extern HWND	hwndHelp;
extern HWND	hwndDlg;
extern HWND	hwndExitButton;
extern HWND	hwndHelpButton;
extern HWND	hwndInst;
extern HWND	hwndFileCopy;
extern HANDLE	hInstWS;
extern HICON	hiconOld;
extern BOOL	fFirstTime;
extern BOOL	fMono;
extern BOOL	fCancel;
extern BOOL	bIsNetSetup;
extern BOOL	bUpdateOldWindows;
extern BOOL	bIs386;
extern BOOL	bTweakMouse;
extern WORD	fExit;
extern int	dyChar;
extern long int liTotalSize;
extern WORD	capsScreen;
extern HICON	hiconWinApp;
extern char szRoot[];
extern char szSystem[];
extern char szNull[];
extern char szDisplay[];
extern char szMouse[];
extern char szKeytype[];
extern char szSystemIni[];
extern char szProgmanIni[];
extern char szCaption[];
extern char szEGASYS[20];	// #:EGA.SYS if we are doing an ega.sys hack
extern char szNetwork[];
extern char szK[];
extern char szHelpFile[];



#define CP_PRINTERSETUP (WM_USER + 401)
#define CP_PRINTERHELP  (WM_USER + 402)

/*
 *  Exit flags.
 */
#define EF_RESTART  0x01        // User allowed option to start windows.
#define EF_REBOOT   0x02        // User needs to reboot
#define EF_DOS      0x04        // User should be returned to DOS

void FAR cdecl DlgPrintf (HWND hwnd, int id, PSTR pch,...);

BOOL PUBLIC wsInstallDlg(HWND,unsigned,WORD,LONG);
BOOL PUBLIC wsMaintainDlg(HWND,unsigned,WORD,LONG);
BOOL PUBLIC wsAppsDlg(HWND,unsigned,WORD,LONG);
BOOL PUBLIC wsDiskDlg(HWND,unsigned,WORD,LONG);
BOOL PUBLIC wsErrorDlg(HWND,unsigned,WORD,LONG);
BOOL PUBLIC wsPifSearchDlg(HWND,unsigned,WORD,LONG);
BOOL PUBLIC wsConfigSysDlg(HWND,unsigned,WORD,LONG);
BOOL PUBLIC wsSysOldDlg(HWND,unsigned,WORD,LONG);
BOOL PUBLIC wsViewSettingsDlg(HWND,unsigned,WORD,LONG);
BOOL PUBLIC wsChangeSettingsDlg(HWND,unsigned,WORD,LONG);
BOOL PUBLIC wsExitDlg(HWND,unsigned,WORD,LONG);
BOOL PUBLIC wsDoOemDisplay(HWND hDlg, PSTR pinf);
BOOL PUBLIC wsDoOemMouse(HWND hDlg, PSTR pinf);
long PUBLIC InstWndProc(HWND,unsigned,WORD,LONG);
long PUBLIC DeskWndProc(HWND,unsigned,WORD,LONG);
void PUBLIC FixPath(PSTR szPath);
void PUBLIC EnableExit(BOOL bEnable);
void PUBLIC EnableHelp(BOOL bEnable);
void PUBLIC CloseSetup(HWND hwnd);


BOOL PUBLIC wsYield(void);

PSTR PUBLIC wsLoadSz(int ids, PSTR pch);
BOOL PUBLIC wsHelp(int id);
BOOL PUBLIC wsHelpMeSpock(HWND);
BOOL PUBLIC wsHelpTerm(HWND hwnd);

void PUBLIC wsDlgInit(HWND hDlg);
WORD PUBLIC wsCopyError(int n, LPSTR sz);
WORD PUBLIC wsInsertDisk(int n, LPSTR sz);

BOOL PUBLIC wsHelpInit(HANDLE hinst);

BOOL PUBLIC wsCopyFiles(PSTR szSource, PSTR szDest);
BOOL PUBLIC wsCopy(PSTR szSection);
BOOL PUBLIC wsCopySys(BOOL bCopy);

BOOL PUBLIC wsCopyFonts(HWND);
BOOL PUBLIC wsBootShell(void);
BOOL PUBLIC wsInstallApps(HWND);
BOOL PUBLIC wsDickWithConfig(HWND hwnd);

void PUBLIC wsStartWait(void);
void PUBLIC wsEndWait(void);
int  PUBLIC QueryRadioButton(HWND hDlg, int idLow, int idHigh);

WORD PUBLIC wsCopyStatus(int msg, int n, LPSTR szFile);
WORD PUBLIC wsCopySingleStatus(int msg, int n, LPSTR szFile);
BOOL PUBLIC WinExecWait(LPSTR szCmd, int sw);
#ifdef DO_PATCH
BOOL PUBLIC wsRunPatches(void);
#endif

PSTR PUBLIC GetDriverName(PSTR szDriver, PSTR szBuf);
BOOL PUBLIC CheckSpecialKeys(LPMSG pmsg);
BOOL PUBLIC wsInstallWindows(WORD fOptions);
BOOL PUBLIC QueryExit();
LONG PUBLIC atoi(PSTR sz);
int  PUBLIC CopyOldAppFonts(PSTR szGrabber, BOOL bIsNet);
LPSTR PUBLIC lgetenv(LPSTR lpszName);
LONG PUBLIC ldiv(LONG dw, LONG divisor);
void PUBLIC SizeWinHelp(BOOL bSetDefault);
void PUBLIC GenerateProgmanPath(PSTR filename, PSTR szPath);










		

/*----------------------------------------------------------------------------*\
|   PROGRESS.H								       |
|                                                                              |
|   Description:                                                               |
|                                                                              |
|   Notes:                                                                     |
|                                                                              |
|   History:                                                                   |
|	11/01/87 Toddla     Created					       |
|                                                                              |
\*----------------------------------------------------------------------------*/

BOOL EXPORT ProDlgProc(HWND, unsigned, WORD, LONG);
BOOL PUBLIC ProInit(HANDLE,HANDLE);
void PUBLIC ProClear(HWND hDlg);
HWND PUBLIC ProOpen(HWND,int);
BOOL PUBLIC ProClose(void);
BOOL PUBLIC ProSetBarRange(int);
BOOL PUBLIC ProSetBarPos(int);
BOOL PUBLIC ProDeltaPos(int);
BOOL PUBLIC ProSetText (int,LPSTR);
BOOL FAR cdecl ProPrintf (int,LPSTR,...);


/* 'About' dialog box resource id */

#define STD_EXIT_MSGBOX   MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2 | MB_SYSTEMMODAL
#define STD_OK_MSGBOX     MB_ICONEXCLAMATION | MB_OK | MB_SYSTEMMODAL

#define ID_APP      1000
#define ABOUTBOX         1
/* #define DLG_WINSETUP     11
#define DLG_INSERTDISK   16
#define DLG_COPYERROR	 19	 */
#define DLG_PROGRESS     400
#define DLG_EXIT         25
#define ID_BAR	         401
#define PRO_CLASS        "PRO"

/* Dialog box items and controls. */

#define ID_NULL           -1
#define ID_OK              1
#define ID_CANCEL          2
#define ID_INSTALLAPPS	   102
#define ID_README	   106
#define ID_INSTALLPRINTER  104
#define ID_ABORT           4
#define ID_RETRY           5
#define ID_IGNORE          6
#define ID_EDIT            11
#define ID_TEXT            12
#define ID_EDIT1           200
#define ID_EXITSETUP       7

#define IDS_ERROR    20000

#define ID_STATUS0   4000
#define ID_STATUS1   (ID_STATUS0 + 1)
#define ID_STATUS2   (ID_STATUS0 + 2)
#define ID_STATUS3   (ID_STATUS0 + 3)
#define ID_STATUS4   (ID_STATUS0 + 4)

/* setup template options */

#define F_PRINTER   0x01
#define F_APPS      0x02
#define F_README    0x04
#define F_CONTINUE  0x08

/*
 *  String IDs
 */
#define IDS_APPNAME             302
#define IDS_NOINF		372
#define IDS_NOINFMEM		393
#define IDS_WINCOPY             332
#define IDS_WAITCOPY            319
#define IDS_COPYING             317
#define IDS_OUTOFDISK		359
#define IDS_DISKS               329
#define IDS_PROGMAN_GROUP	354
#define IDS_BUILDPMG            431
#define IDS_EXITMSG             432
#define IDS_EXITCAPTION         433
#define IDS_DISKSPACE           434
#define IDS_NEEDROOM            435
#define IDS_INFNAME             436
#define IDS_DEFAULT_DIR         437
#define IDS_FINISHED            438

/* 
 *  Function prototypes
 */
BOOL FAR PASCAL ControlInit (HANDLE hPrev, HANDLE hInst);
LONG FAR PASCAL fnText   (HWND, unsigned, WORD, LONG);
BOOL PUBLIC wsProgman(HWND hwnd);
void PRIVATE SetProgmanSize();
void PUBLIC ActivateMain();
BOOL PUBLIC fnErrorMsg(int ID_Error_Message);
VOID PUBLIC fnOkMsgBox(int ID_Error_Message);
//int  FAR fDialog(int id,HWND hwnd,FARPROC fpfn);
int  FAR fDialog(LPSTR id,HWND hwnd,FARPROC fpfn);

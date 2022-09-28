/*----------------------------------------------------------------------------*\
|   frogman.h     Module to Write a FrogMan Config File.                       |
|                                                                              |
|   History:                                                                   |
|	03/09/89 toddla     Created					       |
|                                                                              |
\*----------------------------------------------------------------------------*/

//
//  Format of a ProgMan Group file.
//
//      magic number                    (LONG)   must equal GRP_MAGIC
//      group header                    (GRPDEF)
//      group name                      (STRING)
//      item1
//      item2
//       ...
//      itemN
//
//  Format of a ITEM
//
//      item header                     (ITEMDEF)
//      icon header
//      icon AND mask
//      icon XOR mask
//      Item Name                       (STRING)
//      Item Command line               (STRING)
//      Name of EXE containing Icon     (STRING)
//
//  Format of a STRING
//
//      Strings are Length prefixed, the length occupies 2 bytes
//
//
//  ProgMan will open ALL groups listed in the [Groups] section of PROGMAN.INI
//
//      [Groups]
//          GROUP1 = GROUP1.GRP
//          GROUP2 = GROUP2.GRP
//


#define GRP_MAGIC   0x43434D50L     // "PMCC"

/* structure of group and items in .GRP files
 */
typedef struct tagGROUPDEF
  {
    WORD	  nCmdShow;	      /* min, max, or normal state */
    RECT	  rcNormal;	      /* rectangle of normal window */
    POINT	  ptMin;	      /* point of icon */
    WORD	  wLogPixelsX;	      /* screen info for icon extraction */
    WORD	  wLogPixelsY;	      /*  ... */
    WORD	  wBitsPixel;	      /*  ... */
    WORD	  wPlanes;	      /*  ... */
    WORD	  cItems;	      /* number of items in group */
  } GROUPDEF;
typedef GROUPDEF FAR * LPGROUPDEF;

typedef struct tagITEMDEF
  {
    POINT	  pt;		      /* location of item icon in group */
    WORD	  iIcon;	      /* index of item icon */
    WORD	  cbHeader;	      /* size of icon header */
    WORD	  cbANDPlane;	      /* size of and part of icon */
    WORD	  cbXORPlane;	      /* size of xor part of icon */
  } ITEMDEF;
typedef ITEMDEF FAR * LPITEMDEF;

BOOL	PUBLIC ddeInit(HANDLE hInst, HANDLE hPrev);

HANDLE	PUBLIC fmOpen(PSTR szName, BOOL fDelete);
BOOL	PUBLIC fmMinimize(PSTR szName);
BOOL	PUBLIC fmActivate(PSTR szName);
BOOL	PUBLIC fmClose();
BOOL	PUBLIC fmAddItem(PSTR szName, PSTR szCmd, PSTR szExe, int iIcon);
HICON	PUBLIC ExtractIcon(LPSTR lpszExeFileName, WORD nIconIndex);


/*
 * EXPORTED stuff from USER
 */
DWORD PUBLIC DumpIcon(LPSTR, LPINT, LPSTR, LPSTR);



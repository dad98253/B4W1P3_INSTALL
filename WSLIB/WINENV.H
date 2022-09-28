/* winenv.h
 *
 * Standard Windows environment types, macros etc.
 *
 * This file includes (and therefore replaces) <windows.h> and an analog of
 * <winexp.h>.  The following macros are also defined:
 *    dprintf(), WinAssert(), WinEval()
 *
 */

#define NOCLIPBOARD	/* clipboard routines */
#define NOCOMM		/* COMM driver routines */
#define NOCREATESTRUCT	/* typedef CREATESTRUCT */
#define NOFONT		/* typedef FONT and associated routines */
#define NOGDICAPMASKS	/* CC_*, LC_*, PC_*, CP_*, TC_*, RC_ */
#define NOKANJI		/* Kanji support stuff. */
#define NOKEYBOARDINFO	/* keyboard information functions */
#define NOLANGUAGE	/* 3.0 language-dependent text functions */
#define NOMETAFILE	/* typedef METAFILEPICT */
#define NOREGION	/* typedef HRGN and associated routines */
#define NOSOUND		/* Sound driver routines */

#include <windows.h>

#ifndef NOLSTRING
    LPSTR	far PASCAL lstrbscan( LPSTR, LPSTR );
    LPSTR	far PASCAL lstrbskip( LPSTR, LPSTR );
#endif

#ifndef NOLFILEIO
    int 	far PASCAL OpenPathname( LPSTR, int );
#endif

/* flags for _lseek */
#define  SEEK_CUR 1
#define  SEEK_END 2
#define  SEEK_SET 0

int FAR cdecl WinPrintf(LPSTR,...);

#ifdef DEBUG
        BOOL    __f;
        extern  int FAR _Assert(PSTR,int);
	extern  BOOL fDebug;
        #define dprintf if (fDebug) WinPrintf
        #define WinAssert(exp)  ((exp) ? 0 : _Assert(__FILE__,__LINE__))
        #define WinEval(exp) (__f=(exp),WinAssert(__f),__f)
#else
        #define dprintf if (0) ((int (*)(char *, ...)) 0)
        #define WinAssert(exp) 0
        #define WinEval(exp) (exp)
#endif

/*
 *  macros used to define functions:
 *
 *	EXPORT	    - This function can be called from external to this DLL
 *		      but be sure to list it in you DEF file!
 *
 *	PUBLIC	    - This function can be called from another file
 *
 *	PRIVATE     - This function can only be called from inside this file
 *
 *  eg	  int EXPORT foo();
 */

#define EXPORT	    FAR  PASCAL
#define PUBLIC	    FAR  PASCAL
#define PRIVATE     NEAR PASCAL

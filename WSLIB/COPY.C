/*
 *  copy.c - Copy routine for WinDosSetup
 *  Toddla
 *
 *  Modification History:
 *  3/24/89  Toddla      Wrote it
 *
 *  notes:
 *	we now use the LZCopy stuff so COMPRESS is NOT defined
 *	we now set the crit error handler ourselves so CHECKFLOPPY is
 *	NOT defined
 */

#include <dos.h>
#include <string.h>
#include <ctype.h>
#include "sulib.h"
#include "..\install.h"
#include "ws.h"
#include "progdde.h"
#include "gauge.h"


#define MAXBUF	(60 * 1024)	// size of default copy buffer

/*
 *  global vars used by DosCopy
 */
static LPSTR    lpBuf = NULL;   // copy buffer
static int      iBuf = 0;       // usage count
static WORD     nBufSize;
static char	cDisk;
static LPSTR	szEdit;
static char	szDstName[MAXPATHLEN];
extern PSTR	pErrMsg;
static BOOL	bAppend = FALSE;
static WORD nCheckSum1,nCheckSum2,nCheckSum3;
extern BOOL bCheckForVirus;
static char szDisks[] = "disks";
static long int liCopiedSoFar=0;


void NEAR PASCAL AllocCopyBuf();
void NEAR PASCAL FreeCopyBuf();
BOOL NEAR GetDiskPath(char cDisk, PSTR szPath);
WORD FAR PASCAL wsCopyStatus(int msg, int n, LPSTR szFile);
BOOL fnMystrstr(char *szSrcStr, char *szSearchStr);
void NEAR PASCAL AllocCopyBuf( void );
WORD H2D ( LPSTR szCHKSUM );
void NEAR PASCAL FreeCopyBuf( void );
void VirusError ( LPSTR szName );
void OutOfMem ( void );
WORD DoCheckSum ( WORD nCheckSumA , WORD *nSeedA , LPSTR lpBuf , WORD size );
long int FAR randJCK ( long int r );
BOOL NEAR IsDiskInDrive(int iDisk);

void FAR PASCAL fartonear(LPSTR dst, LPSTR src)
{
	while (*src)
		*dst++ = *src++;

	*dst = 0;
}

/*  WORD FileCopy (szSource, szDir, fpfnCopy, WORD f)
 *
 *  This function will copy a group of files to a single destination
 *
 *  ENTRY:
 *
 *  szSourc      : pointer to a SETUP.INF section
 *  szDest       : pointer to a string containing the target DIR
 *  fpfnCopy     : callback function used to notify called of copy status
 *  fCopy        : flags
 *
 *      FC_SECTION      - szSource is a section name
 *      FC_LIST         - szSource is a pointer to a list, each item \0
 *                        terminated and the whole list \0\0 terminated
 *      FC_FILE         - szSource is a file name.
 *      FC_QUALIFIED    - szSource is a fully qualified file name.
 *      FC_DEST_QUALIFIED - szDir is fully qualified. Don't expand this.
 *	FC_APPEND	- szSource is to be appended to the destination file.
 *
 *  NOTES:
 *      if szSource points to a string of the form '#name' the section
 *      named by 'name' will be used as the source files
 *
 *      the first field of each line in the secion is used as the name of the
 *      source file.  A file name has the following form:
 *
 *          #:name
 *
 *          #       - Disk number containing file 1-9,A-Z
 *          name    - name of the file, may be a wild card expression
 *
 *  Format for copy status function
 *
 *  BOOL FAR PASCAL CopyStatus(int msg, int n, PSTR szFile)
 *
 *      msg:
 *          COPY_ERROR          error occured while copying file(s)
 *                              n      is the DOS error number
 *                              szFile is the file that got the error
 *                              return: TRUE ok, FALSE abort copy
 *
 *          COPY_STATUS         Called each time a new file is copied
 *                              n      is the percent done
 *                              szFile is the file being copied
 *                              return: TRUE ok, FALSE abort copy
 *
 *          COPY_INSERTDISK     Please tell the user to insert a disk
 *                              n      is the disk needed ('1' - '9')
 *                              return: TRUE try again, FALSE abort copy
 *
 *          COPY_QUERYCOPY      Should this file be copied?
 *                              n      line index in SETUP.INF section (0 based)
 *                              szFile is the line from section
 *                              return: TRUE copy it, FALSE dont copy
 *
 *          COPY_START          Sent before any files are copied
 *
 *          COPY_END            Sent after all files have been copied
 *                              n   is dos error if copy failed
 *
 *
 *  EXIT: returns TRUE if successful, FALSE if failure.
 *
 */

WORD FAR PASCAL FileCopy (LPSTR szSource, PSTR szDir, FPFNCOPY fpfnCopy, WORD fCopy)
{
    int       err = ERROR_OK;
    char      szFile[MAXPATHLEN];
    char      szPath[MAXPATHLEN];
    char      szLogSrc[MAXPATHLEN];
    char      szSrcBase[15];
    char      szSrc[MAXPATHLEN];
    char      szErrFile[MAXPATHLEN];
    LPSTR far *pFileList;
    LPSTR far *pFileListBegin;
    LPSTR     pFile;
    LPSTR     pFileBegin;
    BOOL      f;
    BOOL      fDoCopy;
    int       n = 0;
    int       nDisk;
    char      cDisk;
    int       cntFiles = 0;
    PINF      pinf;
    char      szCKSM[20];

    #define CALLBACK1(msg,n,pFile) \
        (fpfnCopy ? ((*fpfnCopy)((WORD)(msg),(int)(n),(LPSTR)(pFile))) : FC_IGNORE)

    if (!szSource || !*szSource || !szDir || !*szDir)
        return FALSE;

#ifndef COMPRESS
    AllocCopyBuf();
#endif

    /*
     *  fix up the drive in the destination
     */

    if ( fCopy & FC_DEST_QUALIFIED )
       lstrcpy(szPath,szDir);
    else
       ExpandFileName(szDir,szPath);

    fCopy &= ~FC_DEST_QUALIFIED;

    if (szSource[0] == '#' && fCopy == FC_FILE)
    {
        fCopy = FC_SECTION;
        ++szSource;
    }

    switch (fCopy)
    {
        case FC_LSTPTR:
            pFileList = pFileListBegin = (LPSTR far *)szSource;
            pFileBegin = *pFileList;

            while ( pFileList[n] ) {
               if ( *pFileList[n] )
                  ++cntFiles;
               ++n;
            }

            break;

        case FC_SECTION:
	    {
	    char buf[40];

	    fartonear(buf, szSource);
            szSource = infFindSection(NULL,buf);
            if (szSource == NULL)
                goto exit;

	    fCopy = FC_LIST;
	    }
            // fall through to FC_LIST

        case FC_LIST:
            pFileBegin = szSource;
            cntFiles = infLineCount(szSource);
            break;

        case FC_FILE:
        case FC_QUALIFIED:
        default:
            pFileBegin = szSource;
            cntFiles = 1;
    }

    /*
     * Does the destination directory exist? if not create it.
     */
    if (!DosValidDir(szPath)) {

        err = DosMkDir(szPath);

	// oh no! this is bad

        if (err != ERROR_OK) {
	    CALLBACK1(COPY_ERROR,err,szPath);
            goto exit;
        }
    }

    /*
     *  walk all files in the list and call DosCopy ....
     *
     *  NOTES:
     *      we must walk file list sorted by disk number.
     *      we should use the disk that is currently inserted.
     *      we should do a find first/find next on the files????
     *      we need to check for errors.
     *      we need to ask the user to insert disk in drive.
     *
     */
    CALLBACK1(COPY_START,0,NULL);

    for (nDisk = 1; cntFiles > 0; nDisk++) {

        cDisk      = CHDISK(nDisk);
        pFileList  = pFileListBegin;
        pFile      = pFileBegin;
        n          = 0;

        while (pFile) {

            /*
             *  should we copy this file?
             *  copy the files in disk order.
             */
            fDoCopy = pFile[1] == ':' && cDisk == UP_CASE(pFile[0]) ||
                      pFile[1] != ':' && nDisk == 1 && *pFile ||
                      fCopy == FC_QUALIFIED;

            if (fDoCopy)
                cntFiles--;         // done with a file. decrement count.

	    if (fDoCopy && CALLBACK1(COPY_QUERYCOPY,n,pFile)) {

		if (CALLBACK1(COPY_STATUS, 0, pFile) == FC_ABORT) {
		    err = ERROR_NOFILES;
                    goto exit;
		}

		bAppend = FALSE;
		if ( infParseField(pFile,3,szLogSrc) &&
		     infParseField(pFile,4,szDstName) )
		{
		    if ( szLogSrc[0] == '+' )
		    {
		       bAppend = TRUE;
		       infParseField(pFile, 1, szLogSrc);	// logical source
		       if ( fCopy != FC_QUALIFIED )
			  ExpandFileName(szLogSrc, szSrc); // full physical source
		       else
			  lstrcpy(szSrc,szLogSrc);
	  /*	       switch (CALLBACK1(COPY_INSERTDISK, szLogSrc[0], szSrc))
		       {
			  case FC_RETRY:
		       //	    catpath(szSrc, szSrcBase);	    // add the file back on
				  goto tryagain;		  // and try again...

			  case FC_ABORT:
				  goto exit;

			  case FC_IGNORE:
				  break;
		       }
	   */
		    }
		}

		// +++++++++++++++++++++++++++++++++++++++++++++++++++++
		// +++++++++++++++++++++++++++++++++++++++++++++++++++++

		// check for checksum fields on the pFile... if found,
		// decode them into Hex and save for later use

		if ( infParseField ( pFile, 5, szCKSM ) )
		{
		    nCheckSum1 = H2D ( szCKSM );
		    dprintf("Checksum 1 = %x\n",nCheckSum1);
		}
		else
		{
		    nCheckSum1 = 0;
		}

		if ( infParseField ( pFile, 6, szCKSM ) )
		{
		    nCheckSum2 = H2D ( szCKSM );
		    dprintf("Checksum 2 = %x\n",nCheckSum2);
		}
		else
		{
		    nCheckSum2 = 0;
		}

		// +++++++++++++++++++++++++++++++++++++++++++++++++++++
		// +++++++++++++++++++++++++++++++++++++++++++++++++++++

		// now we convert logical dest into a physical (unless FC_QUALIFIED)

                infParseField(pFile, 1, szLogSrc);	 // logical source
                if ( fCopy != FC_QUALIFIED )
                   ExpandFileName(szLogSrc, szSrc); // full physical source
                else
		   lstrcpy(szSrc,szLogSrc);

tryagain:   
                // Call low level copy command

                err = DosCopy(szSrc, szPath);
      
                if (err != ERROR_OK) {

		    lstrcpy(szSrcBase, FileName(szSrc)); // save base name

                    if (err == ERROR_FILENOTFOUND || err == ERROR_PATHNOTFOUND) {

                        // isolate the path

                        StripPathName(szSrc);

		   	// now try to get a new path in szSrc

			switch (CALLBACK1(COPY_INSERTDISK, szLogSrc[0], szSrc)) {
	           	case FC_RETRY:
		   		catpath(szSrc, szSrcBase);	// add the file back on
		   		goto tryagain;			// and try again...

                   	case FC_ABORT:
                   		goto exit;

	           	case FC_IGNORE:
        	        	break;
                   	}

                    }

		    // ERROR situation
		    //
		    // this may be a real error or something like
		    // a share violation on a network.

		    ExpandFileName(szLogSrc, szSrc);	// full physical source

		    // if it is a write error report the destination file
		    // otherwise report with the source file

		    switch (err) {
		    case ERROR_WRITE:
		    	lstrcpy(szErrFile, szPath);
			catpath(szErrFile, szSrcBase);
		    	break;

		    default:
		    	lstrcpy(szErrFile, szSrc);
		    }

		    switch (CALLBACK1(COPY_ERROR, err, szErrFile)) {

                    case FC_RETRY:
                            goto tryagain;
    
                    case FC_ABORT:
                            goto exit;

                    case FC_IGNORE:
                            break;
                    }
                }

		if (CALLBACK1(COPY_STATUS,101,pFile) == FC_ABORT) {
                    err = !ERROR_OK;
                    goto exit;
                }
            }

            /*
             * Move on to next file in the list
             */
            n++;
            if (fCopy == FC_LSTPTR)
                pFile = *(++pFileList);
	         else if (fCopy == FC_LIST)
                pFile = infNextLine(pFile);
            else
                pFile = NULL;
        }
    }

    err = ERROR_OK;

exit:
    CALLBACK1(COPY_END,err,NULL);
#ifndef COMPRESS
    FreeCopyBuf();
#endif
    return err;

    #undef CALLBACK1
}

#ifndef COMPRESS

/*  AllocCopyBuf()
 *
 *  allocate a buffer for DosCopy to use
 *
 */
void NEAR PASCAL AllocCopyBuf()
{
    if (iBuf++ == 0)
    {
        nBufSize = MAXBUF;
        for(;;)
        {
            lpBuf = FALLOC(nBufSize);
            if (lpBuf || nBufSize == 1)
                break;
            nBufSize /= 2;
        }
        if (lpBuf == NULL)
            iBuf--;
    }
}

/*  FreeCopyBuf()
 *
 *  free copy buffer, if its use count is zero
 *
 */
void NEAR PASCAL FreeCopyBuf()
{
    if (iBuf > 0 && --iBuf == 0 && lpBuf)
    {
        FFREE(lpBuf);
    }
}
#endif

PSTR GetExtension(PSTR szFile)
{
	PSTR ptr;

	for (ptr = szFile; *ptr && *ptr != '.'; ptr++);

	if (*ptr != '.')
		return NULL;
	else
		return ptr+1;

}

BOOL GetCompressedName(PSTR szComp, PSTR szSrc)
{
	PSTR ptr;

	lstrcpy(szComp, szSrc);

	ptr = GetExtension(szComp);

	if (ptr && !lstrcmpi(ptr, "sys")) {
		szComp[lstrlen(szComp)-1] = '$';
		return TRUE;
	}

	return FALSE;
}

/*  DosCopy(PSTR szSrc, PSTR szPath)
 *
 *  Copy the file specifed by szSrc to the drive and directory
 *  specifed by szPath
 *
 *  ENTRY:
 *      szSrc   - File name to copy from
 *      szPath  - directory to copy to
 *
 *  RETURNS:
 *      0 - no error, else dos error code
 *
 */
int NEAR DosCopy(PSTR szSrc, PSTR szPath)
{
    FCB         fcb;
    WORD        size;
    int         fhSrc,fhDst;
    char        szFile[MAXPATHLEN];
    char        szComp[MAXPATHLEN];
    int         f = ERROR_OK;
    unsigned    date;
    unsigned    time;
    long	l;
    BOOL	bCompressedName;
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++
    WORD	nCheckSumA,nCheckSumB;
    WORD	nSeedA,nSeedB;
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++
    

#ifdef DEBUG
    if (fDontCopy)
        return ERROR_OK;

    if (infGetProfileString(NULL,"setup","copy",szFile) && szFile[0] == 'f')
        return ERROR_OK;
#endif

    AllocCopyBuf();

    if (!lpBuf)
        return ERROR_NOMEMORY;

#ifdef CHECK_FLOPPY
    if (!IsDiskInDrive(szSrc[0]))
    {
        f = ERROR_FILENOTFOUND;
        goto errfree;
    }
#endif


    // allows both sy$ and sys on the disks

    if (GetCompressedName(szComp, szSrc) &&
    	DosFindFirst(&fcb, szComp, ATTR_FILES)) {

    	bCompressedName = TRUE;

    } else {

        bCompressedName = FALSE;

    	if (!DosFindFirst(&fcb, szSrc, ATTR_FILES)) {
		f = ERROR_FILENOTFOUND;
		goto errfree;
	}
    }

    /*
     * copy every file that matches the file pattern passed in.
     */
    do
    {
        /*
         * create the source file name from the source path and the file
         * name that DosFindFirst/Next found
         */
        lstrcpy(szFile,szSrc);
        StripPathName(szFile);
        catpath(szFile,fcb.szName);

        fhSrc = FOPEN(szFile);

        if (fhSrc == -1)
        {
            f = FERROR();
            goto errfree;
        }

        /* Save date of opened file */

        if (_dos_getftime(fhSrc,&date,&time))
           goto errclose1;

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++

	// if we are suppose to check for a virus, then the system
	// time tag on the file should agree with the checksum
	// read from the pFile... if not, display the "Virus!"
	// MessageBox and quit

	nCheckSum3 = time;
	dprintf("time tag = %x\n",nCheckSum3);
	if ( bCheckForVirus && nCheckSum2 != 0 && nCheckSum2 != nCheckSum3 )
	{
	    VirusError ( fcb.szName );
	    f = 77;
	    goto errclose1;
	}

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++


        /*
         * create the destination file name from the dest path and the file
         * name that DosFindFirst/Next found
         */
        lstrcpy(szFile,szPath);

	// don't support wildcards for compressed files

	if ( !bAppend )
	{
	    if (bCompressedName)
		catpath(szFile,FileName(szSrc));
	    else
		catpath(szFile,fcb.szName);	// use name from fcb
	    fhDst = FCREATE(szFile);
	}
	else
	{
	    catpath(szFile,szDstName); // use name from 4th field in .INF file
	    fhDst = _lopen(szFile, OF_READWRITE );
	    _llseek(fhDst,(long int)0,(int)2);
	}

        if (fhDst == -1)
        {
            f = FERROR();
            goto errclose1;
        }

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++

	nCheckSumA = nCheckSumB = 0;
	nSeedA = 1;
	nSeedB = 777;

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++

        while (size = FREAD(fhSrc,lpBuf,nBufSize))
        {
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++

	    nCheckSumA = DoCheckSum ( nCheckSumA , &nSeedA , lpBuf , size );
	    nCheckSumB = DoCheckSum ( nCheckSumB , &nSeedB , lpBuf , size );

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++
            if (FWRITE(fhDst,lpBuf,size) != size)
            {
                /* write error? */
                f = FERROR();
                if (f == ERROR_OK)
                    f = ERROR_WRITE;
                goto errclose;
            }
		// +++++++++++++++++++++++++++++++++++++++++++++++++++++
		// +++++++++++++++++++++++++++++++++++++++++++++++++++++
		liCopiedSoFar += (long int)size;
		ProSetBarPos (
		    (int)(100.0*(float)liCopiedSoFar/(float)liTotalSize)
									 );
		if (! wsYield() )
		{
		    f = 78;
		    goto errclose;
                }
	       // +++++++++++++++++++++++++++++++++++++++++++++++++++++
	       // +++++++++++++++++++++++++++++++++++++++++++++++++++++
        }

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++

	if ( nCheckSum1 == 0 && nCheckSum2 == 0 )
	{
	    dprintf("Checksum C = %x\n",nCheckSumA+nCheckSumB);
            if ( bCheckForVirus && ( nCheckSumA + nCheckSumB != nCheckSum3 ))
	    {

	    // if the DEBUG flag is defined, reset the time that the source
	    // file was writen to CheckSum C

#if defined ( DEBUG )
		_dos_setftime(fhSrc,date,nCheckSumA+nCheckSumB);
#endif

		VirusError ( fcb.szName );
		f = 77;
		goto errclose1;
	    }
	}
	else
	{
	    dprintf("Checksum A = %x , Checksum B = %x\n",nCheckSumA,nCheckSumB);
            if ( bCheckForVirus &&
                ( nCheckSumA != nCheckSum1 || nCheckSumB != nCheckSum2 ))
	    {

	    // if the DEBUG flag is defined, reset the time that the source
	    // file was writen to CheckSum B

#if defined ( DEBUG )
		_dos_setftime(fhSrc,date,nCheckSumB);
#endif

		VirusError ( fcb.szName );
		f = 77;
		goto errclose1;
	    }
	}

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++

        /* Restore date of written file */
        _dos_setftime(fhDst,date,time);

errclose:
        FCLOSE(fhDst);
errclose1:
        FCLOSE(fhSrc);

    }   while (f == ERROR_OK && DosFindNext(&fcb));

errfree:

    FreeCopyBuf();

    return f;
}


/* BOOL fnMystrstr(char *szSrcStr, char *szSearchStr);
 *
 * Function will return BOOL value as to weather the Search string exists
 * any where within the source string. The difference between this func
 * the C run time func is that this one is simpler and is also not case
 * sensitive.
 *
 * ENTRY: szSrcStr    - Char buffer to be searched.
 *
 *        szSearchStr - String that will be searched for.
 *
 * EXIT:  BOOL value as to weather or not string was found.
 *
 *
 * WARNING: Source and search strings MUST be null terminated.
 *          
 *
 */
BOOL fnMystrstr(szSrcStr, szSearchStr)
char       *szSrcStr;
char       *szSearchStr;
{
   unsigned      len;             // Get length of search string.

   while (szSearchStr[0] == '.' && SLASH(szSearchStr[1]))
      szSearchStr+=2;

   len = lstrlen(szSearchStr);

   while ( !ISEOL(*szSrcStr) ) {
      if ( ! strnicmp(szSrcStr,szSearchStr,len))
         return TRUE;
      ++szSrcStr;
   }
   return FALSE;

}

/*  BOOL GetDiskPath(char cDisk, szPath)
 *
 *  This function will retrive the full path name for a logical disk
 *  the code reads the [disks] section of SETUP.INF and looks for
 *  n = path where n is the disk char.  NOTE the disk '0' defaults to
 *  the root windows directory.
 *
 *  ENTRY:
 *
 *  cDisk        : what disk to find 0-9,A-Z
 *  szPath       : buffer to hold disk path
 *
 */
BOOL NEAR GetDiskPath(char cDisk, PSTR szPath)
{
    char    ach[2];
    char    szBuf[MAXPATHLEN];

    if (cDisk == '0')
    {
        /*
         * return the windows setup directory
         */
        lstrcpy(szPath,szSetupPath);
        return TRUE;
    }

    /*
     * now look in the [disks] section for a full path name
     */
    ach[0] = cDisk;
    ach[1] = 0;
    if ( !infGetProfileString(NULL,szDisks,ach,szPath) )
       return FALSE;
    infParseField(szPath,1,szPath);
    /*
     *  is the path relative? is so prepend the szDiskPath
     */
    if (szPath[0] == '.' || szPath[0] == 0)
    {
        lstrcpy(szBuf,szDiskPath);
        if (! fnMystrstr(szDiskPath,szPath) )
           catpath(szBuf,szPath);
        lstrcpy(szPath,szBuf);
    }
    return TRUE;
}


/*  BOOL FAR PASCAL ExpandFileName(PSTR szFile, PSTR szPath)
 *
 *  This function will retrive the full path name for a file
 *  it will expand, logical disk letters to pyshical ones
 *  will use current disk and directory if non specifed.
 *
 *  if the drive specifed is 0-9, it will expand the drive into a
 *  full pathname using GetDiskPath()
 *
 *  IE  0:system ==>  c:windows\system
 *      1:foo.txt     a:\foo.txt
 *
 *  ENTRY:
 *
 *  szFile       : File name to expandwhat disk to find
 *  szPath       : buffer to hold full file name
 *
 */
BOOL FAR PASCAL ExpandFileName(PSTR szFile, PSTR szPath)
{
    char    szBuf[MAXPATHLEN*2];

    if (szFile[1] == ':' && GetDiskPath(szFile[0],szBuf))
    {
        lstrcpy(szPath,szBuf);
        if (szFile[2])
            catpath(szPath,szFile + 2);
    }
    else
    {
        lstrcpy(szPath,szFile);
    }
    return TRUE;
}

/*----------------------------------------------------------------------------*\
|   wsCopy()                                                                   |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL PUBLIC wsCopy(PSTR szSection)
{
    PINF    pinf;
    char    szSource[MAXSTR];
    char    szDest[MAXSTR];
    char    szLocalTmp[MAXSTR];
    int     nFiles;
    int	    err = ERROR_OK;
    char    buf[MAXSTR];

    // we use different sections for net and win386 setup.  net setup
    // overrides win386.

    dprintf("wsCopy %s\n",szSection);

    pinf = infFindSection(NULL,szSection);
    if (pinf == NULL)
        return FALSE;

    ProPrintf(ID_STATUS1, wsLoadSz(IDS_WAITCOPY,NULL));

    for (nFiles=0; pinf; pinf = infNextLine(pinf))
    {

    	dprintf("inf line:%ls\n", (LPSTR)pinf);

        infParseField(pinf,1,szSource);

        if (szSource[0] == '#') {

	    // count these, and copy below

	    nFiles += infLineCount(infFindSection(NULL, szSource + 1));
	    dprintf("nFiles:%d\n", nFiles);

        } else
            nFiles++;
    }
    dprintf("nFiles:%d\n", nFiles);

//    ProSetBarRange(nFiles);

    for (pinf = infFindSection(NULL,szSection); pinf; pinf = infNextLine(pinf))
    {
        infParseField(pinf,1,szSource);
        infParseField(pinf,2,szDest);
        if ((err = FileCopy(szSource,szDest,(FPFNCOPY)wsCopyStatus,FC_FILE)) != ERROR_OK)
           break;
    }

    return err == ERROR_OK;
}

/*----------------------------------------------------------------------------*\
|   define the call back function used by the FileCopy() function.	       |
|                                                                              |
\*----------------------------------------------------------------------------*/

WORD FAR PASCAL wsCopyStatus(int msg, int n, LPSTR szFile)
{
    char buf[80];

    switch (msg)
    {
	case COPY_INSERTDISK:
            return wsInsertDisk(n,szFile);

	case COPY_ERROR:
            return wsCopyError(n,szFile);

	case COPY_QUERYCOPY:

	    // special case hack for .386 files built into win386.exe

	    infParseField(szFile, 1, buf);

	    if (*FileName(buf) == '*')
	    	return FALSE;		// don't copy

	    return TRUE;

	case COPY_END:
	case COPY_START:
	    SetErrorMode(msg == COPY_START);	// don't crit error on us
	    break;

        case COPY_STATUS:
	    if (n == 0)
	    {
	    	dprintf("%ls\n", (LPSTR)szFile);

		// if their is a title update it.  this allows shared titles

		if (infParseField(szFile,2,buf))
	                ProPrintf(ID_STATUS2, wsLoadSz(IDS_COPYING,NULL), (LPSTR)buf);
	    }

	    else
	    {
		if ( n < 101 ) ProSetBarPos ( n );
	    }

   //	      if (n == 100)
   //	      {
   //		  ProDeltaPos(1);
   //	      }
            if (! wsYield() )
                return FC_ABORT;

	    break;
    }
    return FC_IGNORE;
}

/*----------------------------------------------------------------------------*\
|                                                                              |
| wsCopyError()                                                                |
|                                                                              |
|   Handles errors, as the result of copying files.                            |
|                                                                              |
|   this may include net contention errors, in witch case the user must	       |
|   retry the operation.						       |
|                                                                              |
\*----------------------------------------------------------------------------*/
WORD PUBLIC wsCopyError(int n, LPSTR sz)
{
    char buf[200];
    char file[MAXSTR];
    int  res,msg;
    int   result = 0;
    FARPROC lpfn;


    lstrcpy(file, sz);		// in case our DS moves durring wsLoadSz()

    if (!wsLoadSz(IDS_ERROR + n, buf)) {

    	// error string doesn't exist.  if it is a net error use
	// net string, if not that use generic error string

//	  if (n > ERROR_SHARE)
//		  wsLoadSz(IDS_ERROR + ERROR_SHARE, buf);
//	  else
		wsprintf(buf, "DOS error number = %d ", n);
    }

//    lstrcat(buf,"\n");

    // check for the out of disk space case

    if (n == ERROR_WRITE) {		// check for out of disk space

    	dprintf("free space on disk %ld\n", DosDiskFreeSpace(0));

    	if (DosDiskFreeSpace(0) < 50000)
		lstrcat(buf, "Insufficient disk space; exit Install and delete one or more files to increase available disk space. ");
    }

    lstrcat(buf, file);		// add the file name
    lstrcat(buf,"\n");

    // pass these through globals

    pErrMsg = buf;

//    if ( ( lpfn = MakeProcInstance ( wsErrorDlg , hInstWS ) ) == NULL ) goto ERR_EXIT;
//    result = DialogBox(hInstWS, "COPYERROR" , hwndWS, lpfn);
//    FreeProcInstance(lpfn);

    MessageBeep(0);
    msg = MessageBox ( hwndWS , buf , "File Copy Error" , MB_ABORTRETRYIGNORE |
				MB_SYSTEMMODAL | MB_ICONHAND );

    dprintf ( " Copy Error Message Box Status = %i\n" , msg );

    result = FC_ABORT;
    if ( msg == IDRETRY ) result = FC_RETRY;
    if ( msg == IDIGNORE ) result = FC_IGNORE;

    dprintf ( " Copy Error return value = %i\n" , result);


ERR_EXIT:
    return result;
}


/*----------------------------------------------------------------------------*\
|                                                                              |
| wsInsertDisk()                                                               |
|                                                                              |
|   Handles errors, as the result of copying files.                            |
|                                                                              |
\*----------------------------------------------------------------------------*/
WORD PUBLIC wsInsertDisk(int n, LPSTR szSrcPath)
{
    int   result = 0;
    int   msg,i,j;
    char plstr[]="Please insert \0";
    char ach[2];
    char buf[MAXSTR];
    char buf2[MAXSTR];

    cDisk  = (char)n;

    szEdit = szSrcPath;

    ach[0] = cDisk;
    ach[1] = 0;

    infGetProfileString(NULL,"disks",ach,buf);

    infParseField(buf,2,buf2);

    for ( i = 0 ; plstr[i] != '\0' ; i++ ) buf[i] = plstr[i];
    for ( j = 0 ; buf2[j] != '\0' ; j++ ) buf[i+j] = buf2[j];
    buf[i+j] = '\0';

    MessageBeep(0);

    msg = MessageBox ( hwndWS , buf , "Insert Disk" , MB_OKCANCEL |
						       MB_ICONEXCLAMATION );

    dprintf ( " InsertDisk Message Box Status = %i\n" , msg );

    if ( msg == IDOK ) result = FC_RETRY;
	else result = FC_IGNORE;

    dprintf ( " InsertDisk return value = %i\n" , result);

    return result;
}

/*----------------------------------------------------------------------------*\
|   wsDiskDlg( hDlg, uiMessage, wParam, lParam )                               |
|                                                                              |
|   Arguments:                                                                 |
|       hDlg            window handle of about dialog window                   |
|       uiMessage       message number                                         |
|       wParam          message-dependent                                      |
|       lParam          message-dependent                                      |
|                                                                              |
|   Returns:                                                                   |
|       TRUE if message has been processed, else FALSE                         |
|                                                                              |
\*----------------------------------------------------------------------------*/

BOOL FAR PASCAL wsDiskDlg(HWND hDlg, unsigned uiMessage, WORD wParam, long lParam)
{

    switch (uiMessage)
    {
	case WM_COMMAND:
	    switch (wParam)
	    {
		case ID_OK:

		    // szEdit points to the path that will be retried

		    GetDlgItemText(hDlg, ID_EDIT, szEdit, MAXSTR);
		    lstrcpy(szDiskPath, szEdit);	// and make this the default

                    EndDialog(hDlg, FC_RETRY);
		    break;

		case ID_CANCEL:

		    EndDialog(hDlg, FC_IGNORE);
		    break;
	    }
	    return TRUE;

        case WM_INITDIALOG:
	    {
            /*
             *  now look in the [disks] section for the disk name
             *  the disk name is the second field.
             */
	    char ach[2];
	    char buf[MAXSTR];
	    char buf2[MAXSTR];

            ach[0] = cDisk;
            ach[1] = 0;

	    infGetProfileString(NULL,"disks",ach,buf);

            infParseField(buf,2,buf2);

            SetDlgItemText(hDlg,ID_TEXT,buf2);
	    SetDlgItemText(hDlg,ID_EDIT,szEdit);

    //	      wsDlgInit(hDlg);

	    MessageBeep(0);

            return TRUE;
	    }
    }
    return FALSE;
}

/*----------------------------------------------------------------------------*\
|   wsYield()                                                                  |
|                                                                              |
|   Description:                                                               |
|       Handle any messages in our Que, return when the Que is empty           |
|                                                                              |
|   Returns:                                                                   |
|       FALSE if a WM_QUIT message was encountered, TRUE otherwise             |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL PUBLIC wsYield()
{
    MSG     msg;
    BOOL    retval = TRUE;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

        if ( msg.message == WM_QUIT )
           retval = FALSE;

    	if (CheckSpecialKeys(&msg))
		continue;

        TranslateMessage(&msg);
        DispatchMessage(&msg);

    }
    return retval;
}

/***************************************************************************
 * BOOL PUBLIC CheckSpecialKeys(PMSG pmsg)
 * 
 * returns:
 *	TRUE	it was special and I delt with it
 *	FALSE	it wasn't special so pass it on
 *
 **************************************************************************/
BOOL PUBLIC CheckSpecialKeys(LPMSG lpmsg)
{
	static HWND hwndOldActive = NULL;

        if (lpmsg->message != WM_KEYDOWN)
		return FALSE;

        switch(lpmsg->wParam) {

	// exit keys

        case VK_F3:
		PostMessage(hwndWS,WM_COMMAND,ID_EXITSETUP,0L);
		break;

	default:
		return FALSE;
        }
	return TRUE;		// yes, this was a special key
}

#define CBSECTORSIZE	512
#define INT13_READ	2

#ifdef CHECK_FLOPPY

/*--------------------------------------------------------------------------
									    
  IsValidDiskette() - 						    
									    
--------------------------------------------------------------------------*/

BOOL NEAR IsValidDiskette(int iDrive)
{
  char	    buf[CBSECTORSIZE];

  iDrive |= 0x0020;	// make lower case

  iDrive -= 'a';	// A = 0, B = 1, etc. for BIOS stuff

  return MyReadWriteSector(buf, INT13_READ, iDrive, 0, 0, 1);
}


BOOL NEAR DosRemoveable(int iDrive)
{
  union REGS inregs, outregs;
  struct SREGS segregs;

  iDrive |= 0x0020;	// make lower case

  iDrive -= 'a';	// A = 0, B = 1, etc. for BIOS stuff
  iDrive++;
  inregs.h.ah = 0x1c;
  inregs.h.dl = (char)iDrive;
  intdosx( &inregs, &outregs, &segregs );
  return ( outregs.h.al != 0xff );
}


/*  BOOL IsDiskInDrive(char cDisk)
 *
 *  Is the specifed disk in the drive
 *
 *  ENTRY:
 *
 *  cDisk        : what disk required to be in the drive (logical)
 *
 *  return TRUE if the specifed disk is in the drive
 *         FALSE if the wrong disk is in the drive or disk error
 *
 */
BOOL NEAR IsDiskInDrive(int iDisk)
{

    if ((iDisk  >= 'A' && iDisk <= 'Z') || 
    	(iDisk  >= 'a' && iDisk <= 'z')) {

	    if (DosRemoveable(iDisk)) {

	        if (!IsValidDiskette(iDisk))
        		return FALSE;
	    }


	    return TRUE;
    }

    return TRUE;	// for non drive letters assume a path
    			// and thus always in.
}

#endif 

void FAR PASCAL catpath(PSTR path, PSTR sz)
{
    //
    // Remove any drive letters from the directory to append
    //
    if ( sz[1] == ':' )
       sz+=2;

    //
    // Remove any current directories ".\" from directory to append
    //
    while (sz[0] == '.' && SLASH(sz[1]))
        sz+=2;

    //
    // Dont append a NULL string or a single "."
    //
    if (*sz && !(sz[0] == '.' && sz[1] == 0))
    {
       if ( (!SLASH(path[lstrlen(path)-1])) && ((path[lstrlen(path)-1]) != ':') )
          lstrcat(path,CHSEPSTR);
       lstrcat(path,sz);
    }
}


PSTR FAR PASCAL FileName(PSTR szPath)
{
    PSTR   sz;

    for (sz=szPath; *sz; sz++)
        ;
    for (; sz>=szPath && !SLASH(*sz) && *sz!=':'; sz--)
        ;
    return ++sz;
}

PSTR FAR PASCAL StripPathName(PSTR szPath)
{
    PSTR   sz;

    sz = FileName(szPath);

    if (sz > szPath+1 && SLASH(sz[-1]) && sz[-2] != ':')
        sz--;

    *sz = 0;
    return szPath;
}

WORD H2D ( LPSTR szCHKSUM )
{
    WORD nHexWord=0;
    int iChar;
    int iNumChar=0;

    while ( *szCHKSUM )
    {
	iChar = (int)*szCHKSUM ;
	iChar = tolower ( iChar );

	if ( iChar >= (int)'0' && iChar <= (int)'9' )
	{
	    nHexWord = ( nHexWord << 4 ) + ( iChar - (int)'0' );
	}
	else
	{
	    if ( iChar >= (int)'a' && iChar <= (int)'f' )
	    {
		nHexWord = ( nHexWord << 4 ) + ( iChar - (int)'a' ) + 10;
	    }
	    else
	    {
		return ( 0 );
	    }
	}
	if ( iNumChar++ > 4 ) return ( 0 );
	szCHKSUM++;
    }
    return ( nHexWord );
}

void VirusError ( LPSTR szName )
{
    char buf[MAXSTR];
    static char plstr[]="The ";
    static char plstr2[]=" file has been either damaged or infected by a virus.  Please read the README.TXT file on the install diskette.";

    buf[0]='\0';
    lstrcat ( buf, plstr );
    lstrcat ( buf, szName );
    lstrcat ( buf, plstr2 );

    MessageBeep ( 0 );

    if ( MessageBox ( hwndWS , buf , "Virus Attack!", MB_OK |
				    MB_ICONEXCLAMATION ) == 0 ) OutOfMem ();
    return;
}

void OutOfMem ( void )
{
    int msg;

    MessageBeep ( 0 );
    msg = MessageBox ( hwndWS , "Out of Memory - close some windows and try again",
		    "Out of Memory", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL );

#if defined ( DEBUG )
    if ( msg == 0 )
    {
	dprintf ( "Out of Memory\n" );
	FatalExit ( -1 );
    }
#else
    PostQuitMessage ( 1 );
#endif

    return;
}

WORD DoCheckSum ( WORD nCheckSum , WORD *nSeed , LPSTR lpBuf , WORD size )
{
    WORD n,nTemp;
    long int liSeed;
    LPSTR szTemp;
    int n2;

    if ( size < 3 ) return ( nCheckSum );
    size = ( size - 1 ) >> 1;
    szTemp = lpBuf;
    liSeed = randJCK ( (long int)*nSeed );
    for ( n = 0 , n2 = 0 ; n < size ; n++ , n2++ )
    {
	nTemp = ((WORD)(*szTemp)) << 8;
	szTemp++;
	nTemp = nTemp | (WORD)(*szTemp);
	szTemp++;
	if ( n2 == 10 )
	{
	   n2 = 0;
	   liSeed = randJCK ( 0L );
	}
	nCheckSum +=  ((WORD)liSeed) ^ nTemp;
    }
    *nSeed = (WORD)( (unsigned long int)randJCK ( -1L ) & 0x0000ffff );
    return ( nCheckSum );
}

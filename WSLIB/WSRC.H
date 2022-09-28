/*----------------------------------------------------------------------------*\
|   ws.c  - Windows Setup						       |
|                                                                              |
|   History:                                                                   |
|	03/09/89 toddla     Created					       |
|                                                                              |
\*----------------------------------------------------------------------------*/

#define BUTTON_DLG_SIZE	24

/*
 *  Dialog resource ID's
 */
#define DLG_ABOUT        10

#define DLG_WINSETUP     11
#define DLG_COPYAPPS     12
#define DLG_INSERTDISK   16
#define DLG_CONFIGSYS    17
#define DLG_SHELL        18
#define DLG_COPYERROR    19
#define DLG_MAINTAIN     20
#define DLG_CHANGESET    21
#define DLG_VIEWSET      22
#define DLG_SYSEDIT      24
#define DLG_SYSNOSAVE    25
#define DLG_SELECTAPPS   26
#define DLG_APPS         27
#define DLG_PIFSEARCH    28
#define DLG_NOTSETUP     29
#define DLG_ASKEXIT      30
#define DLG_SYSOLD       31
#define DLG_EXITRESTART  32
#define DLG_EXITREBOOT   33
#define DLG_SYSBACKEDIT  34

/*
 *  Dialog control ID's
 */
#define ID_NULL     -1
#define ID_OK        1
#define ID_CANCEL    2
#define ID_HELP      3
#define ID_ABORT     4
#define ID_RETRY     5
#define ID_IGNORE    6
#define ID_EXITSETUP 7

#define ID_LB	    10
#define ID_EDIT     11
#define ID_TEXT     12
#define ID_CB	    13
#define ID_LB_FOUND  14
#define ID_LB_USE    15
#define ID_ADD       16
#define ID_ADD_ALL   17
#define ID_REMOVE    18

#define ID_INSTALLWINDOWS  101
#define ID_INSTALLAPPS	   102
#define ID_INSTALLPIF	   103
#define ID_INSTALLPRINTER  104
#define ID_INSTALLFONT	   105
#define ID_README	   106
#define ID_SETTINGS	   107

#define ID_COPY 	   200
#define ID_ALL             201
#define ID_PRINT           202

#define ID_MODIFY          205
#define ID_REVIEW          206
#define ID_NOMODIFY        207

#define ID_EDIT1           209

#define ID_STATUS0   4000
#define ID_STATUS1   (ID_STATUS0 + 1)
#define ID_STATUS2   (ID_STATUS0 + 2)
#define ID_STATUS3   (ID_STATUS0 + 3)
#define ID_STATUS4   (ID_STATUS0 + 4)

/*
 * stuff for the setting dialog
 */
#define ID_DISPLAY         301
#define ID_KEYBOARD        302
#define ID_MOUSE           303
#define ID_HIMEM           306
#define ID_MEMORY          307
#define ID_XMS             308
#define ID_DOSVER          309
#define ID_SWIN            310
#define ID_SDOS            311
#define ID_REBOOT          312
#define ID_NETWORK         313
#define ID_SWAPFILE        314

/*
 *  Menu/cursor/icon/accelerators ID's
 */
#define MENU_EXIT       101
#define MENU_ABOUT      103
#define MENU_DEBUG      104
#define MENU_FIRSTTIME  105
#define MENU_TILED      106
#define MENU_WASH       107
#define MENU_SOLID      108
#define MENU_DONTCOPY   109
#define MENU_CHANGE	111
#define	MENU_README	112
#define	MENU_INSTALL	113
#define MENU_HELPINDEX 	114
#define	MENU_HELPONHELP	115
#define	MENU_386	116
#define	MENU_NET	117


/*
 *  Menu/cursor/icon/accelerators ID's
 */
#define ID_ICON      201
#define ID_MENU      202
#define ID_LOGO      203
#define ID_TILE      204
#define ID_HELPTEXT  205
#define ID_WINAPP    206
#define ID_OLDAPP    207
#define ID_MAINTAIN  208

/*
 *  Custom resource ID's
 */
#define RT_HELPTEXT  4096

/*
 *  Class ID's
 */
#define CLS_APP             MAKEINTATOM(1)
#define CLS_DESK            MAKEINTATOM(2)
#define CLS_INSTRUCTIONS    MAKEINTATOM(3)
#define CLS_SBUTTON         "sbutton"
#define CLS_HELP            "rtf"

/*
 *  String ID's
 */
#define IDS_VERSION             301
#define IDS_APPNAME             302
#define IDS_INSERTDISK          304
#define IDS_INTERNALERROR       306
#define IDS_CONFIGSYS           308
#define IDS_AUTOEXECBAT         309
#define IDS_SEARCHINGPATH       310
#define IDS_SEARCHINGDISK       311
#define IDS_SEARCHDRIVE         313
#define IDS_SEARCHPATH          314
#define IDS_SEARCHALL           315
#define IDS_COPYING             317
#define IDS_CONTINUOUSSCALING   318
#define IDS_WAITCOPY            319
#define IDS_WAITCOPYFONT        320
#define IDS_NOSETUPINF          321
#define IDS_SYSTEM              322
#define IDS_KERNEL              323
#define IDS_SHELL               324
#define IDS_BOOT                325
#define IDS_CONFIGWIN           326
#define IDS_AUTOEXECWIN         327
#define IDS_F1HELP              328
#define IDS_DISKS               329
#define IDS_APPS                330
#define IDS_PIF                 331
#define IDS_WINCOPY             332
#define IDS_FONTS               333
#define IDS_HIMEMYES            334
#define IDS_HIMEMNO             335
#define IDS_CONTROLCLS          336
#define IDS_CONTROLEXE          337
#define IDS_PARSLEY             338
#define IDS_APPSEARCH           339
#define IDS_NOTINSTALLED        340
#define IDS_ESCEXIT             341
#define IDS_README              342
#define IDS_INSTRUCTIONS        343
#define IDS_HELPFILE            344
#define IDS_OLDAPPSGRP          345
#define IDS_WINAPPSGRP          346
#define IDS_OLDAPPS             347
#define IDS_WINAPPS             348
#define IDS_386DISPLAY 		350
#define IDS_386MOUSE            351
#define IDS_386KEYBOARD         352
#define IDS_386SECTION          353
#define IDS_PROGMAN_GROUP	354
#define IDS_OEMDISPLAY		355
#define IDS_OEMMOUSE		356
#define IDS_OEMDISKPROMPT	357
#define IDS_DEFDRIVE		358
#define IDS_OUTOFDISK		359
#define IDS_AUTO1		363
#define IDS_AUTO2		364
#define IDS_CONFIG1		365
#define IDS_CONFIG2		366
#define IDS_GRABBER286          367
#define IDS_GRABBER386          368
#define IDS_EXITNOTSETUP	369
#define IDS_EXITASKEXIT		370
#define IDS_EXITCAP		371
#define IDS_NOINF		372
#define IDS_MODIFIEDBOTH	373
#define IDS_MODIFIEDAUTO	374
#define IDS_MODIFIEDCONFIG	375
#define IDS_NEEDCHANGEBOTH	376
#define IDS_NEEDCHANGEAUTO	377
#define IDS_NEEDCHANGECONFIG	378
#define IDS_DISPLAY		379
#define IDS_DEVICES		380
#define IDS_OLDWINVER		381
#define IDS_NEWWINVER		382
#define IDS_APPSCANCEL		383
#define IDS_SPARTMENU		384
#define IDS_SPARTEXE		385
#define IDS_HELPNOTFOUND	386
#define IDS_REBOOT		387
#define IDS_SYSCHANGED		388
#define IDS_NETWORK		389
#define IDS_NETWORKDRV		390
#define IDS_WIN386		391
#define IDS_NET			392
#define IDS_NOINFMEM		393
#define IDS_WINCNF		394
#define IDS_386EBIOS		395
#define IDS_SWAPFILETEMP	396
#define IDS_SWAPFILEPERM	397
#define IDS_SWAPFILENONE	398
#define IDS_SWAPFILE		399
#define IDS_ADDOLD		400
#define IDS_ADDWIN		401
#define IDS_CONTROLERROR	402
#define IDS_LIMDETECTED1	403
#define IDS_LIMDETECTED2	404
#define IDS_LIMDETECTED3	405
#define IDS_LIMWARNING1		406
#define IDS_LIMWARNING2		407
#define IDS_LIMWARNING3		408
#define IDS_LIMWARNING4		409

#define IDS_LIMFLAME1		410
#define IDS_LIMFLAME2		411
#define IDS_LIMFLAME3		412
#define IDS_LIMFLAME4		413

#define IDS_MODIFYREMOTE	420
#define IDS_COPYHIMEM		421

#define IDS_FOUNDAPP            430
#define IDS_BUILDPMG            431
#define IDS_WRITEPIF            432
#define IDS_OS2WARNING		433


#define IDS_HELP     1000
#define IDS_ERROR    20000

/*
 *  Help id's
 */

#define HLP_NULL         -1
#define HLP_ERROR        1
#define HLP_COPYING      2
#define HLP_COPYERROR    3
#define HLP_README       4
#define HLP_SEARCHING    5
#define HLP_BLANK	 6

#define HLP_PRINT        50

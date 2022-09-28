;****************************************************************************
;*                                                                          *
;*  WFDOSDIR.ASM -                                                          *
;*                                                                          *
;*      Directory searching primitives                                      *
;*                                                                          *
;****************************************************************************

?PLM=1      ; PASCAL Calling convention is DEFAULT
?WIN=1      ; Windows calling convention
?386=0      ; Use 386 code?

include cmacros.inc

; The following structure should be used to access high and low
; words of a DWORD.  This means that "word ptr foo[2]" -> "foo.hi".

LONG    struc
lo      dw      ?
hi      dw      ?
LONG    ends

FARPOINTER      struc
off     dw      ?
sel     dw      ?
FARPOINTER      ends

ifndef SEGNAME
    SEGNAME equ <TEXT>
endif

if ?386
    createSeg _%SEGNAME, CodeSeg, word, use16, CODE
else
    createSeg _%SEGNAME, CodeSeg, word, public, CODE
endif

NOT_SUPPORTED     =  2h      ; Return code from IsDeviceRemote function.
REMOTE            =  3h      ; Return code for remote drive found.
TRUE              =  1h      ; TRUE Definition
FALSE             =  0h      ; False Definition.

;=============================================================================
EXTRN	__aNlshr:NEAR
EXTRN	__aNlshl:NEAR

sBegin DATA

VolExtendedFCB  db  0FFh
                db  0, 0, 0, 0, 0
                db  1000b
                db  0
                db  11 dup('?')
                db  5 dup(0)
                db  11 dup('?')
                db  9 dup(0)

DTA		db  64 dup(0)

$S164_ia1	DW	0600H
$S165_ia0	DW	0405H
$S166_ia1ma0	DW	01fbH
$S167_ic	DW	06c3H
$S168_ix1	DD	00H
$S169_ix0	DD	00H


sEnd   DATA

;=============================================================================

sBegin CodeSeg

assumes CS,CodeSeg
assumes DS,DATA

;*--------------------------------------------------------------------------*
;*                                                                          *
;*  DosFindFirst() -                                                        *
;*                                                                          *
;*--------------------------------------------------------------------------*

; Get the first directory entry.

cProc DosFindFirst, <FAR, PUBLIC>

ParmD lpDest
ParmD szFileSpec
ParmW attrib

cBegin DosFindFirst
            push    ds
            lds     dx,lpDest
            mov     ah,1Ah          ; Set DTA
            int     21h

            mov     cx,attrib       ; Find First File
            lds     dx,szFileSpec   ; Path = szFileSpec
            mov     ah,4Eh
            int     21h
            jc      fferr
            mov     ax,1
            jmp     short ffdone

fferr:
            xor     ax,ax           ; Return zero on error
ffdone:
            pop     ds
cEnd  DosFindFirst


;*--------------------------------------------------------------------------*
;*                                                                          *
;*  DosFindNext() -                                                         *
;*                                                                          *
;*--------------------------------------------------------------------------*

cProc DosFindNext, <FAR, PUBLIC>

ParmD lpDest

cBegin DosFindNext
            push    ds
            lds     dx,lpDest
            mov     ah,1Ah          ; Set DTA
            int     21h
            pop     ds

            les     bx,lpDest       ; ES:BX = lpDest
            mov     ah,4Fh          ; Find Next File
            int     21h
            mov     ax,1
            jnc     FNExit          ; Exit if no error
FNErr:
            xor     ax,ax           ; Return FALSE
FNExit:
cEnd   DosFindNext

;*--------------------------------------------------------------------------*
;*                                                                          *
;*  DosMkDir()                                                              *
;*                                                                          *
;*--------------------------------------------------------------------------*

cProc DosMkDir, <FAR, PUBLIC>
ParmD   szDir
cBegin DosMkDir
        lds     dx,szDir
        mov     ah,39h
        int     21h
        jc      mderror
        xor     ax,ax
        jmp     short mdexit

mderror:
        mov     ah,59h
        xor     bx,bx
        int     21h

mdexit:

cEnd   DosMkDir

;*--------------------------------------------------------------------------*
;*                                                                          *
;*  MyReadWriteSector() -                                                   *
;*                                                                          *
;*--------------------------------------------------------------------------*

; Uses INT 13h to read/write an absolute sector.

cProc MyReadWriteSector, <PUBLIC, FAR>, <SI,DI>

ParmD   lpBuffer
ParmW   Function                       ; 02 for Read and 03 for Write
ParmW   Drive
ParmW   Cylinder
ParmW   Head
ParmW   Count

LocalW  wRetryCount 

cBegin	MyReadWriteSector
        ; Retry this operation three times.
        mov     wRetryCount,4

MRWS_TryAgain:
        mov     ax,Count                ; AL = Number of sectors
        mov     ah,byte ptr Function    ; AH = Function #
        mov     ch,byte ptr Cylinder    ; CH = Starting Cylinder
        mov     cl,1                    ; CL = Starting Sector

        mov     dx,Drive        ; DL = INT 13h drive designation

        mov     dh,byte ptr Head        ; DH = Head #
        les     bx,lpBuffer             ; ES:BX = Buffer
        int     13h
        
        mov     ax, 1                   ; success

        jnc     MRWS_End_success        ; Problems?
        dec     wRetryCount             ; Yup, retry
        jz      MRWS_End_fail           ; Are we out of retries?

        xor     ah,ah                   ; Nope, reset the disk
        mov     dx,Drive
        int     13h
        jmp     short MRWS_TryAgain
MRWS_End_fail:
        xor     al,al           ; AH contains the error code, if any.
MRWS_End_success:

cEnd	MyReadWriteSector

;*--------------------------------------------------------------------------*
;*                                                                          *
;*  GetCurrentDrive() -                                                     *
;*                                                                          *
;*--------------------------------------------------------------------------*

cProc GetCurrentDrive, <FAR, PUBLIC>

cBegin GetCurrentDrive
        mov     ah,19h              ; Get Current Drive
        int     21h
        sub     ah,ah               ; Zero out AH
cEnd   GetCurrentDrive

;*--------------------------------------------------------------------------*
;*                                                                          *
;*  GetCurrentDirectory() -                                                 *
;*                                                                          *
;*--------------------------------------------------------------------------*

cProc DosCwd, <FAR, PUBLIC>, <SI, DI>

ParmD lpDest

cBegin	DosCwd
            push    ds                  ; Preserve DS

            call    GetCurrentDrive
            mov     si,ax               ; SI = Current drive

            les     di,lpDest           ; ES:DI = lpDest
            push    es
            pop     ds                  ; DS:DI = lpDest
            cld
            mov     ax,si               ; AX = Current drive
            inc     al                  ; Convert to logical drive number
            mov     dl,al               ; DL = Logical Drive Number
            add     al,'@'              ; Convert to ASCII drive letter
            stosb
            mov     al,':'
            stosb
            mov     al,'\'              ; Start string with a backslash
            stosb
            mov     byte ptr es:[di],0  ; Null terminate in case of error
            mov     si,di               ; DS:SI = lpDest[1]
            mov     ah,47h              ; Get Current Directory
            int     21h
            jc      CDExit              ; Skip if error
            xor     ax,ax               ; Return FALSE if no error
CDExit:
            pop     ds                  ; Restore DS
cEnd	DosCwd

;*--------------------------------------------------------------------------*
;*                                                                          *
;*  SetCurrentDrive() -                                                     *
;*                                                                          *
;*--------------------------------------------------------------------------*

; Returns the number of drives in AX.

cProc SetCurrentDrive, <FAR, PUBLIC>

ParmW Drive

cBegin	SetCurrentDrive
            mov     dx,Drive
            mov     ah,0Eh              ; Set Current Drive
            int     21h
            sub     ah,ah               ; Zero out AH
cEnd	SetCurrentDrive

;*--------------------------------------------------------------------------*
;*                                                                          *
;*  SetCurrentDirectory() -                                                 *
;*                                                                          *
;*--------------------------------------------------------------------------*

cProc DosChDir, <FAR, PUBLIC>

ParmD lpDirName

cBegin	DosChDir
            push    ds                  ; Preserve DS
            lds     dx,lpDirName        ; DS:DX = lpDirName

            mov     bx,dx
            mov     ax,ds:[bx]
            cmp     ah,':'
            jnz     cdnodrive

            ;
            ;       Convert drive letter to drive index
            ;
            or      al,20h
            sub     al,'a'
            xor     ah,ah

            push    dx
            cCall   SetCurrentDrive,<ax>
            pop     dx
cdnodrive:
            mov     ah,3Bh              ; Change Current Directory
            int     21h
            jc      SCDExit             ; Skip on error
            xor     ax,ax               ; Return FALSE if successful
SCDExit:
            pop     ds                  ; Restore DS
cEnd	DosChDir

;*--------------------------------------------------------------------------*
;*                                                                          *
;*  DosValidDir()                                                           *
;*                                                                          *
;*--------------------------------------------------------------------------*

cProc DosValidDir, <FAR, PUBLIC>, <SI, DI>
ParmD       szDir
LocalV      szCwd, 128
cBegin	DosValidDir
    lea     si,szCwd
    cCall   DosCwd,<ss,si>
    push    szDir.sel
    push    szDir.off
    call    DosChDir
    or      ax,ax
    pushf
    cCall   DosChDir,<ss,si>
    ;
    ;   return TRUE if DosChdir returns 0, FALSE otherwise
    ;
    xor     ax,ax                ; don't care about this return val.
    popf
    jnz     vdexit
    inc     ax
vdexit:

cEnd	DosValidDir

;*--------------------------------------------------------------------------
;*
;*  LONG DosDiskFreeSpace(Drive)
;*
;*  note:
;*      Drive == 0      default
;*      Drive == 1      A
;*      Drive == 2      B
;*--------------------------------------------------------------------------

; Returns the number of bytes free in DX:AX

cProc DosDiskFreeSpace, <FAR, PUBLIC>
ParmW Drive
cBegin DosDiskFreeSpace
            mov     dx,Drive
            mov     ah,36h              ; Set Current Drive
            int     21h
            cmp     ax, 0ffffh          ; bogus drive?
            je      error
            mul     cx                  ;
            mul     bx
            jmp     done                ; DX:AX contains free space
error:
            mov     dx, ax              ; return dx:ax = -1L
done:
cEnd   DosDiskFreeSpace

;*--------------------------------------------------------------------------*
;*                                                                          *
;*  DosExit(ec);                                                            *
;*                                                                          *
;*  Terminate program                                                       *
;*                                                                          *
;*--------------------------------------------------------------------------*

cProc DosExit, <FAR, PUBLIC>
ParmW   ec
cBegin	DosExit
        mov     al,byte ptr ec
        mov     ah,4Ch
        int     21h
cEnd	DosExit

;*--------------------------------------------------------------------------*
;*                                                                          *
;*  randJCK(iseed);							    *
;*                                                                          *
;*  Generate a 16 bit pseudo randum sequence				    *
;*                                                                          *
;*--------------------------------------------------------------------------*

cProc _randJCK, <FAR, PUBLIC>, <SI, DI>
;ParmW	 iseed
cBegin	_randJCK
;	 push	 bp
;	 mov	 bp,sp
;	 sub	 sp,4
;	 push	 di
;	 push	 si
;	r = 4
; Line 86
	cmp	WORD PTR [bp+8],0
	jge	$JCC12
	jmp	$L10173
$JCC12:
; Line 87
	jl	$L20000
	jg	$L20001
	cmp	WORD PTR [bp+6],0	;r
	je	$L20000
$L20001:
; Line 104
	mov	ax,WORD PTR [bp+6]	;r
	mov	dx,WORD PTR [bp+8]
	mov	WORD PTR $S168_ix1,ax
	mov	WORD PTR $S168_ix1+2,dx
; Line 105
	and	ah,7
	sub	dx,dx
	mov	WORD PTR $S169_ix0,ax
	mov	WORD PTR $S169_ix0+2,dx
; Line 106
	mov	cx,ax
	mov	bx,dx
	mov	ax,WORD PTR [bp+6]	;r
	mov	dx,WORD PTR [bp+8]
	sub	ax,cx
	sbb	dx,bx
	mov	al,ah
	mov	ah,dl
	mov	dl,dh
	shl	dh,1
	sbb	dh,dh
	sar	dx,1
	rcr	ax,1
	sar	dx,1
	rcr	ax,1
	sar	dx,1
	rcr	ax,1
	mov	WORD PTR $S168_ix1,ax
	mov	WORD PTR $S168_ix1+2,dx
; Line 107
	jmp	$L10173
$L20000:
; Line 93

	mov	bx,WORD PTR $S168_ix1
	mov	ax,WORD PTR $S164_ia1
	mul	bx
	mov	si,ax
	mov	di,dx
	mov	bx,WORD PTR $S169_ix0
	mov	ax,WORD PTR $S165_ia0
	mul	bx
	mov	cx,ax
	mov	bx,dx
	mov	ax,WORD PTR $S166_ia1ma0
	mov	dx,WORD PTR $S169_ix0
	sub	dx,WORD PTR $S168_ix1
	mul	dx
	add	si,cx
	adc	di,bx
	add	si,ax
	adc	di,dx
	add	cx,WORD PTR $S167_ic
	adc	bx,0000h
	mov	ax,cx
	and	ah,07h
	mov	WORD PTR $S169_ix0,ax
	sub	cx,ax
	sbb	bx,0000h
	rcl	cx,1
	rcl	bx,1
	rcl	cx,1
	rcl	bx,1
	rcl	cx,1
	rcl	bx,1
	rcl	cx,1
	rcl	bx,1
	rcl	cx,1
	rcl	bx,1
	add	bx,si
	and	bx,07ffh
	mov	WORD PTR $S168_ix1,bx
	mov	cl,5
	rol	ax,cl
	shr	bx,1
	rcr	ax,1
	shr	bx,1
	rcr	ax,1
	shr	bx,1
	rcr	ax,1
	shr	bx,1
	rcr	ax,1
	shr	bx,1
	rcr	ax,1
	mov	dx,bx
	jmp	short	$L106jk


; Line 99
$L10173:
; Line 101
	mov	ax,WORD PTR $S168_ix1
	mov	dx,WORD PTR $S168_ix1+2
	mov	dh,dl
	mov	dl,ah
	mov	ah,al
	sub	al,al
	shl	ax,1
	rcl	dx,1
	shl	ax,1
	rcl	dx,1
	shl	ax,1
	rcl	dx,1
	add	ax,WORD PTR $S169_ix0
	adc	dx,WORD PTR $S169_ix0+2
; Line 109
$L106jk:
;	 pop	 si
;	 pop	 di
;	 mov	 sp,bp
;	 pop	 bp

cEnd  _randJCK

sEnd CodeSeg

end

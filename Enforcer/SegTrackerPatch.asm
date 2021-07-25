*                             Enforcer  by
*                             Michael Sinz
*
*                         Copyright 1992-1998
*                         All Rights Reserved
*
*****************************************************************************
*                                                                           *
* Permission is hereby granted to distribute the Enforcer archive           *
* containing the executables and documentation for non-commercial purposes  *
* so long as the archive and its contents are not modified in any way.      *
*                                                                           *
* Enforcer and related tools are not in the public domain.                  *
*                                                                           *
* Enforcer and related tools may not be distributed for a profit.           *
*                                                                           *
*****************************************************************************
*
*******************************************************************************
*
* Well, the patches to LoadSeg and NewLoadSeg and UnLoadSeg are
* a bit more complex than can be done in normal C...
*
*******************************************************************************
*
		xref	_TrackSeg
		xref	_UnTrackSeg
*
*******************************************************************************
*
* Patch header for the LoadSeg() DOS function...
*
_OldLoadSeg:	xdef	_OldLoadSeg
		dc.l	0
_MyLoadSeg:	xdef	_MyLoadSeg
		move.l	_OldLoadSeg(pc),a0	; Get old function
DoLoadSeg:	move.l	d1,-(sp)		; Save name...
		jsr	(a0)			; Call it...
		move.l	(sp)+,a0		; Restore name...
		movem.l	d0/d1,-(sp)		; Save result...
		jsr	_TrackSeg		; Track the segment (d0/a0)
		movem.l	(sp)+,d0/d1		; Restore d0/d1
		rts
*
*******************************************************************************
*
* Patch header for the NewLoadSeg() DOS function...
*
_OldNewLoadSeg:	xdef	_OldNewLoadSeg
		dc.l	0
_MyNewLoadSeg:	xdef	_MyNewLoadSeg
		move.l	_OldNewLoadSeg(pc),a0	; Get old function
		bra.s	DoLoadSeg		; Do the work...
*
*******************************************************************************
*
* Patch header for the UnLoadSeg() DOS function...
*
_OldUnLoadSeg:	xdef	_OldUnLoadSeg
		dc.l	0
_MyUnLoadSeg:	xdef	_MyUnLoadSeg
		move.l	d1,-(sp)		; Save segment...
		jsr	_UnTrackSeg		; Untrack it (d1)
		move.l	(sp)+,d1		; Get Segment back
		move.l	_OldUnLoadSeg(pc),a0	; Get old function...
		jmp	(a0)			; Do it (and return)
*
*******************************************************************************
*
		END

*                             Enforcer  by
*                             Michael Sinz
*
*                         Copyright 1992-1998
*                         All Rights Reserved
*
*****************************************************************************
*                                                                           *
* Licensed under the Apache License, Version 2.0 (the "License");           *
* you may not use this file except in compliance with the License.          *
* You may obtain a copy of the License at                                   *
*                                                                           *
*    http://www.apache.org/licenses/LICENSE-2.0                             *
*                                                                           *
* Unless required by applicable law or agreed to in writing, software       *
* distributed under the License is distributed on an "AS IS" BASIS,         *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
* See the License for the specific language governing permissions and       *
* limitations under the License.                                            *
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

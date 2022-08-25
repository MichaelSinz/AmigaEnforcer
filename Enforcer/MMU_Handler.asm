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
* Ok, so here is the really tricky part of MMU
*
*******************************************************************************
*
		include	"exec/types.i"
		include	"exec/lists.i"
		include	"exec/nodes.i"
		include	"exec/tasks.i"
		include	"exec/macros.i"
		include	"exec/execbase.i"
		include	"exec/ables.i"
		include	"libraries/configvars.i"
*
		include	"MMU_rev.i"
*
*******************************************************************************
*
		opt	p=68040			; Use 68040 instructions
*
*******************************************************************************
*
* ULONG GetPageEntry(ExecBase, Address)
* d0                 a6        a0
*
* This routine will look up the page entry in the MMU tables of the 68040
* and return the MMU table entry for that page.  Note that this will be the
* RAW MMU entry for that page.  No checks are made for invalid MMU pages.
* If the MMU is not on, this function will return 0.
*
_GetPageEntry:	xdef	_GetPageEntry
		move.l	a5,-(sp)
		lea	GetPageEntry(pc),a5
		JSRLIB	Supervisor
		move.l	(sp)+,a5
		rts
*
GetPageEntry:	moveq.l	#0,d0			; Assume it did not work
		movec.l	tc,d1			; Get MMU setting...
		tst.w	d1			; Is it on?
		bpl.s	gpe_RTE			; If not, exit...
		btst.l	#14,d1			; Check for 4K pages
		bne.s	gpe_RTE			; If not 4K, we exit...
*
* So we have 4K pages like we are supposed to.  Lets continue...
*
		move.l	a0,d0			; Get base address...
		bsr.s	GetPageAddress		; Get address of page entry...
		move.l	(a0),d0			; Get page entry...
gpe_RTE:	rte				; Return
*
*******************************************************************************
*
* VOID SetPageEntry(ExecBase, Address, Value)
*                   a6        d0       d1
*
* This routine will look up the page entry in the MMU tables and update
* it with the value given.
*
_SetPageEntry:	xdef	_SetPageEntry
		move.l	a5,-(sp)
		lea	SetPageEntry(pc),a5
		JSRLIB	Supervisor
		move.l	(sp)+,a5
		rts
*
SetPageEntry:	move.l	d1,a1			; Save here for a bit...
		movec.l	tc,d1			; Get MMU setting...
		tst.w	d1			; Is it on?
		bpl.s	spe_RTE			; If not, exit...
		btst.l	#14,d1			; Check for 4K pages
		bne.s	spe_RTE			; If not 4K, we exit...
*
* All set, so let us continue...
*
		bsr.s	GetPageAddress		; Get the page address
		or.w	#$0700,sr		; Protect here...
		pflusha				; Flush the ATC
		move.l	a1,(a0)			; Store new MMU entry
		cpushl	dc,(a0)			; Flush to RAM
spe_RTE:	rte				; And we are done...
*
*******************************************************************************
*
* ULONG *GetPageAddress(VOID *)
* a0                    d0
*
* This routine will return a pointer to the page entry for the
* address given.  It returns the address in a0.  d1 is trashed.
*
GetPageAddress:	movec.l	urp,a0			; Get ROOT pointer...
		bfextu	d0{0:7},d1		; Get the root index...
		add.l	d1,d1			; *2
		add.l	d1,d1			; *4
		add.l	d1,a0			; Add to root pointer...
		move.l	(a0),d1			; Get page entry
		and.w	#$FE00,d1		; Mask into the page table
		move.l	d1,a0			; Store pointer...
		bfextu	d0{7:7},d1		; Get the pointer index...
		add.l	d1,d1			; *2
		add.l	d1,d1			; *4
		add.l	d1,a0			; Add to table pointer...
		move.l	(a0),d1			; Get page entry...
		and.w	#$FF00,d1		; Mask to the pointer...
		move.l	d1,a0			; Put into address register...
		bfextu	d0{14:6},d1		; Get index into page table
		add.l	d1,d1			; *2
		add.l	d1,d1			; *4
		add.l	d1,a0			; a0 now points at the page...
		move.l	(a0),d1			; Get page entry...
		btst.l	#0,d1			; Check if bit 0 is set...
		bne.s	gpa_RTS			; If set, we are valid...
		bclr.l	#1,d1			; Check if indirect...
		beq.s	gpa_RTS			; If not indirect, A0 is valid
		move.l	d1,a0			; a0 is now the page entry...
gpa_RTS:	rts				; Return with A0 set...
*
*******************************************************************************
*
		VERSTAG	; Version string...
*
*******************************************************************************
*
* "A master's secrets are only as good as the
*  master's ability to explain them to others."  -  Michael Sinz
*
*******************************************************************************
*
		END

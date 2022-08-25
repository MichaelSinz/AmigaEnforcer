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
******* Move4K ****************************************************************
*
*   NAME
*	Move4K - Moves as much out of the lower 4K of RAM as possible
*
*   SYNOPSIS
*	On 68040 systems, as much of the lower 4K of CHIP RAM as possible
*	is removed from system use.
*
*   FUNCTION
*	On 68040 systems the MMU page sizes are 4K and 8K.  Enforcer
*	uses the 4K page size.  Since watching for hits of low memory
*	is a vital part of Enforcer, this means that the first 4K
*	of RAM will be marked invalid.  On current systems, only
*	the first 1K of RAM is invalid and thus 3K of RAM in that
*	first 4K will end up needing to be emulated in Enforcer.
*	In order to reduce the overhead that this causes (and the
*	major performance loss) this program will try to move as much
*	from that first 4K as possible and make any of the free
*	memory within the first 4K inaccessible.
*
*	Enforcer itself also has this logic, but it may be useful
*	to be able to run this program as the first program in
*	the Startup-Sequence (*AFTER* SetPatch) to try to limit
*	the number of things that may use the lower 4K of RAM.
*
*   INPUTS
*	Just run it...  Can be run from CLI or Workbench
*
*   RESULTS
*	Any available memory in the lower 4K of CHIP RAM is removed
*	plus a special graphics buffer is moved if it needs to be.
*	After running this program you may have a bit less CHIP RAM
*	than before.  You can run this program as many times as you
*	wish since it only moves things if it needs to.
*
*   NOTES
*	This program will do nothing on systems without a 68040.
*	It does not, however, check for the MMU and thus it will
*	move the lower 4K even if the CPU is not able to run Enforcer.
*
*	V39 of the operating system already does have the lowest
*	MMU page empty and thus this program will effectively do
*	nothing under V39.
*
*   SEE ALSO
*	"Eloquence is vehement simplicity"
*
*   BUGS
*	None.
*
*******************************************************************************
*
*******************************************************************************
*
* This program is used to remove as much used memory from the lower 4K as
* possible.  It is rather rude hacking, but it should work.
*
		include	"exec/types.i"
		include	"exec/execbase.i"
		include	"exec/macros.i"
		include	"exec/memory.i"
		include	"graphics/gfxbase.i"
		include	"dos/dosextens.i"
*
		include	"move4k_rev.i"
*
Start:		move.l	4,a6
		move.w	AttnFlags(a6),d1	; Get CPU flags
		btst	#AFB_68040,d1		; Is it a 68040?
		beq	Exit			; (Cheat: this also checks for
						; V37 since 68040 flag does
						; not get set in pre-V37 ;^)
*
		opt	p=68040
*
* The following code is a bit of hacking...
* It moves certain items out of memory that is below the 4K mark...
*
* It also allocates all memory that is below that mark and throws it away
*
hack_alloc:	moveq.l	#1,d0			; Allocate 1 byte...
		moveq.l	#MEMF_CHIP,d1		; CHIP memory...
		JSRLIB	AllocVec		; Allocate it...
		tst.l	d0			; Did it work?
		beq.s	hack_alloc_done		; (This should never happen)
		bftst	d0{0:20}		; Check if in lower 4K
		beq.s	hack_alloc		; If so, go for another...
		move.l	d0,a1			; This one is too far...
		JSRLIB	FreeVec			; Put it back...
hack_alloc_done:
*
* Now, we check if the MemList header is still in the lower 4K...
*
		JSRLIB	Forbid			; We must FORBID in here...
		lea.l	MemList(a6),a3		; Get MemList...
hack_mem:	move.l	(a3),a3			; Get next node...
		move.l	(a3),d0			; Check for done...
		beq.s	hack_mem_done		; If no more, we are done!
		move.l	a3,d0			; Get node address...
		bftst	d0{0:20}		; Is it in the lower 4K?
		bne.s	hack_mem		; If not, keep looking...
*
* Ah! so this header is in low mem!  Ok, we will move it...
*
		moveq.l	#MH_SIZE,d0		; Get size to allocate...
		moveq.l	#0,d1			; Any memory type...
		JSRLIB	AllocMem		; Allocate it...
		tst.l	d0			; It should always work, but...
		beq.s	hack_mem_done		; If this ever happens...
		move.l	d0,a0			; Store in address register
		movem.l	(a3),d0-d7		; Get the whole thing... (8*4)
		move.l	a0,a3			; Get new one...
		movem.l	d0-d7,(a3)		; Make new header a copy...
		move.l	(a0)+,a1		; Get LN_SUCC
		move.l	a3,LN_PRED(a1)		; Link in the bottom
		move.l	(a0)+,a1		; Get LN_PRED
		move.l	a3,LN_SUCC(a1)		; Link in the top...
		bra.s	hack_mem		; Loop back for more...
hack_mem_done:	JSRLIB	Permit
*
* Finally, if graphics is V39 or less, do the LastChanceMemory...
*
		lea	gfxName(pc),a1		; Get library name...
		moveq.l	#37,d0			; Minimum of V37
		JSRLIB	OpenLibrary		; Open the library...
		tst.l	d0			; Did it open?
		beq.s	no_gfx_hack		; If not, skip it...
		move.l	d0,a2			; Store in a2...
		move.w	LIB_VERSION(a2),d0	; Get version
		sub.w	#40,d0			; Are we over V39?
		bcc.s	close_gfx		; If so, skip hack...
		move.l	gb_LastChanceMemory(a2),a0	; Get semaphore...
		JSRLIB	ObtainSemaphore		; Get a lock on LCM...
		move.l	gb_LCMptr(a2),d0	; Get pointer...
		bftst	d0{0:20}		; Check if in the 4K
		bne.s	free_lcm		; If not, free the semaphore
		move.l	#4096,d0		; Size of LCM memory...
		move.l	#MEMF_CHIP,d1		; Type of memory...
		JSRLIB	AllocMem		; Allocate it...
		move.l	d0,d1			; Did the allocation work?
		beq.s	free_lcm		; If not, skip...
		move.l	d0,gb_LCMptr(a2)	; Store new LCM memory...
free_lcm:	move.l	gb_LastChanceMemory(a2),a0	; Get semaphore...
		JSRLIB	ReleaseSemaphore	; Release the lock...
close_gfx:	move.l	a2,a1			; Get GfxBase
		JSRLIB	CloseLibrary		; Close the library
no_gfx_hack:
*
* Now exit but first, reply the startup message as needed...
*
Exit:		move.l	ThisTask(a6),a4	; Get our process pointer...
		tst.l	pr_CLI(a4)	; Are we a CLI?
		bne.s	Do_Exit		; If not NULL, we are a CLI...
*
* Handle Workbench Startup...
*
		lea	pr_MsgPort(a4),a4	; Get message port...
		move.l	a4,a0		; We first wait for the WB Startup MSG
		JSRLIB	WaitPort	; (it should be here very quickly)
		move.l	a4,a0		; Now we get the message...
		JSRLIB	GetMsg		; Get it...
		move.l	d0,a1		; Get message pointer...
		JSRLIB	Forbid		; Forbid...  (No register trash)
		JSRLIB	ReplyMsg	; Reply the startup message...
*
Do_Exit:	moveq.l	#0,d0
		rts
*
		VERSTAG
gfxName:	dc.b	'graphics.library',0
*
		END

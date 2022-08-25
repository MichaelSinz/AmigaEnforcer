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
******* LawBreaker ************************************************************
*
*   NAME
*	LawBreaker - A quicky test of Enforcer
*
*   SYNOPSIS
*	This is a quick test of Enforcer and its reporting abilities.
*
*   FUNCTION
*	This program is used to make sure that Enforcer is correctly
*	installed and operating.  LawBreaker works from either the CLI
*	or Workbench.  It will try to read and write certain memory
*	areas that will cause an Enforcer hit or four.
*
*	LawBreaker will also do an Alert to show how Enforcer reports
*	an Alert.
*
*	Note that the LawBreaker executable has debugging information
*	in it (standard LINE format debug hunk) such that you can
*	try the FindHit program to find the line that causes the hit.
*
*   INPUTS
*	Just run it...
*
*   RESULTS
*	When running Enforcer, you will see some output from Enforcer.
*	Output on a 68030 machine would look something like this:
*
*25-Jul-93  17:15:04
*WORD-WRITE to  00000000        data=0000       PC: 0763857C
*USP:  07657C14 SR: 0004 SW: 04C1  (U0)(-)(-)  TCB: 07642F70
*Data: DDDD0000 DDDD1111 DDDD2222 DDDD3333 0763852A DDDD5555 DDDD6666 DDDD7777
*Addr: AAAA0000 AAAA1111 AAAA2222 AAAA3333 AAAA4444 0763852A 07400810 --------
*Stck: 00000000 0752EE9A 00002800 07643994 00000000 076786D8 000208B0 2EAC80EE
*Stck: 487AFD12 486C82C4 4EBA3D50 4EBAEA28 4FEF0014 52ACE2E4 204D43EC 88BC203C
*----> 0763857C - "lawbreaker"  Hunk 0000 Offset 00000074
*Name: "Shell"  CLI: "LawBreaker"  Hunk 0000 Offset 00000074
*
*25-Jul-93  17:15:04
*LONG-READ from AAAA4444                        PC: 07638580
*USP:  07657C14 SR: 0015 SW: 0501  (U0)(F)(-)  TCB: 07642F70
*Data: DDDD0000 DDDD1111 DDDD2222 DDDD3333 0763852A DDDD5555 DDDD6666 DDDD7777
*Addr: AAAA0000 AAAA1111 AAAA2222 AAAA3333 AAAA4444 0763852A 07400810 --------
*Stck: 00000000 0752EE9A 00002800 07643994 00000000 076786D8 000208B0 2EAC80EE
*Stck: 487AFD12 486C82C4 4EBA3D50 4EBAEA28 4FEF0014 52ACE2E4 204D43EC 88BC203C
*----> 07638580 - "lawbreaker"  Hunk 0000 Offset 00000078
*Name: "Shell"  CLI: "LawBreaker"  Hunk 0000 Offset 00000078
*
*25-Jul-93  17:15:04
*BYTE-WRITE to  00000101        data=11         PC: 0763858A
*USP:  07657C14 SR: 0010 SW: 04A1  (U0)(F)(D)  TCB: 07642F70
*Data: 00000000 DDDD1111 DDDD2222 DDDD3333 0763852A DDDD5555 DDDD6666 DDDD7777
*Addr: AAAA0000 AAAA1111 AAAA2222 AAAA3333 AAAA4444 0763852A 07400810 --------
*Stck: 00000000 0752EE9A 00002800 07643994 00000000 076786D8 000208B0 2EAC80EE
*Stck: 487AFD12 486C82C4 4EBA3D50 4EBAEA28 4FEF0014 52ACE2E4 204D43EC 88BC203C
*----> 0763858A - "lawbreaker"  Hunk 0000 Offset 00000082
*Name: "Shell"  CLI: "LawBreaker"  Hunk 0000 Offset 00000082
*
*25-Jul-93  17:15:04
*LONG-WRITE to  00000102        data=00000000   PC: 07638592
*USP:  07657C14 SR: 0014 SW: 0481  (U0)(-)(D)  TCB: 07642F70
*Data: 00000000 DDDD1111 DDDD2222 DDDD3333 0763852A DDDD5555 DDDD6666 DDDD7777
*Addr: AAAA0000 AAAA1111 AAAA2222 AAAA3333 AAAA4444 0763852A 07400810 --------
*Stck: 00000000 0752EE9A 00002800 07643994 00000000 076786D8 000208B0 2EAC80EE
*Stck: 487AFD12 486C82C4 4EBA3D50 4EBAEA28 4FEF0014 52ACE2E4 204D43EC 88BC203C
*----> 07638592 - "lawbreaker"  Hunk 0000 Offset 0000008A
*Name: "Shell"  CLI: "LawBreaker"  Hunk 0000 Offset 0000008A
*
*25-Jul-93  17:15:06
*Alert !! Alert 35000000     TCB: 07642F70     USP: 07657C10
*Data: 00000000 DDDD1111 DDDD2222 DDDD3333 0763852A DDDD5555 DDDD6666 35000000
*Addr: AAAA0000 AAAA1111 AAAA2222 AAAA3333 AAAA4444 0763852A 07400810 --------
*Stck: 076385A0 00000000 0752EE9A 00002800 07643994 00000000 0762F710 076305F0
*----> 076385A0 - "lawbreaker"  Hunk 0000 Offset 00000098
*
*	Now, using FindHit, you would type:
*
*	FindHit LawBreaker 0:82
*
*	and it will tell you the source file name and the line number
*	where the hit happened.  See the FindHit documentation.
*
*   NOTES
*	If enforcer is not running, the program should not cause the
*	system to crash.  It will, however, write to certain areas
*	of low memory.  Also, it will cause read access of some
*	addresses that may not exist.  This may cause bus faults.
*
*   SEE ALSO
*	"Quantum Physics:  The Dreams that Stuff is made of." - Michael Sinz
*
*   BUGS
*	There are 4 known Enforcer hits in this code and 1 alert, however,
*	they will not be fixed.  ;^)
*
*******************************************************************************
*
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/execbase.i"
		INCLUDE	"exec/macros.i"
		INCLUDE	"exec/alerts.i"
		INCLUDE	"dos/dosextens.i"
*
*******************************************************************************
*
* This turns on the HX68 (and CAPE) source line debugging information
* This is required for FindHit to work in assembly files...
*
		DEBUG
*
*******************************************************************************
*
* The main code...
*
LawBreaker:	move.l	4.w,a6		; Get ExecBase...
		clr.l	-(sp)		; Clear the message pointer...
		move.l	ThisTask(a6),a4	; Get our process pointer...
		tst.l	pr_CLI(a4)	; Are we a CLI?
		bne.s	Do_Law		; If NULL, we are a WB run program...
*
* Handle Workbench Startup...
*
		lea	pr_MsgPort(a4),a4	; Get message port...
		move.l	a4,a0		; We first wait for the WB Startup MSG
		JSRLIB	WaitPort	; (it should be here very quickly)
		move.l	a4,a0		; Now we get the message...
		JSRLIB	GetMsg		; Get it...
		move.l	d0,(sp)		; Now, store it on the stack...
*
* Ok, now be nasty...
*
* First fill up the registers with patterns
*
Do_Law:		move.l	#$AAAA0000,a0
		move.l	#$AAAA1111,a1
		move.l	#$AAAA2222,a2
		move.l	#$AAAA3333,a3
		move.l	#$AAAA4444,a4
		lea	Do_Law(pc),a5
		move.l	#$DDDD0000,d0
		move.l	#$DDDD1111,d1
		move.l	#$DDDD2222,d2
		move.l	#$DDDD3333,d3
		move.l	a5,d4
		move.l	#$DDDD5555,d5
		move.l	#$DDDD6666,d6
		move.l	#$DDDD7777,d7
*
		move.w	d0,$0000	; Ping location 0 (to 0)
		JSRLIB	Forbid
		move.l	(a4),d0		; Ping another spot
		JSRLIB	Disable
		move.b	d1,$0101	; Ping one more...
		JSRLIB	Permit
		move.l	d0,$0102	; One last one...
		JSRLIB	Enable
*
* Now do an Alert just to show what would happen:
*
		move.l	#AN_Unknown,d7	; An unknown alert...
		JSRLIB	Alert		; Display it...
*
* Ok, so now we quit...
*
		move.l	(sp)+,d0	; Get message...
		beq.s	Done		; If we have one, do WB exit...
*
* We have a Workbench message, so reply it...
*
		JSRLIB	Forbid		; We need to FORBID for a bit...
		move.l	d0,a1		; Get message into register
		JSRLIB	ReplyMsg	; Reply the workbench message...
*
Done:		rts			; Return with NULL...
*
*******************************************************************************
*
* The version string...
*
		INCLUDE	"lawbreaker_rev.i"
		VERSTAG
*
*******************************************************************************
*
* "A master's secrets are only as good as the
*  master's ability to explain them to others."  -  Michael Sinz
*
*******************************************************************************
*
		END

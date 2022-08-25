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
******* RebootOff *************************************************************
*
*   NAME
*	RebootOff - A keyboard reset handler to turn off Enforcer
*
*   SYNOPSIS
*	This is a simple utility that will turn off Enforcer when a
*	keyboard reset happens.
*
*   FUNCTION
*	This utility uses the feature of the A1000/A2000/A3000/A4000
*	Amiga systems to turn off Enforcer when the user does a
*	keyboard reset (ctrl-Amiga-Amiga).  This utility requires that
*	your Amiga supports (in hardware) the keyboard reset system.
*
*	The reason this was written was such that Enforcer could be
*	"quit" just before you reboot your Amiga 3000.  This way
*	it will let the kickstart not need to be reloaded and
*	thus let utilities such as RAD: work across reboots.  Note
*	that this does *not* help in the case where the Amiga reboots
*	under software conditions.  It is only for keyboard resets.
*
*   INPUTS
*	Just run it from either the CLI or Workbench.  It installs
*	a handler and exits.  On a keyboard reset, it will turn Enforcer
*	off before it lets the reset continue...  (max time of 10 seconds)
*
*   RESULTS
*	Installs a small reset handler object and task into the system.
*	About 3700 bytes needed the first time it is run.
*
*   NOTES
*	If Enforcer is not running, nothing will happen at Reset time.
*	If Enforcer can not quit, the reset system will continue to try
*	to quit Enforcer until the hardware timeout happens...
*
*   SEE ALSO
*	From the home of the imaginary deadlines:
*	"It will take 2i weeks to do that project." - Michael Sinz
*
*******************************************************************************

	include	"exec/types.i"
	include	"exec/memory.i"
	include	"exec/execbase.i"
	include	"devices/keyboard.i"
	include	"exec/libraries.i"
	include	"exec/interrupts.i"
	include	"exec/io.i"
	include	"exec/ables.i"
	include	"libraries/dosextens.i"
	include	"devices/input.i"

	XREF	_AbsExecBase

*******************************************************************************
*
* Some macros that make calling the system routines easier...
*
CALLSYS		MACRO
		IFND	_LVO\1
		xref	_LVO\1		; Set the external reference
		ENDC
		CALLLIB	_LVO\1		; Call the EXEC definied macro
		ENDM
*
*******************************************************************************
*
* Now, for the start of the code...
*
BootProtect:	move.l	_AbsExecBase,a6		; Get the EXEC library base
*
		clr.l	d7		; Clear d7...
*
		move.l	d7,a1		; Clear a1
		CALLSYS	FindTask	; Get our task pointer...
		move.l	d0,a4		; Now, move it to a1 for addressing use
		lea	pr_MsgPort(a4),a3
		tst.l	pr_CLI(a4)	; Check if this is a CLI task...
		bne.s	QuickCLI	; If so, skip the next section
*
*******************************************************************************
*
* This section deals with starting from the WorkBench...
* It is just here for completeness...
*
QuickWorkBench:	move.l	a3,a0		; Get message port
		CALLSYS	WaitPort	; Wait for the WorkBench message...
		move.l	a3,a0		; ...it's here, so let's get it
		CALLSYS	GetMsg		; off the message port...
		bra.s	QuickCont	; ...and go to the continue routine
*
*******************************************************************************
*
* The routine was started from the CLI  (prefered)
* Let's store a NULL pointer so that there is no WB message...
*
QuickCLI:	move.l	d7,d0		; No reply message...
*
*******************************************************************************
*
* Continue with the Quick initialization
*
QuickCont:
		move.l	d0,-(sp)	; Save the message pointer...
*
*******************************************************************************
*
* Check if we are running already...
*
		lea	taskName(PC),a1	; Get the special task name...
		CALLSYS	FindTask	; Look for it...
		tst.l	d0		; If it is there, we are installed...
		bne.s	abortInstall	; Abort if we are not to do this...
*
*******************************************************************************
*
* Now we allocate and copy the magic code...
*
		move.l	#CodeSize,d0		; Size of memory
		move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1	; Type of memory
		CALLSYS	AllocMem		; Get the memory
		move.l	d0,a5			; Save it...
		tst.l	d0			; Check it...
		beq.s	abortInstall		; No memory, no copy...
		move.l	d0,a1			; Destination...
		lea	StartOfCode(pc),a0	; Source
		move.l	#CodeCopySize-1,d6	; Copy from 0 to CodeCopySize-1
CopyLoop:	move.b	(a0)+,(a1)+		; Copy it...
		dbra	d6,CopyLoop		; All of it...
*
*******************************************************************************
*
* Now that it is copied, we need to create the task for it...
*
		lea	TaskStruct-StartOfCode(a5),a1	; Point at Task structure
		lea	TaskStackLower-StartOfCode(a5),a0
		move.l	a0,TC_SPLOWER(a1)
		lea	TaskStackUpper-StartOfCode(a5),a0
		move.l	a0,TC_SPUPPER(a1)
		move.l	a0,TC_SPREG(a1)
		move.b	#NT_TASK,LN_TYPE(a1)
		move.b	#50,LN_PRI(a1)		; We need a HIGH priority
		lea	taskNameOffset(a5),a0
		move.l	a0,LN_NAME(a1)		; Name of task...
		move.l	a5,a2			; Starting PC
		sub.l	a3,a3			; No final PC...
		CALLSYS	AddTask			; Add it to the list
*
*******************************************************************************
*
* Standard exit...
*
abortInstall:
		move.l	(sp)+,d0	; Get that message back
		beq.s	abortNoWB	; If none, we are done
		move.l	d0,a1		; Get the pointer ready
		FORBID			; manual says we must forbid...
		CALLSYS	ReplyMsg	; ...when returning WB message
abortNoWB:
		rts			; RTS out of this task...
*
*******************************************************************************

*******************************************************************************
*
* This is the start of the special REBOOT detection code
*
StartOfCode:	move.l	_AbsExecBase,a6		; Get EXEC Base
		lea	HandlerInfo(pc),a4	; Get my handler structure...
		lea	PortStruct(pc),a5	; Get my message port...
		lea	MP_MSGLIST(a5),a0	; Get message port list...
		NEWLIST	a0			; NewList the thing...
		lea	TaskStruct(pc),a0	; Get task address...
		move.l	a0,MP_SIGTASK(a5)	; Store the task
		move.b	#PA_SIGNAL,MP_FLAGS(a5)	; Type of message port...
		moveq.l	#-1,d0			; Any signal will do...
		CALLSYS	AllocSignal		; Get it...
		move.b	d0,MP_SIGBIT(a5)	; Store the signal...
		bmi	NoOpen			; If no signal, exit...
		moveq	#1,d1			; Signal 0...
		asl.l	d0,d1			; Make it signal n...
*
* Reset handler interrupt structure...
*
		move.l	d1,IS_DATA(a4)		; Set up the data...
		lea	ResetHandler(pc),a0	; Get code start...
		move.l	a0,IS_CODE(a4)		; And the code it runs...
*
* Now, we need to open the device...
*
		lea	InputBlock(pc),a2	; Get I/O block...
		move.b	#NT_MESSAGE,LN_TYPE(a2)	; initialize it...
		move.w	#IOSTD_SIZE,MN_LENGTH(a2)
		move.l	a5,MN_REPLYPORT(a2)	; Our reply port...
		lea	keyboardName(pc),a0	; Get input.device name
		moveq.l	#0,d0		; Clear d0
		move.l	d0,d1		;	and d1
		move.l	a2,a1
		CALLSYS	OpenDevice	; a1 is still set to the I/O block
		tst.l	d0		; Check...
		bne	NoOpen
*
*******************************************************************************
*
* Install the handler...
*
		move.l	a2,a1			; Get i/o block
		move.l	a4,IO_DATA(a1)		; Set the data address...
		move.w	#KBD_ADDRESETHANDLER,IO_COMMAND(a1)	; the ADD command...
		CALLSYS	DoIO			; All set, handler is running
*
*******************************************************************************
*
* Wait for the signal...
*
		move.l	IS_DATA(a4),d0		; Get signal bit...
		CALLSYS	Wait			; Wait for it...
*
*******************************************************************************
*
* We now need to do try to unload Enforcer...
*
		FORBID				; Protect all of this
killLoop:	lea	enforcerTask(pc),a1	; Get Enforcer task name
		CALLSYS	FindTask		; Check if it is here...
		tst.l	d0			; (NULL if not)
		beq.s	gone			; If not, it must be gone...
		move.l	d0,a1			; Task pointer...
		move.l	#SIGBREAKF_CTRL_C,d0	; Ctrl-C to quit Enforcer
		CALLSYS	Signal			; Signal it...
*
* Now, we wait a bit and check if it is gone...
* This does not have to be fast, so why not just open
* graphics.library here and do a few WaitTOF()s?
*
		lea	gfxName(pc),a1		; Get graphics name...
		moveq.l	#0,d0			; Any version
		CALLSYS	OpenLibrary		; Open it...
		tst.l	d0			; Did it open?
		beq.s	gone			; If not, we are done...
		move.l	a6,-(sp)		; Save
		move.l	d0,a6			; Get...
		CALLSYS	WaitTOF			; Wait a frame
		CALLSYS	WaitTOF			; ... and another
		move.l	a6,a1			; Get GfxBase back
		move.l	(sp)+,a6		; Restore
		CALLSYS	CloseLibrary		; Close it...
		bra.s	killLoop		; Loop back and check again
*
* Now, we get here if enforcer is gone or we can not open graphics...
* (Which means we get here when enforcer is gone :-)
*
gone:		CALLSYS	Permit			; No need for protection...
*
*******************************************************************************
*
* Now, tell keyboard.device that I am done...
*
		move.l	a2,a1		; Get I/O block...
		move.l	a4,IO_DATA(a1)
		move.w	#KBD_RESETHANDLERDONE,IO_COMMAND(a1)
		CALLSYS	DoIO		; Tell keyboard OK to reboot...
*
*******************************************************************************
*
* Uninstall the handler
*
CloseDown:
		move.l	a2,a1		; Get I/O block...
		move.l	a4,IO_DATA(a1)	; handler...
		move.w	#KBD_REMRESETHANDLER,IO_COMMAND(a1)
		CALLSYS	DoIO		; Remove the handler...
*
*******************************************************************************
*
* Close the device...
*
		move.l	a2,a1		; Get I/O block...
		CALLSYS	CloseDevice	; Close it...
*
*******************************************************************************
*
* Free memory and end task...
*
NoOpen:
		FORBID			; Forbid task switch...
		lea	StartOfCode(pc),a1
		move.l	#CodeSize,d0
		CALLSYS	FreeMem		; Free the memory
		rts			; Drop out of task...
*
*******************************************************************************
*
* The handler gets called here...
*
ResetHandler:	move.l	a1,d0			; Get signal to send
		lea	TaskStruct(pc),a1	; Get task
*
* Now signal the task...
*
		move.l	a6,-(sp)	; Save the stack...
		move.l	_AbsExecBase,a6	; Get ExecBase
		CALLSYS	Signal		; Send the signal
		move.l	(sp)+,a6	; Restore A6
*
* Return to let	other handlers execute.
*
		rts			; return from handler...
*
*******************************************************************************
*******************************************************************************
*
taskName:	dc.b	'Reset '
enforcerTask:	dc.b	'� Enforcer �',0
*
keyboardName:	dc.b	'keyboard.device',0
*
gfxName:	dc.b	'graphics.library',0
*
EndData:
CodeCopySize	EQU	EndData-StartOfCode
*
		CNOP	0,4
*
HandlerInfo:					; The handler structure...
InputBlock	EQU	HandlerInfo+IS_SIZE	; Input Block...
TaskStruct	EQU	InputBlock+IOSTD_SIZE	; Task structure...
PortStruct	EQU	TaskStruct+TC_SIZE	; Message port
TaskStackLower	EQU	PortStruct+MP_SIZE
TaskStackUpper	EQU	TaskStackLower+3000	; 3000 byte stack
EndOfCode	EQU	TaskStackUpper+8
CodeSize	EQU	EndOfCode-StartOfCode
HandlerOffset	EQU	HandlerInfo-StartOfCode
taskNameOffset	EQU	taskName-StartOfCode
*
*******************************************************************************
*
* And, with that we come to the end of another full-length feature staring
* that wonderful MC680x0 and the Amiga system.  Join us again next time
* for more Wonderful World of Amiga...  Until then, keep boinging...
*
		end

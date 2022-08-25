/*
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
*/

/*
******* Enforcer **************************************************************
*
*   NAME
*	Enforcer V37 - An advanced version of Enforcer - Requires V37
*
*   SYNOPSIS
*	Enforcer - A tool to watch for illegal memory accesses
*
*   FUNCTION
*	Enforcer will use the MMU in the advanced 680x0 processors
*	to set up MMU tables to watch for illegal accesses to memory
*	such as the low-page and non-existent pages.
*
*	To use, run Enforcer (plus any options you may wish)
*	If you wish to detach, just use RUN >NIL: <NIL: to start it.
*	You can also start it from the Workbench.  When started from Workbench,
*	Enforcer will read the tooltypes of its icon or selected project icon
*	for its options.  (See the sample project icons)
*
*	Enforcer should only be run *after* SetPatch.
*
*	If SegTracker is running in the system when Enforcer is started,
*	Enforcer will use the public SegTracker seglist tracking for
*	identifying the hits.
*
*   INPUTS
*	The options for Enforcer are as follows:
*
*	QUIET/S        - This tells Enforcer not to complain about any invalid
*	                 access and to just build MMU tables for cache setting
*	                 reasons -- mainly used in conjunction with an
*	                 Amiga BridgeBoard in a 68030 environment so that
*	                 the system can run with the data cache turned on.
*	                 In this case,
*	                                RUN >NIL: Enforcer QUIET
*	                 should be placed into the startup-sequence right
*	                 after SetPatch.
*
*	TINY/S         - This tells Enforcer to output a minimal hit.  The
*	                 output is basically the first line of the Enforcer
*	                 hit.  (see below)
*
*	SMALL/S        - This tells Enforcer to output the hit line, the
*	                 USP: line, and the Name: line.  (This means that
*	                 no register or stack display will be output)
*
*	SHOWPC/S       - This tells Enforcer to also output the two lines
*	                 that contain the memory area around the PC where
*	                 the hit happened.  Useful for disassembly.
*	                 This option will not do anything if QUIET, SMALL or
*	                 TINY output modes are selected.
*
*	STACKLINES/K/N - This lets you pick the number of lines of stack
*	                 backtrace to display.  The default is 2.  If set
*	                 to 0, no stack backtrace will be displayed.  There
*	                 is NO ENFORCED LIMIT on the number of lines.
*
*	STACKCHECK/S   - This option tells Enforcer that you wish all of
*	                 the long words displayed in the stack to be checked
*	                 against the global seglists via SegTracker.
*	                 This will tell you what seglist various return
*	                 addresses are on the stack.  If you are not
*	                 displaying stack information in the Enforcer hit
*	                 then STACKCHECK will have nothing to check.
*	                 If you are displaying stack information, then
*	                 each long word will be check and only those which
*	                 are in one of the tracked seglists will be
*	                 displayed in a SegTracker line.
*	                 The output will show the PC address first and
*	                 then work its way back on the stack such that you
*	                 can read it from bottom up as the order of calling
*	                 or from top down as the stack-frame backtrace.
*
*	AREGCHECK/S    - This option tells Enforcer that you wish all of
*	                 the values in the Address Registers checked via
*	                 SegTracker, much like STACKCHECK.
*
*	DREGCHECK/S    - This option tells Enforcer that you wish all of
*	                 the values in the Data Registers checked via
*	                 SegTracker, much like STACKCHECK.
*
*	DATESTAMP/S    - This makes Enforcer output a date and time with each
*	                 hit.  Due to the nature of the way Enforcer must
*	                 work, the time can not be read during the Enforcer
*	                 hit itself so the time output will be the last time
*	                 value the main Enforcer task set up.  Enforcer will
*	                 update this value every second as to try to not
*	                 use any real CPU time.  The time displayed in the
*	                 hit will thus be exact.
*	                 (Assuming the system clock is correct.)
*	                 The date is output before anything from the hit
*	                 other than the optional introduction string.
*
*	DEADLY/S       - This makes Enforcer a bit nasty.  Normally,
*	                 when an illegal read happens, Enforcer returns 0
*	                 as the result of this read.  With this option,
*	                 Enforcer will return $ABADFEED as the read data.
*	                 This option can make programs with Enforcer hits
*	                 cause even more hits.
*
*	FSPACE/S       - This option will make the special $00F00000 address
*	                 space available for writing.  This is useful for
*	                 those people with $00F00000 boards.  Mainly Commodore
*	                 internal development work -- should only be used
*	                 in that enviroment.
*
*	VERBOSE/S      - This option will make Enforcer display information
*	                 as to the mapping of the I/O boards and other
*	                 technical information.  This information maybe useful
*	                 in specialized debugging.
*
*	LED/K/N        - This option lets you specify the speed at which
*	                 the LED will be toggled for each Enforcer hit.
*	                 The default is 1 (which is like it always was)
*	                 Setting it to 0 will make Enforcer not touch
*	                 the LED.  Using a larger value will make the
*	                 flash take longer (such that it can be noticed
*	                 when doing I/O models other than the default
*	                 serial output)  The time that the flash will
*	                 take is a bit more than 1.3 microseconds times
*	                 the number.  So 1000 will be a bit more than
*	                 1.3 milliseconds.  (Or 1000000 is a bit more than
*	                 1.3 seconds.)
*
*	PARALLEL/S     - This option will make Enforcer use the parallel port
*	                 hardware rather than the serial port for output.
*
*	RAWIO/S        - This option will make Enforcer stuff the hit report
*	(special IO)     into an internal buffer and then from the main
*	                 Enforcer process output the results via the
*	                 RawPutChar() EXEC debugging LVO.  Since the output
*	                 happens on the Enforcer task it is possible for a
*	                 hit that ends in a system crash to not be able to
*	                 be reported.  This option is here such that tools
*	                 which can redirect debugging output can redirect
*	                 the Enforcer output too.
*
*	FILE/K         - This option will make Enforcer output the hit report
*	(special IO)     but to a file insted of sending it to the hardware
*	                 directly or using the RAWIO LVO.  A good example of
*	                 such a file is CON:0/0/640/100/HIT/AUTO/WAIT.
*	                 Another thing that can be done is to have a program
*	                 sit on a named pipe and have Enforcer output to it.
*	                 This program can then do whatever it feels like with
*	                 the Enforcer hits.  (Such as decode them, etc.)
*	                 *NOTE*  It is not a good idea to have Enforcer hits
*	                 go to a file on a disk as if the system crashes
*	                 during/after the Enforcer hit, the disk may
*	                 become corrupt.
*
*	STDIO/S        - This option will make Enforcer output the hit report
*	(special IO)     to STDOUT.  This option only works from the CLI as it
*	                 requires STDOUT.  It is best used with redirection or
*	                 pipes.
*
*	BUFFERSIZE/K/N - This lets you set Enforcer's internal output buffer
*	                 for the special I/O options.  This option is only
*	                 valid with the RAWIO, FILE, or STDIO options.
*	                 The minimum setting is 8000.  The default is 8000.
*	                 Having the right amount of buffer is rather
*	                 important for the special I/O modes.  The reason
*	                 is due to the fact that no operating system calls
*	                 can be made from a bus error.  Thus, in the
*	                 special I/O mode, Enforcer must store the output
*	                 in this buffer and, via some special magic,
*	                 wake up the Enforcer task to read the buffer and
*	                 write it out as needed.  However, if a task is
*	                 in Forbid() or Disable() when the Enforcer hit
*	                 happens, the Enforcer task will not be able to
*	                 output the results of the hit.  This buffer lets
*	                 a number of hits happen even if the Enforcer task
*	                 was unable to do the I/O.  If the number of
*	                 hits that happen before the I/O was able to
*	                 run gets too large, the last few hits will either
*	                 be cut off completely or contain only partial
*	                 information.
*
*	INTRO/K        - This optional introduction string will be output
*	                 at the start of every Enforcer hit.  For example:
*	                 INTRO="*NBad Program!"   The default is no string.
*
*	PRIORITY/K/N   - This lets you set Enforcer's I/O task priority.
*	                 The default for this priority is 99.  In some
*	                 special cases, you may wish to adjust this.
*	                 It is, however, recommended that if you are using
*	                 one of the special I/O options (RAWIO, FILE, or
*	                 STDIO) that you keep the priority rather high.
*	                 If the priority you supply is outside of the
*	                 valid task priority range (-127 to 127) Enforcer
*	                 will use the default priority.
*
*	NOALERTPATCH/S - This option disables the patching of the EXEC
*	                 Alert() function.  Normally Enforcer will patch
*	                 this function to provide information as to what
*	                 called Alert() and to prevent the Enforcer hits
*	                 that a call to Alert() would cause.
*
*	ON/S           - Mainly for completeness.  If not specified, it
*	                 is assumed you want to turn ON Enforcer.
*
*	QUIT=OFF/S     - Tells Enforcer to turn off.  Enforcer can also be
*	                 stopped by sending a CTRL-C to its process.
*
*   RESULTS
*	When run, a set of MMU tables that map addresses that are not
*	in the system's address map as invalid are installed.  Enforcer
*	will then trap invalid access attempts and generate a diagnostic
*	message as to what the illegal access was.  The first memory page
*	(the one starting at location 0) is also marked as invalid as many
*	programming errors cause invalid access to these addresses.  Invalid
*	addresses are completely off limits to applications.
*
*	When an	access violation happens, a report such as the following
*	is output.
*
*03-Apr-93  21:26:18
*WORD-WRITE to  00000000        data=4444       PC: 07895CA4
*USP:  078D692C SR: 0000 SW: 0729  (U0)(-)(-)  TCB: 078A2690
*Data: DDDD0000 DDDD1111 DDDD2222 DDDD3333 DDDD4444 DDDD5555 DDDD6666 DDDD7777
*Addr: AAAA0000 AAAA1111 AAAA2222 AAAA3333 AAAA4444 AAAA5555 07800804 --------
*Stck: 00000000 07848E1C 00009C40 078A30B4 BBBBBBBB BBBBBBBB BBBBBBBB BBBBBBBB
*Stck: BBBBBBBB BBBBBBBB BBBBBBBB BBBBBBBB BBBBBBBB 078E9048 00011DA8 DEADBEEF
*----> 07895CA4 - "lawbreaker"  Hunk 0000 Offset 0000007C
*PC-8: AAAA1111 247CAAAA 2222267C AAAA3333 287CAAAA 44442A7C AAAA5555 31C40000
*PC *: 522E0127 201433FC 400000DF F09A522E 012611C7 00CE4EAE FF7642B8 0324532E
*Name: "New_Shell"  CLI: "lawbreaker"  Hunk 0000 Offset 0000007C
*
*LONG-READ from AAAA4444                        PC: 07895CA8
*USP:  078D692C SR: 0015 SW: 0749  (U0)(F)(-)  TCB: 078A2690
*Data: DDDD0000 DDDD1111 DDDD2222 DDDD3333 DDDD4444 DDDD5555 DDDD6666 DDDD7777
*Addr: AAAA0000 AAAA1111 AAAA2222 AAAA3333 AAAA4444 AAAA5555 07800804 --------
*Stck: 00000000 07848E1C 00009C40 078A30B4 BBBBBBBB BBBBBBBB BBBBBBBB BBBBBBBB
*Stck: BBBBBBBB BBBBBBBB BBBBBBBB BBBBBBBB BBBBBBBB 078E9048 00011DA8 DEADBEEF
*----> 07895CA8 - "lawbreaker"  Hunk 0000 Offset 00000080
*PC-8: 247CAAAA 2222267C AAAA3333 287CAAAA 44442A7C AAAA5555 31C40000 522E0127
*PC *: 201433FC 400000DF F09A522E 012611C7 00CE4EAE FF7642B8 0324532E 01266C08
*Name: "New_Shell"  CLI: "lawbreaker"  Hunk 0000 Offset 00000080
*
*25-Jul-93  17:15:06
*Alert !! Alert 35000000     TCB: 07642F70     USP: 07657C10
*Data: 00000000 DDDD1111 DDDD2222 DDDD3333 0763852A DDDD5555 DDDD6666 35000000
*Addr: AAAA0000 AAAA1111 AAAA2222 AAAA3333 AAAA4444 0763852A 07400810 --------
*Stck: 076385A0 00000000 0752EE9A 00002800 07643994 00000000 0762F710 076305F0
*----> 076385A0 - "lawbreaker"  Hunk 0000 Offset 00000098
*
*	Here is a breakdown of what these reports are saying:
*
*	In the first report, the first line is the date stamp.
*
*	The first line of each report describes the access violation
*	and where it happened from.  In the case of a WRITE, the data
*	that was being written will be displayed as well.  If an instruction
*	mode access caused the fault, there will be an (INST) in the line.
*
*	The first line may also contain the BUS ERROR message.  This will
*	be displayed when an address that is valid in the system lists
*	causes a physical bus fault during the access.  This usually
*	will happen with plug-in cards or when a hardware problem causes
*	some form of system fault.  Watch out, if this does show up, your
*	system may be unstable and/or unreliable.
*
*	The second line (starts USP:) displays the USER stack pointer (USP),
*	the status register (SR:), the special status word (SW:).  It then
*	displays the supervisor/user state and the interrupt level.  This
*	will be from (U0) to (U7) or (S0) to (S7)  (S=Supervisor)  Next
*	is the forbid state (F=forbid, -=not) and the disable state (D or -)
*	of the task that was running when the access fault took place.
*	Finally, the task control block address is displayed (TCB:)
*
*	The next two lines contain the data and address register dumps from
*	when the access fault happened.  Note that A7 is not listed here.
*	It is the stack pointer and is listed as USP: in the line above.
*
*	Then come the lines of stack backtrace.  These lines show the
*	data on the stack.  If the stack is in invalid memory, Enforcer will
*	display a message to that fact.
*
*	If SegTracker was installed before Enforcer, the "---->" lines
*	will display in which seglist the given addresses are in based on the
*	global tracking that SegTracker does.  (See docs on SegTracker)
*	If no seglist match is found, no lines will be displayed.
*	One line will be displayed for each of the stack longwords asked
*	for (see the STACKCHECK option) and one line for the PC address of
*	the Enforcer hit.  (The PC line is always checked for is SegTracker
*	is installed.)  The lines are in order: hit, first stack find,
*	second stack find, etc.  This is useful for tracking down who
*	called the routine that caused the Enforcer hit.
*
*	Next, optionally, comes the data around the program counter when the
*	access fault happened.  The first line (PC-8:) is the 8 long-words
*	before the program counter.  The second line starts at the program
*	counter and goes for 8 long words.
*
*	The last line displays the name of the task that was running when
*	the access fault took place.  If the task was a CLI, it will display
*	the name of the CLI command that was running.  If the access fault
*	was found to have happened within the seglist of a loaded program,
*	the segment number and the offset from the start of the segment will
*	be displayed.  (Note that this works for any LoadSeg()'ed process)
*
*	Note that the name will display as "Processor Interrupt Level x"
*	if the access happened in an interrupt.
*
*	The other output that could happen is when a program or the OS
*	calls the EXEC Alert function.  Enforcer catches these calls
*	and will display the alert information as seen above.  (With the
*	date and time if needed)
*
*   WARNING
*	Enforcer is for software testing.  In this role it is vital.
*	Software that causes Enforcer hits may not be able to run on
*	newer hardware.  (Enforcer hits of high addresses on systems not
*	running Enforcer but with a 68040 will most likely crash the system)
*	Future systems and hardware will make this even more important.  The
*	system can NOT survive software that causes Enforcer hits.
*
*	However, Enforcer is NOT a system protector.  As a side effect, it
*	may well keep a system from crashing when Enforcer hits happen, but
*	it may just as well make the software crash earlier.  Enforcer is
*	mainly a development and testing tool.
*
*	Enforcer causes	no ill effects with correctly working software.
*	If a program fails to work while Enforcer is active, you should
*	contact the developer of that program.
*
*   NOTES
*	This is Enforcer V37.  Bryce Nesbitt came up with the original
*	"Enforcer" that has been instrumental to the improvement in the
*	quality of software on the Amiga.  The Amiga users and developers
*	owe him a great deal for this.  Thank you Bryce!  Enforcer V37,
*	however, is a greatly enhanced and more advanced tool.
*
*	Enforcer V37 came about due to a number of needs.  These included
*	the need for more output options and better performance.  It also
*	marks the removal of all kludges that were in the older versions.
*	Also, some future plans required some of these changes...
*
*	In addition, the complete redesign was needed in order to
*	support the 68040.  The internal design of Enforcer is now set up
*	such that CPU/MMU specific code can be cleanly accessed from the
*	general house keeping aspect of the code.  The MMU bus error
*	handling is, however, 100% CPU specific.
*
*	Since AbsExecBase is in low memory, reads of this address are slower
*	with Enforcer running.  Caching AbsExecBase locally is highly
*	recommended since it is in CHIP memory and on systems with FAST
*	memory, it will be faster to access the local cached value. (In
*	addition to the performance increase when running Enforcer) Note
*	that doing many reads of location 4 will hurt interrupt performance.
*
*	When the Amiga produces an ALERT, EXEC places some magic numbers
*	into some special locations in low memory.  The exact pattern
*	changes between versions of the operating system.
*
*	Enforcer will patch the EXEC function ColdReboot() in an attempt to
*	"get out of the way" when someone tries to reboot the system.
*	Enforcer will clean up as much as possible the MMU tables and then
*	call the original LVO.  When Enforcer is asked to quit, it will
*	check to make sure it can remove itself from this LVO. If it can
*	not, it will not quit at that time.  If run from the shell, it will
*	display a message saying that it tried but could not exit.  Enforcer
*	will continue to be active and you can try later to deactivate it.
*
*	Enforcer will also patch the EXEC function Alert() in an attempt to
*	provide better tracking of other events in the system.  It is also
*	patched such that dead-end alerts will correctly reset the system
*	and be displayed.  With this patch in place, the normal alerts will
*	not be seen but will be replaced by the Enforcer output shown
*	above.  See LawBreaker for a more complete example of this.
*
*   68020 NOTES
*	The 68020 does not have a built-in MMU but has a co-processor
*	feature that lets an external MMU be connected.  Enforcer MMU code
*	is designed for use with 68851 MMU.  This is the some-what 68030
*	compatible MMU by Motorola.  Enforcer uses the same code for both
*	the 68030 and the 68020/68851.  For this reason, 68020/68851 users
*	should see the 68030 NOTES section.
*
*   68030 NOTES
*	The 68030 uses cycle/instruction continuation and will
*	supply the data on reads and ignore writes during an access
*	fault rather than let the real bus cycle happen.  This means
*	that on a fault caused by MMU tables, no bus cycle to the
*	fault address will be generated.  (For those of you with analyzers)
*
*	In some cases, the 68030 will have advanced the Program Counter
*	past the instruction by the time the access fault happens.
*	This is usually only on WRITE faults.  For this reason, the PC
*	may either point at the instruction that caused the fault or
*	just after the instruction that caused the fault.  (Which could
*	mean that it is pointing to the middle of the instruction
*	that caused the fault.)
*
*	Note that there is a processor called 68EC030.  This processor
*	has a disabled or defective MMU.  However, it may function well
*	enough for Enforcer to think it has a fully functional MMU and
*	thus Enforcer will attempt to run.  However, even if it looks like
*	the MMU is functioning, it is not fully operational and thus may
*	cause strange system activity and even crashes.  Do not assume
*	that Enforcer is safe to use on 68EC030 systems.
*
*   68040 NOTES
*	Enforcer, on the 68040, *requires* that the 68040.library be
*	installed and it requires an MMU 68040 CPU.  The 68EC040 does not
*	have a MMU.  The 68LC040 does have an MMU and is supported. Enforcer
*	will work best in a system with the 68040.library 37.10 or better
*	but it does know how to deal with systems that do not have that
*	version.
*
*	Due to the design of the 68040, Enforcer is required to do a number
*	of things differently.  For example, the MMU page size can only be
*	either 8K or 4K.  This means that to protect the low 1K of memory,
*	Enforcer will end up having to mark the first 4K of memory as
*	invalid and emulate the access to the 3K of that memory that is
*	valid. For this reason Enforcer moves a number of possible
*	structures from the first 4K of memory to higher addresses.  This
*	means that the system will continue to run at a reasonable speed.
*	The first time Enforcer is run it may need to allocate memory for
*	these structures that it will move.  Enforcer can never return this
*	memory to the system.
*
*	In addition to the fact that the 68040 MMU table size is different,
*	the address fault handling is also different.  Namely, the 68040 can
*	only rerun the cycle and not continue it like the 68030. This means
*	that on a 68040, the page must be made available first and then made
*	unavailable.  To make this work, Enforcer will switch the instruction
*	that caused the error into trace mode and let it run with a special
*	MMU setup.  When the trace exception comes in, the MMU is set
*	back to the way it was.  Enforcer does its best to keep debuggers
*	working.  Note, however, that the interrupt level during a trace of
*	a READ will end up being set to 7.  This is to prevent interrupts
*	from changing the order of trace/MMU table execution.  The level
*	will be restored to the original state before continuing.  Since T0
*	mode tracing is also supported, there are also some changes in the
*	way it operates.  T0 mode tracing is defined, on the 68040, to cause
*	a trace whenever the instruction pipeline needed to be reloaded.
*	While on the 68020/030 processors this was normally only for the
*	branch instructions, in the 68040 this includes a large number of
*	other instructions.  (Including NOP!)  Anyway, if an Enforcer hit
*	happens while in T0 tracing mode, the trace will happen even on
*	instructions that normally would not cause a T0 mode trace.  Since
*	this may actually help in debugging and because it was not possible
*	to do anything else, this method of operation is deemed acceptable.
*
*	Another issue with the 68040 is that WRITE faults happen *after* the
*	instruction has executed.  (Except for MOVEM)  In fact, it is common
*	for the 68040 to execute one or more extra instructions before the
*	WRITE fault is executed.  This design makes the 68040 much faster,
*	but it also makes the Program Counter value that Enforcer can report
*	for the fault much less likely to be pointing to the instruction
*	that caused it.  The worst cases are sequences such as a write fault
*	followed by a branch instruction.  In these cases, the branch is
*	usually already executed before the write fault happens and thus the
*	PC will be pointing to the target of the branch.  There is nothing
*	that can be done within Enforcer to help out here.  You will just
*	need to be aware of this and deal with it as best as possible.
*
*	Along with the above issue, is the fact that since a write fault may
*	be delayed, a read fault may happen before the write fault shows up.
*	Internally, enforcer does not do special processing for these and
*	they will not show up.  Since another hit was happening anyway, it
*	is felt that it is best to just not report the hit.  Along the same
*	lines, the hit generated from a MOVEM instruction may only show as a
*	single hit rather than 1 for each register moved.
*
*	On the Amiga, MOVE16 is not supported 100%.  Causing an Enforcer hit
*	with a MOVE16 will cause major problems and maybe cause Enforcer or
*	your task to lock.  Since MOVE16 is not supported, this is not a
*	major issue.  Just watch out if you are using this 68040
*	instruction.  (Also, watch out for the 68040 CPU bug with MOVE16)
*
*	The functions CachePreDMA(), CachePostDMA(), and CacheControl() are
*	patched when the 68040 MMU is turned on by Enforcer.  These
*	functions are patched such the issues with DMA and the 68040
*	COPYBACK data caches are addressed.  The 68040.library normally
*	deals with this, however since Enforcer turns on the MMU, the method
*	of dealing with it in the 68040.library will not work. For this
*	reason, Enforcer will patch these and implement the required fix for
*	when the MMU is on.  When Enforcer is asked to exit, it will check
*	if it can remove itself from these functions.  If it can not, it
*	will ignore the request to exit.  If Enforcer was run from the CLI,
*	it will print a message saying that it can not exit when the attempt
*	is made.
*
*   68060 NOTES
*	Enforcer, on the 68060, *requires* that the 68060.library be
*	installed.  Due to the fact that various possible 68060.library
*	versions may exist, Enforcer tries to not second guess it.
*	Thus, Enforcer assumes that the 68060.library has all of the
*	same functionality as V37.30 or better of the 68040.library.
*
*	It turns out that some of the 68060 libraries do not have the
*	same functionality of the 68040.library.  One common library
*	has elected not to handle Pre/Post DMA MMU table operations
*	when Enforcer installs its MMU table.  This results in some
*	DMA/Cache interactions.  Enforcer can not work around this
*	problem safely.  If you happen to have a 68060.library with
*	version 2.1 (19.07.96) you may be able to patch it to not
*	have this problem.  At offset $09BE there should be the
*	4-byte sequence $20 6D 00 04  Changing this to $4E 7A 88 06
*	will let it handle Enforcer's MMU tables too.  (The same
*	patch may work in other versions of the library)
*
*	For implementers of 68060.library, see my notes as to what
*	had to be done in 68040.library for correct operation.
*	Note that this does not mean that Enforcer needs this.  The Amiga
*	system needs this to operate correctly.  Enforcer just may
*	cause these problems to become more evident.  The notes are only
*	in the AmigaGuide version of the Enforcer documentation.
*
*	The 68060 exception model is full-restart, which means that
*	all instructions are re-run.  Both reads *and* writes.
*	This means that Enforcer can not tell you what the data
*	that would be written is, unlike the 68040 and earlier CPUs.
*	So, the output for a write will not include the data that
*	was to be written.  This does mean that faults happen
*	before the instruction is executed (usually) and thus the
*	reported PC will be more exact.  This restart model also
*	means that if an real bus-fault happens, Enforcer will be
*	unable to do much other than let it happen.  (The same is
*	true for reads on the 68040)  Enforcer maps all addresses
*	as either valid based on system configuration or invalid.
*	This is so that no address should cause a bus fault unless
*	the system configuration is incorrect and an address that
*	was marked valid actually causes a fault.
*
*	Be sure to read the 68040 notes as the 68060 is a superset
*	of much of these notes.
*
*	Due to the complexity of emulating access to lower memory and
*	the fact that the 68060 was introduced well after V39 kickstart,
*	it is highly recommended that you use V39 or better with 68060 CPUs.
*	This mainly has to deal with lower 4K of memory.  As of V38 of
*	exec.library, 68040/68060 processors would map out the lower 4K
*	of RAM rather than just 1K.  This was required since the newer
*	CPUs did not have page sizes less than 4K.
*
*	It turns out that some 68060 CPU cards also have other hardware
*	on them.  This is not a problem, unless this hardware does not
*	autoconfigure.  Enforcer needs to know about hardware in the
*	system so it can map the MMU to that hardware.  If the hardware
*	is not true Amiga AutoConfig (as in no expansion.library entry)
*	then Enforcer has no way of knowing it exists.
*
*	A common location for such hardware control registers to be
*	placed is in the reserved $00F00000 address range (known as
*	F-Space).  This 512K space was reserved for future Kickstart
*	growth.  It also has some magic in it so that you can wedge
*	special startup routines there for things like 68060 cards.
*	At least one vendor is doing all of this correctly and
*	has even made sure that expansion.library knows about the
*	hardware that is located in the $00F00000 address range.
*
*	If, when running Enforcer, your machine does lock up *and*
*	when you run Enforcer with the VERBOSE option it does not
*	say that the $00F00000 address range is a "board address"
*	then you may wish to try the FSPACE option to see if this
*	is the reason.  If this does not fix the problem, you more
*	than likely have either a real bug or some other non-AutoConfig
*	hardware in your system.
*
*   WRITING DEBUGGERS
*	If you wish to make a debugger that works with Enforcer to help
*	pinpoint Enforcer hits in the application and not cause Enforcer
*	hits itself, here are some simple tips and a bit of code.
*
*   DEBUGGERS:  TRAPPING A HIT
*	To trap a hit requires a number of things to work.
*
*	First, the debugger itself must never cause an Enforcer hit.
*	For help on that, see the "DEBUGGERS: NOT CAUSING A HIT"
*
*	Second, the debugger must be global.  That is, you must be
*	able to deal with a task getting a hit that is not the task
*	under test.  There are a number of simple ways to deal with
*	this, and I will leave this up to the debugger writer.
*	(One method will be shown below)
*
*	Third, the debugger must start *AFTER* Enforcer starts.
*	If it is started before Enforcer, the hits will not be
*	trapped.  (Note that this is not a problem)
*
*	A very important point:  The code needs to be fast for
*	the special case of location 4.  This is shown in the
*	code below.  It is very important that this be fast.
*
*	Note that it is much prefered that debuggers use the
*	method described below for trapping hits.  It should
*	be much more supportable this way as any of the tricky
*	work that may need to be done in the hit processing
*	will be handled by Enforcer itself.  If you wish the
*	hit decoded, you can capture the Enforcer output via a
*	pipe or some other method (such as RAWIO) or you can
*	leave that issue up to the user.
*
*	Now, given the above, the following bits of code can be
*	used to get the debugger to switch into single-step mode
*	at the point of the Enforcer hit.  You can also set some
*	data value here to tell your debugger about this.
*
*	;
*	; The following code is inserted into the bus error vector.
*	; Make sure you follow the VBR to find the vector.
*	; Store the old vector in the address OldVector
*	; Make sure you already have the single-step trap vector
*	; installed before you install this.  Note that any extra
*	; code you add in the comment area *MUST NOT* cause a bus
*	; fault of any kind, including reading of location 4.
*	;
*	; This is the common part...
*	;
*	EnforcerHit:    ds.l    1                       ; Some private flag
*	MyTask:         ds.l    1                       ; Task under test
*	MyExecBase:     ds.l    1                       ; The local copy
*	OldVector:      ds.l    1                       ; One long word
*	                ;
*	                ; Now, if you wish to only trap a specific task,
*	                ; do the check at this point.  For example, a
*	                ; simple single-task debugger would do something
*	                ; like this:
*	Common:         move.l  a0,-(sp)                ; Save this...
*	                move.l  MyExecBase(pc),a0       ; Get ExecBase...
*	                move.l  ThisTask(a0),a0         ; Get ThisTask
*	                cmp.l   MyTask(pc),a0           ; Are they the same?
*	                move.l  (sp)+,a0                ; Restore A0 (no flags)
*	                bne.s   TraceSkip               ; If not my task, skip
*	                ;
*	                bset.b  #7,(sp)                 ; Set trace bit...
*	                ; If you have any other data to set, do it now...
*	                ; Set as setting the EnforcerHit bit in your data...
*	                addq.l	#1,EnforcerHit          : Count the hit...
*	                ;
*	TraceSkip:      move.l  OldVector(pc),-(sp)     ; Ready to return
*	                rts
*	;
*	; This is the 68020/68030 version...
*	;
*	NewVector030:   cmp.l   #4,$10(sp)              ; 68020 and 68030
*	                beq.s   TraceSkip               ; If AbsExecBase, OK
*	                bra.s	Common			; Do the common stuff
*	;
*	; This is the 68040 version...
*	;
*	NewVector040:   cmp.l   #4,$14(sp)              ; 68040
*	                beq.s   TraceSkip               ; If AbsExecBase, OK
*	                bra.s	Common			; Do the common stuff
*	;
*	; This is the 68060 version...
*	;
*	NewVector060:   cmp.l   #4,$08(sp)              ; 68060
*	                beq.s   TraceSkip               ; If AbsExecBase, OK
*	                bra.s	Common			; Do the common stuff
*
*
*   DEBUGGERS:  NOT CAUSING A HIT
*	In order not to cause Enforcer hits, you can do a number
*	of things.  The easiest is to test the address with the TypeOfMem()
*	EXEC function.  If TypeOfMem() returns 0, the address is not
*	in the memory lists.  However, this does not mean it is not a
*	valid address in all cases.  (ROM, chip registers, I/O boards)
*	For those cases, you can build a "valid memory access table"
*	much like Enforcer does.  Here is the code from Enforcer for
*	the base memory tables:
*
*	\*
*	 * Mark_Address(mmu,start address,length,type)
*	 *\
*
*	\*
*	 * Special case the first page of CHIP RAM
*	 *\
*	mmu=Mark_Address(mmu,0,0x1000,INVALID | NONCACHEABLE);
*
*	\*
*	 * Map in the free memory
*	 *\
*	Forbid();
*	mem=(struct MemHeader *)SysBase->MemList.lh_Head;
*	while (mem->mh_Node.ln_Succ)
*	{
*	  mmu=Mark_Address(mmu,
*	                   (ULONG)(mem->mh_Lower),
*	                   (ULONG)(mem->mh_Upper)-(ULONG)(mem->mh_Lower),
*	                   ((MEMF_CHIP & TypeOfMem(mem->mh_Lower)) ?
*	                     (NONCACHEABLE | VALID) : (CACHEABLE | VALID)));
*	  mem=(struct MemHeader *)(mem->mh_Node.ln_Succ);
*	}
*	Permit();
*
*	\*
*	 * Map in the autoconfig boards
*	 *\
*	if (ExpansionBase=OpenLibrary("expansion.library",0))
*	{
*	struct	ConfigDev	*cd=NULL;
*
*	  while (cd=FindConfigDev(cd,-1L,-1L))
*	  {
*	    \* Skip memory boards... *\
*	    if (!(cd->cd_Rom.er_Type & ERTF_MEMLIST))
*	    {
*	      mmu=Mark_Address(mmu,
*	                       (ULONG)(cd->cd_BoardAddr),
*	                       cd->cd_BoardSize,
*	                       VALID | NONCACHEABLE);
*	    }
*	  }
*	  CloseLibrary(ExpansionBase);
*	}
*
*	\*
*	 * Now for the control areas...
*	 *\
*	mmu=Mark_Address(mmu,0x00BC0000,0x00040000,VALID | NONCACHEABLE);
*	mmu=Mark_Address(mmu,0x00D80000,0x00080000,VALID | NONCACHEABLE);
*
*	\*
*	 * and the ROM...
*	 *\
*	mmu=Mark_Address(mmu,
*	                 0x00F80000,
*	                 0x00080000,
*	                 VALID | CACHEABLE | WRITEPROTECT);
*
*	\*
*	 * If the credit card resource, make the addresses valid...
*	 *\
*	if (OpenResource("card.resource"))
*	{
*	  mmu=Mark_Address(mmu,0x00600000,0x00440002,VALID | NONCACHEABLE);
*	}
*
*	\*
*	 * If CD-based Amiga (CDTV, A570, etc.)
*	 *\
*	if (FindResident("cdstrap"))
*	{
*	  mmu=Mark_Address(mmu,0x00E00000,0x00080000,VALID | NONCACHEABLE);
*	  mmu=Mark_Address(mmu,0x00B80000,0x00040000,VALID | NONCACHEABLE);
*	}
*
*	\*
*	 * Check for ReKick/ZKick/KickIt
*	 *\
*	if ((((ULONG)(SysBase->LibNode.lib_Node.ln_Name)) >> 16) == 0x20)
*	{
*	  mmu=Mark_Address(mmu,
*	                   0x00200000,
*	                   0x00080000,
*	                   VALID | CACHEABLE | WRITEPROTECT);
*	}
*
*   SEE ALSO
*	"A master's secrets are only as good as the
*	 master's ability to explain them to others."  -  Michael Sinz
*
*   BUGS
*	None?
*
*******************************************************************************
*/

#include	<exec/types.h>
#include	<exec/execbase.h>
#include	<exec/tasks.h>
#include	<exec/memory.h>
#include	<exec/alerts.h>
#include        <exec/ports.h>
#include        <exec/libraries.h>
#include	<exec/semaphores.h>
#include        <dos/dos.h>
#include        <dos/dosextens.h>
#include	<dos/rdargs.h>
#include	<devices/timer.h>
#include	<workbench/startup.h>
#include	<workbench/workbench.h>
#include	<libraries/configvars.h>

#include	<clib/exec_protos.h>
#include	<pragmas/exec_pragmas.h>

#include	<clib/dos_protos.h>
#include	<pragmas/dos_pragmas.h>

#include	<clib/icon_protos.h>
#include	<pragmas/icon_pragmas.h>

#include	<clib/expansion_protos.h>
#include	<pragmas/expansion_pragmas.h>

#include	<string.h>

/*
 * A private LVO that I will use called RawPutChar...
 * I need to do the prototype myself...
 */
VOID RawPutChar(UBYTE);
#pragma libcall SysBase RawPutChar 204 001

#define EXECBASE (*(struct ExecBase **)4)

/*
 * Minimum buffer size of the output buffer when using one
 * of the special I/O modes.
 */
#define	MIN_BUFFER	(8000)

/*
 * The MMU Frame we will use...  (Note - must match what is in handler.asm)
 */
struct MMU_Frame
{
	ULONG	mmu_Flag;	/* The mmu type flag */

	ULONG	mmu_CRP[2];	/* CRP for 030 is 2 longs...  For the 040 it is the root pointer and only 1 long used */
	ULONG	mmu_TC;		/* TC for both 030 and 040.  Accessed as a long, but only part of it is used on 040 */

	ULONG	mmu_CRP_OLD[2];
	ULONG	mmu_TC_OLD;

	ULONG	*mmu_LevelA;	/* This is the base table.  The root pointer will contain this one... */
	ULONG	*mmu_LevelB;	/* Note that on the 68040/68060, these are just the invalid pages and the full pages are elsewhere */
	ULONG	*mmu_LevelC;	/* " */

	ULONG	mmu_Indirect;	/* The indirect MMU error descriptor ;^)   For the 68040/68060 slimmy trick... */
	ULONG	*mmu_Magic;	/* Special memory area for magic code (such as the 68040/68060 bus error remapping magic) */

	ULONG	*mmu_Page0;	/* For 68040/68060 page 0 work */

	ULONG	mmu_InvalidA;	/* The invalid value at LevelA  (68040/68060 only) */
	ULONG	mmu_InvalidB;	/* The invalid value at LevelB  (68040/68060 only) */
	ULONG	mmu_InvalidC;	/* The invalid value at LevelC  (68040/68060 only) */

	ULONG	mmu_TT[4];	/* Storage for TT regs (68040/68060 only) */
};

/* Values for MMU_Frame.mmu_Flag */
#define	MMU_030		1	/* MMU setup 68030 or 68020+68851 */
#define	MMU_040		2	/* MMU setup 68040 */
#define	MMU_060		3	/* MMU setup 68060 */

/*
 * General flags for setting up the MMU
 */
#define	VALID		(0x01)
#define	NONCACHEABLE	(0x40)
#define	CACHEABLE	(0x00)
#define	INVALID		(0x00)
#define	WRITEPROTECT	(0x04)

/*
 * This is for the 68060 marking of writethrough because
 * of the various 68060.library implementations.  (Enforcer
 * itself does not need it)
 * *DO NOT USE THIS IN NON-68060 MMUs*
 */
#define	MMU_060_TYPE	(0x81)

/*
 * Number of copies of memory headers
 * (Most systems have only 1 or 2 or maybe 3 so 400
 * should cover more than any system for a long time!
 */
#define	NUM_COPIES	(400)

/******************************************************************************/

/* External data from assembly */
extern	char	Copyright[];
extern	char	MyTask[];
extern	char	DateStr[];
extern	char	TimeStr[];
extern	ULONG	ROM_Addr;	/* The physical address of the ROM */
extern	ULONG	Bad_ReadValue;
extern	ULONG	Quiet_Flag;
extern	ULONG	ShowPC_Flag;
extern	ULONG	Small_Flag;
extern	ULONG	Tiny_Flag;
extern	ULONG	Parallel_Flag;
extern	ULONG	LED_Count;
extern	ULONG	StackLines;
extern	ULONG	SegLines;
extern	ULONG	ARegCheck;
extern	ULONG	DRegCheck;
extern	ULONG	AlertOFF;

extern	UBYTE	*Intro;
extern	UBYTE	*OutputBuffer;
extern	ULONG	DoDateStamp;
extern	ULONG	WriteOffset;
extern	ULONG	ReadOffset;
extern	ULONG	BufferSize;

extern	struct	ExecBase	*SysBase;
extern	struct	Library		*DOSBase;
extern	struct	Library		*Lib68040;
extern	struct	Library		*Lib68060;

extern	ULONG	SegTracker;

/* External functions from assembly... */
ULONG __asm Test_MMU(void);
BOOL __asm MMU_On(register __a0 struct MMU_Frame *);
BOOL __asm MMU_Off(register __a0 struct MMU_Frame *);

/******************************************************************************/

/* Internal functions... */
static void Free_MMU_Frame(struct MMU_Frame *mmu);
static struct MMU_Frame *Mark_Address(struct MMU_Frame *mmu,ULONG addr,ULONG size,ULONG orBits);
static struct MMU_Frame *Map_ROM(struct MMU_Frame *mmu,ULONG addr);
static struct MMU_Frame *Mark_Invalid(struct MMU_Frame *mmu);
static struct MMU_Frame *Init_MMU_Frame(ULONG MMU_Flag);

/******************************************************************************/

#define TEMPLATE  "QUIET/S,TINY/S,SMALL/S,SHOWPC/S,STACKLINES/K/N,STACKCHECK/S,AREGCHECK/S,DREGCHECK/S,DATESTAMP/S,DEADLY/S,FSPACE/S,VERBOSE/S,LED/K/N,PARALLEL/S,RAWIO/S,FILE/K,STDIO/S,BUFFERSIZE/K/N,INTRO/K,PRIORITY/K/N,NOALERTPATCH/S,ON/S,QUIT=OFF/S"

#define	OPT_QUIET	0
#define	OPT_TINY	1
#define	OPT_SMALL	2
#define	OPT_SHOWPC	3
#define	OPT_STACKLINES	4
#define	OPT_STACKCHECK	5
#define	OPT_AREGCHECK	6
#define	OPT_DREGCHECK	7
#define	OPT_DATESTAMP	8
#define	OPT_DEADLY	9
#define	OPT_FSPACE	10
#define	OPT_VERBOSE	11
#define	OPT_LED		12
#define	OPT_PARALLEL	13
#define	OPT_RAWIO	14
#define	OPT_FILE	15
#define	OPT_STDIO	16
#define	OPT_BUFFERSIZE	17
#define	OPT_INTRO	18
#define	OPT_PRIORITY	19
#define	OPT_ALERTPATCH	20
#define	OPT_ON		21
#define	OPT_QUIT	22
#define	OPT_COUNT	23

ULONG cmd(void)
{
struct	Library		*ExpansionBase;
struct	Process		*proc;
	ULONG		temp1;
	BPTR		OutputFile=NULL;
struct	RDArgs		*rdargs=NULL;
struct	WBStartup	*msg=NULL;
struct	RDArgs		*myRDArgs=NULL;
	ULONG		rc=RETURN_FAIL;
struct	MMU_Frame	*mmu;
struct	MsgPort		*timerPort=NULL;
struct	timerequest	*timerIO=NULL;
	ULONG		timerOpen=FALSE;
	LONG		opts[OPT_COUNT];

	SysBase = EXECBASE;

	proc=(struct Process *)FindTask(NULL);

	if (!(proc->pr_CLI))
	{
		WaitPort(&(proc->pr_MsgPort));
		msg=(struct WBStartup *)GetMsg(&(proc->pr_MsgPort));
	}

	if (DOSBase = OpenLibrary("dos.library",37))
	{
	char	*oldName=proc->pr_Task.tc_Node.ln_Name;

		memset((char *)opts, 0, sizeof(opts));

		/* Open 68060... If it fails, try 68040... */
		Lib68060=OpenLibrary("68060.library",0);
		Lib68040=(Lib68060 ? NULL : OpenLibrary("68040.library",0));

		/*
		 * Do the option parsing
		 * If from Workbench, use icon tool types
		 * If from CLI, use ReadArgs
		 */
		if (msg)
		{
		struct	Library	*IconBase;
		struct	WBArg	*wbArg;

			/*
			 * Started from Workbench so do icon magic...
			 *
			 * What we will do here is try all of the tooltypes
			 * in the icon and keep only those which do not cause
			 * errors in the RDArgs.
			 */
			if (wbArg=msg->sm_ArgList) if (IconBase=OpenLibrary("icon.library",0))
			{
			struct	DiskObject	*diskObj;
				BPTR		tmplock;

				/*
				 * Use project icon if it is there...
				 */
				if (msg->sm_NumArgs > 1) wbArg++;

				tmplock=CurrentDir(wbArg->wa_Lock);
				if (diskObj=GetDiskObject(wbArg->wa_Name))
				{
				char	**ToolTypes;

					if (ToolTypes=diskObj->do_ToolTypes)
					{
					char	*TotalString;
					ULONG	totalSize=3;

						while (*ToolTypes)
						{
							totalSize+=strlen(*ToolTypes)+1;
							ToolTypes++;
						}

						if (TotalString=AllocVec(totalSize,MEMF_PUBLIC))
						{
						char	*CurrentPos=TotalString;
						ULONG	currentLength;

							ToolTypes=diskObj->do_ToolTypes;
							do
							{
								*CurrentPos='\0';
								if (*ToolTypes) strcpy(CurrentPos,*ToolTypes);
								currentLength=strlen(CurrentPos);
								CurrentPos[currentLength+0]='\n';
								CurrentPos[currentLength+1]='\0';

								if (rdargs) FreeArgs(rdargs);
								rdargs=NULL;
								memset((char *)opts, 0, sizeof(opts));

								if (myRDArgs) FreeDosObject(DOS_RDARGS,myRDArgs);
								if (myRDArgs=AllocDosObject(DOS_RDARGS,NULL))
								{
									myRDArgs->RDA_Source.CS_Buffer=TotalString;
									myRDArgs->RDA_Source.CS_Length=strlen(TotalString);

									if (rdargs=ReadArgs(TEMPLATE, opts, myRDArgs))
									{
										CurrentPos[currentLength]=' ';
										CurrentPos+=currentLength+1;
									}
								}
							} while (*ToolTypes++);
							FreeVec(TotalString);
						}
					}

					FreeDiskObject(diskObj);
				}

				CurrentDir(tmplock);
				CloseLibrary(IconBase);
			}
			rc=RETURN_OK;
		}
		else
		{
			/*
			 * Started from CLI so do standard ReadArgs
			 */
			PutStr(Copyright);
			if (!(rdargs = ReadArgs(TEMPLATE, opts, NULL))) PrintFault(IoErr(),NULL);
			else if (CheckSignal(SIGBREAKF_CTRL_C)) PrintFault(ERROR_BREAK,NULL);
			else rc=RETURN_OK;
		}

		/*
		 * Check if we are running already...
		 */
		if (RETURN_OK==rc)
		{
		struct	Task	*tsk;

			Forbid();
			if (!(tsk=FindTask(MyTask)))
			{
				proc->pr_Task.tc_Node.ln_Name=MyTask;
				if (opts[OPT_QUIT]) if (!msg) PutStr("Enforcer is not running.\n");
			}
			else if (opts[OPT_QUIT])
			{
				Signal(tsk,SIGBREAKF_CTRL_C);
				if (!msg) PutStr("Signalled Enforcer to unload.\n");
			}
			else
			{
				if (!msg) PutStr("Enforcer already running.\n");
				rc=RETURN_FAIL;
			}
			Permit();
		}


		/*
		 * If all is OK, start up the system as needed...
		 */
		if ((RETURN_OK==rc) && (!opts[OPT_QUIT]))
		{
			/* Check for I/O options */
			if (opts[OPT_FILE])
			{
				if (!(OutputFile=Open((char *)opts[OPT_FILE],MODE_NEWFILE)))
				{
					rc=RETURN_FAIL;
					if (!msg) PutStr("Could not open output file.\n");
				}
			}

			if (RETURN_OK==rc)
			{
				if (opts[OPT_BUFFERSIZE]) BufferSize=(*(ULONG *)opts[OPT_BUFFERSIZE]);

				if ((opts[OPT_RAWIO]) || opts[OPT_STDIO] || (OutputFile))
				{
					if (BufferSize<MIN_BUFFER) BufferSize=MIN_BUFFER;
					if (!(OutputBuffer=AllocVec(BufferSize,MEMF_PUBLIC)))
					{
						rc=RETURN_FAIL;
						if (!msg) PutStr("Could not allocate I/O buffer.\n");
					}
				}
				else if (BufferSize)
				{
					rc=RETURN_FAIL;
					if (!msg) PutStr("BufferSize option invalid without an I/O option.\n");
				}
			}
		}

		/*
		 * If all is OK and we are asking for DateStamps...
		 */
		if ((RETURN_OK==rc) && (!opts[OPT_QUIT]) && (opts[OPT_DATESTAMP]))
		{
			rc=RETURN_FAIL;

			if (timerPort=CreateMsgPort())
			{
				if (timerIO=(struct timerequest *)CreateIORequest(timerPort,sizeof(struct timerequest)))
				{
					if (!OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)timerIO,0L))
					{
						rc=RETURN_OK;
						timerOpen=TRUE;
						timerIO->tr_time.tv_secs=0;
						timerIO->tr_time.tv_micro=1;
						timerIO->tr_node.io_Command=TR_ADDREQUEST;
						SendIO((struct IORequest *)timerIO);
					}
				}
			}

			if (RETURN_FAIL==rc) if (!msg) PutStr("Could not initialize timer.\n");
		}

		/*
		 * If all is OK, start up the system as needed...
		 */
		if ((RETURN_OK==rc) && (!opts[OPT_QUIT]))
		{
		struct	SignalSemaphore	*sem;
			ULONG		MMU_Flag;
			ULONG		Verbose=FALSE;

			if (sem=FindSemaphore("SegTracker"))
			{
				sem++;
				SegTracker=*(ULONG *)sem;
			}

			/*
			 * If enforcer is to be deadly, we use this
			 * value as the return from bad READ requests
			 */
			if (opts[OPT_DEADLY]) Bad_ReadValue=0xABADFEED;

			/* Various flags... */
			Quiet_Flag=opts[OPT_QUIET];
			Tiny_Flag=opts[OPT_TINY];
			Small_Flag=opts[OPT_SMALL];
			ShowPC_Flag=opts[OPT_SHOWPC];
			Parallel_Flag=opts[OPT_PARALLEL];
			Intro=(UBYTE *)opts[OPT_INTRO];
			DoDateStamp=opts[OPT_DATESTAMP];
			ARegCheck=opts[OPT_AREGCHECK];
			DRegCheck=opts[OPT_DREGCHECK];

			/* If alert patch is not to be done, set AlertOFF to NULL */
			if (opts[OPT_ALERTPATCH]) AlertOFF=NULL;

			/* Set the number of LED flashes */
			LED_Count=1;	/* Default is 1 */
			if (opts[OPT_LED]) LED_Count=(*(ULONG *)opts[OPT_LED]);

			/* Set the stack lines... */
			StackLines=2;	/* Default to 2... */
			if (opts[OPT_STACKLINES]) StackLines=(*(ULONG *)opts[OPT_STACKLINES]);

			/* Set the number of longwords to check for segments */
			if (opts[OPT_STACKCHECK]) SegLines=StackLines << 3;

			/* Check for verbose */
			if (!msg) if (opts[OPT_VERBOSE]) Verbose=TRUE;

			/*
			 * MMU_Flag is:
			 *
			 *	1 - For standard 68020/68030
			 *	2 - For 68040
			 *	3 - For 68060
			 */
			MMU_Flag=Test_MMU();
			if (mmu=Init_MMU_Frame(MMU_Flag))
			{
			struct	MemHeader	*mem;
			struct	MemHeader	*copies;
				LONG		oldPri=99; /* Default */

				if (opts[OPT_PRIORITY]) oldPri=(*(LONG *)opts[OPT_PRIORITY]);
				if ((oldPri>127) || (oldPri<-127))
				{
					if (!msg) PutStr("Invalid PRIORITY...  Using default.\n");
					oldPri=99;
				}

				/* We really need to be high priority... */
				oldPri=SetTaskPri((struct Task *)proc,oldPri);

				/*
				 * Map the ROM in...
				 */
				mmu=Map_ROM(mmu,ROM_Addr);
				if (Verbose) VPrintf("ROM Physical:   $%08lx  Size: $00080000 (512K)\tType=$FF CACHEABLE\n",(LONG *)&ROM_Addr);

				/*
				 * Protect $00F00000 card memory unless told not to...
				 * We do this up here in case someone made a "card" that
				 * does it later as something else...
				 */
				if (opts[OPT_FSPACE])
				{
					mmu=Mark_Address(mmu,0x00F00000,0x00080000,VALID | NONCACHEABLE);
				}
				else
				{
					mmu=Mark_Address(mmu,0x00F00000,0x00080000,VALID | CACHEABLE | WRITEPROTECT);
				}

				/*
				 * Map in the memory areas...
				 */
				if (copies=AllocVec(sizeof(struct MemHeader)*NUM_COPIES,MEMF_PUBLIC|MEMF_CLEAR))
				{
				LONG	numHeaders=0;
				LONG	i;

					Forbid();
					mem=(struct MemHeader *)SysBase->MemList.lh_Head;
					while ((mem->mh_Node.ln_Succ) && (numHeaders<NUM_COPIES))
					{
						copies[numHeaders++]=*mem;
						mem=(struct MemHeader *)(mem->mh_Node.ln_Succ);
					}
					Permit();

					for (i=0;i<numHeaders;i++)
					{
						mem=&copies[i];
						mmu=Mark_Address(mmu,(ULONG)(mem->mh_Lower),(ULONG)(mem->mh_Upper)-(ULONG)(mem->mh_Lower),((MEMF_CHIP & TypeOfMem(mem->mh_Lower)) ? (NONCACHEABLE | VALID) : (CACHEABLE | VALID)));

						if (Verbose)
						{
						ULONG	tmp[5];

							tmp[0]=(ULONG)(mem->mh_Lower);
							tmp[1]=(ULONG)(mem->mh_Upper)-(ULONG)(mem->mh_Lower);
							tmp[2]=tmp[1] / 1024;
							tmp[3]=(ULONG)(MEMF_CHIP & TypeOfMem(mem->mh_Lower));
							tmp[4]=tmp[3] ? ((ULONG)"CACHE DISABLED") : ((ULONG)"CACHEABLE");
							VPrintf("Memory Address: $%08lx  Size: $%08lx (%ldK)\tType=$%02lx %s\n",(LONG *)tmp);
						}
					}

					FreeVec(copies);
				}
				else
				{
					Free_MMU_Frame(mmu);
					mmu=NULL;
				}

				/*
				 * Map in the autoconfig boards
				 */
				if (ExpansionBase=OpenLibrary("expansion.library",0))
				{
				struct	ConfigDev	*cd=NULL;

					while (cd=FindConfigDev(cd,-1L,-1L))
					{
						/* Skip memory boards... */
						if (!(cd->cd_Rom.er_Type & ERTF_MEMLIST))
						{
							if (Verbose)
							{
							ULONG	tmp[4];

								tmp[0]=(ULONG)(cd->cd_BoardAddr);
								tmp[1]=(ULONG)(cd->cd_BoardSize);
								tmp[2]=tmp[1] / 1024;
								tmp[3]=cd->cd_Rom.er_Type;

								VPrintf("Board Address:  $%08lx  Size: $%08lx (%ldK)\tType=$%02lx CACHE DISABLED\n",(LONG *)tmp);
							}

							mmu=Mark_Address(mmu,(ULONG)(cd->cd_BoardAddr),cd->cd_BoardSize,VALID | NONCACHEABLE);
						}
					}
					CloseLibrary(ExpansionBase);
				}

				/*
				 * Now for the control areas...
				 */
				mmu=Mark_Address(mmu,0x00BC0000,0x00040000,VALID | NONCACHEABLE);
				mmu=Mark_Address(mmu,0x00D80000,0x00080000,VALID | NONCACHEABLE);

				/*
				 * Special case the first page of CHIP RAM
				 */
				mmu=Mark_Address(mmu,0,0x1000,VALID | NONCACHEABLE);

				/*
				 * If the credit card resource, make the addresses valid...
				 */
				if (OpenResource("card.resource"))
				{
					if (Verbose) PutStr("CARD Detected: $00600000   Size: $00440002 (4352K)\t\t CACHE DISABLED\n");
					mmu=Mark_Address(mmu,0x00600000,0x00440002,VALID | NONCACHEABLE);
				}

				/*
				 * If CDTV, make CDTV hardware valid...
				 */
				if (FindResident("cdstrap"))
				{
					if (Verbose) PutStr("CDTV Detected: $00E00000   Size: $00080000 (512K)\t\t CACHE DISABLED\n");
					mmu=Mark_Address(mmu,0x00E00000,0x00080000,VALID | NONCACHEABLE);
					mmu=Mark_Address(mmu,0x00B80000,0x00040000,VALID | NONCACHEABLE);
				}

				/*
				 * Check for ReKick/ZKick/KickIt
				 */
				if ((((ULONG)(SysBase->LibNode.lib_Node.ln_Name)) >> 16) == 0x20)
				{
					mmu=Mark_Address(mmu,0x00200000,0x00080000,VALID | CACHEABLE | WRITEPROTECT);
				}

				/*
				 * Check if we should mark low memory invalid...
				 */
				if (!opts[OPT_QUIET]) mmu=Mark_Invalid(mmu);

				/*
				 * If all OK still, we go and install ourselves...
				 */
				if (MMU_On(mmu))
				{
				ULONG	WaitSigs=SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F;
				BOOL	Done=FALSE;

					/*
					 * If we also have a timer to look at, add that
					 * signal bit to our mask...
					 */
					if (timerPort) WaitSigs |= (1L << timerPort->mp_SigBit);

					if (!msg)
					{
						if (Verbose) PutStr("\n");
						PutStr("Enforcer is on the job");
						if (opts[OPT_QUIET]) PutStr(" and silent");
						PutStr(".\n\n");
					}

/**************************************************************************************************************************************************************/
/**************************************************************************************************************************************************************/
					while (!Done)
					{
						/*
						 * So, we got a signal...  Lets do what we have to...
						 */
						while (ReadOffset != (temp1=WriteOffset))
						{
							/*
							 * Default change in position...
							 */
							if (ReadOffset>temp1) temp1=BufferSize;
							temp1-=ReadOffset;

							if (opts[OPT_RAWIO])
							{
								RawPutChar(OutputBuffer[ReadOffset]);
								temp1=1;
							}
							else if (OutputFile)
							{
								Write(OutputFile,&OutputBuffer[ReadOffset],temp1);
							}
							else if ((!msg) && opts[OPT_STDIO])
							{
								Write(Output(),&OutputBuffer[ReadOffset],temp1);
							}

							/*
							 * Now, adjust the read pointer
							 */
							temp1+=ReadOffset;
							if (temp1==BufferSize) temp1=0;
							ReadOffset=temp1;
						}

						/*
						 * Check if our timer went off...
						 * (We only have timer requests here)
						 */
						if (timerPort) if (GetMsg(timerPort))
						{
						struct	DateTime	ds;

							/*
							 * Ok, so we need to get a new date
							 * string put together...
							 */
							DateStamp(&(ds.dat_Stamp));
							ds.dat_Format=FORMAT_DOS;
							ds.dat_Flags=0;
							ds.dat_StrDay=NULL;
							ds.dat_StrDate=DateStr;
							ds.dat_StrTime=TimeStr;
							DateToStr(&ds);

							timerIO->tr_time.tv_secs=1;
							timerIO->tr_time.tv_micro=0;
							timerIO->tr_node.io_Command=TR_ADDREQUEST;
							SendIO((struct IORequest *)timerIO);
						}

						if (Wait(WaitSigs) & SIGBREAKF_CTRL_C)
						{
							if (MMU_Off(mmu)) Done=TRUE;
							else if (!msg) PutStr("Error - Can not exit at this time.\n");
						}
					}
/**************************************************************************************************************************************************************/
/**************************************************************************************************************************************************************/

					if (!msg) PutStr("Enforcer is no longer active.\n");
				}
				else if (!msg) PutStr("Error while building the MMU tables.\n");

				/* We go now...  So restore priority... */
				SetTaskPri((struct Task *)proc,oldPri);

				/*
				 * Free up the tables now...
				 */
				Free_MMU_Frame(mmu);
			}
			else if (!msg)
			{
				/*
				 * Maybe tell the user why...  ;^)
				 */
				PutStr("Sorry, but Enforcer can not run at this time, MMU is not available.\n");
			}
		}

		/*
		 * Release the timer resources
		 */
		if (timerOpen)
		{
			AbortIO((struct IORequest *)timerIO);
			WaitIO((struct IORequest *)timerIO);
			CloseDevice((struct IORequest *)timerIO);
		}
		DeleteIORequest((struct IORequest *)timerIO);
		DeleteMsgPort(timerPort);

		/*
		 * Free up the output buffer (if we have one)
		 */
		FreeVec(OutputBuffer);

		/*
		 * Close the output file if needed...
		 */
		if (OutputFile) Close(OutputFile);

		if (rdargs) FreeArgs(rdargs);
		if (myRDArgs) FreeDosObject(DOS_RDARGS,myRDArgs);

		proc->pr_Task.tc_Node.ln_Name=oldName;

		CloseLibrary(Lib68060);
		CloseLibrary(Lib68040);
		CloseLibrary(DOSBase);
	}
	else
	{
		if (!msg) if (DOSBase=OpenLibrary("dos.library",0))
		{
			Write(Output(),"Requires Kickstart 2.04 (37.175) or later.\n",43);
			CloseLibrary(DOSBase);
		}
	}

	if (msg)
	{
		Forbid();
		ReplyMsg((struct Message *)msg);
	}

	return(0);
}

static void *AllocAligned(ULONG size,ULONG chunk)
{
void	*data;

	if (data=AllocMem(size+chunk,MEMF_PUBLIC))
	{
		Forbid();
		FreeMem(data,size+chunk);
		data=AllocAbs(size,(APTR)((((ULONG)data)+chunk)&(~chunk)));
		Permit();
	}
	return(data);
}

/* Constants */
#define LEVELA_BITS_030		8	/* Increment in init loop is manually calculated */
#define LEVELB_BITS_030		6
#define LEVELC_BITS_030		8
#define LEVELA_SIZE_030		(1<<LEVELA_BITS_030)
#define LEVELB_SIZE_030		(1<<LEVELB_BITS_030)
#define LEVELC_SIZE_030		(1<<LEVELC_BITS_030)
#define LEVELA_ALLOC_030	(LEVELA_SIZE_030*4)
#define LEVELB_ALLOC_030	(LEVELB_SIZE_030*4)
#define LEVELC_ALLOC_030	(LEVELC_SIZE_030*4)

#define PAGE_SIZE_040	(0x1000)
#define	ALLOC_GRANA_040	(0x01FF)
#define	ALLOC_GRANB_040	(0x01FF)
#define	ALLOC_GRANC_040	(0x00FF)
#define	LEVELA_SIZE_040	(128)
#define	LEVELB_SIZE_040	(128)
#define	LEVELC_SIZE_040	(64)
#define	LEVELA_ALLOC_040	(4*LEVELA_SIZE_040)
#define	LEVELB_ALLOC_040	(4*LEVELB_SIZE_040)
#define	LEVELC_ALLOC_040	(4*LEVELC_SIZE_040)

#define	TABLE_MASK	(0xFFFFFFF3)

/*
 * Clean up the MMU Frame as needed...
 */
static void Free_MMU_Frame(struct MMU_Frame *mmu)
{
ULONG	i;
ULONG	j;
ULONG	*temp1;

	if (mmu)
	{
		switch (mmu->mmu_Flag)
		{
			/* 68030 MMU Frame Cleanup */
		case MMU_030:	if (mmu->mmu_LevelA) FreeMem(mmu->mmu_LevelA,LEVELA_ALLOC_030);
				if (mmu->mmu_LevelB) FreeMem(mmu->mmu_LevelB,LEVELB_ALLOC_030);
				if (mmu->mmu_LevelC) FreeMem(mmu->mmu_LevelC,LEVELC_ALLOC_030);
				break;

			/* 68040/68060 MMU Frame Cleanup */
		case MMU_040:
		case MMU_060:	if ((mmu->mmu_LevelA) && (mmu->mmu_LevelB) && (mmu->mmu_LevelC) && (mmu->mmu_Magic))
				{
					for (i=0;i<LEVELA_SIZE_040;i++) if ((mmu->mmu_LevelA[i]&TABLE_MASK) != mmu->mmu_InvalidA)
					{
						temp1=(ULONG *)(mmu->mmu_LevelA[i] & ~ALLOC_GRANB_040);
						for (j=0;j<LEVELB_SIZE_040;j++) if ((temp1[j]&TABLE_MASK) != mmu->mmu_InvalidB)
						{
							FreeMem((ULONG *)(temp1[j] & ~ALLOC_GRANC_040),LEVELC_ALLOC_040);
						}
						FreeMem(temp1,LEVELB_ALLOC_040);
					}
				}
				if (mmu->mmu_LevelA) FreeMem(mmu->mmu_LevelA,LEVELA_ALLOC_040);
				if (mmu->mmu_LevelB) FreeMem(mmu->mmu_LevelB,LEVELB_ALLOC_040);
				if (mmu->mmu_LevelC) FreeMem(mmu->mmu_LevelC,LEVELC_ALLOC_040);
				if (mmu->mmu_Magic)  FreeMem(mmu->mmu_Magic,PAGE_SIZE_040);
				break;
		}

		/* Free the MMU Frame */
		FreeVec(mmu);
	}
}

static struct MMU_Frame *Mark_Address(struct MMU_Frame *mmu,ULONG addr,ULONG size,ULONG orBits)
{
BOOL	rc=FALSE;	/* Assume failure */
ULONG	i;
ULONG	temp1;
ULONG	temp2;
ULONG	indexa;
ULONG	indexb;
ULONG	indexc;
ULONG	*levelb;
ULONG	*levelc;

	if (mmu)
	{
		switch (mmu->mmu_Flag)
		{
		case MMU_030:	temp1=addr >> 18;
				/* Crude hack routine here for Enforcer-like MMU tables...  Not a full general-perpose MMU thingy yet... */
				if (temp1 < (0xFF / 4))
				{
					temp2=(addr+size-1) >> 18;
					while (temp1 <= temp2)
					{
						if (!(mmu->mmu_LevelB[temp1] & 0x02)) mmu->mmu_LevelB[temp1] |= orBits;
						temp1++;
					}
				}
				else
				{
					temp1=addr >> 24;
					temp2=(addr+size-1) >> 24;
					while (temp1<=temp2)
					{
						mmu->mmu_LevelA[temp1] |= orBits;
						temp1++;
					}
				}
				rc=TRUE;
				break;
			/*
			 * For the 68040/68060 we need to maybe allocate another table
			 * if the address falls within that new table...
			 */
		case MMU_040:
		case MMU_060:	/* First, figure out the replacement orBits */
				temp1=0x20;	/* Copyback, cacheable, writeable, invalid */
				if (orBits & VALID) temp1 |= 0x01;	/* Resident */
				if (orBits & NONCACHEABLE) temp1 += 0x20;	/* Make the 0x20 into a 0x40 */
				if (orBits & WRITEPROTECT) temp1 |= 0x04;	/* Write protected */

				/* If you do 68060 MMU table type, we set to writethrough... */
				if (orBits == MMU_060_TYPE) temp1 = 0x0801;

				orBits=temp1;		/* New orBits */

				temp1=addr >> 12;	/* Remove page offset... */
				temp2=(addr+size-1) >> 12;

				if (temp1>temp2) rc=TRUE;	/* Small size is OK */

				while (mmu && (temp1 <= temp2))
				{
					rc=FALSE;			/* Assume we failed first... */
					indexa=temp1 >> 13;		/* Top 7 bits */
					indexb=(temp1 >> 6) & 0x7F;	/* Middle 7 bits */
					indexc=temp1 & 0x3F;		/* Bottom 6 bits */

					if (mmu->mmu_LevelA[indexa] == mmu->mmu_InvalidA)
					{
						/* We need a new LevelB here... */
						if (levelb=AllocAligned(LEVELB_ALLOC_040,ALLOC_GRANB_040))
						{
							mmu->mmu_LevelA[indexa]=(((ULONG)levelb) | 0x03);	/* Link it in */
							for (i=0;i<LEVELB_SIZE_040;i++) levelb[i]=mmu->mmu_InvalidB;

							/* Now, if 68060, mark it non-cached/serialized */
							if (mmu->mmu_Flag == MMU_060)
							{
								mmu=Mark_Address(mmu,(ULONG)levelb,LEVELB_ALLOC_040,MMU_060_TYPE);
							}
						}
					}

					if (mmu && (mmu->mmu_LevelA[indexa] != mmu->mmu_InvalidA))
					{
						levelb=(ULONG *)(mmu->mmu_LevelA[indexa] & ~ALLOC_GRANB_040);	/* Get table pointer */

						if (levelb[indexb] == mmu->mmu_InvalidB)
						{
							/* We need a new LevelC here... */
							if (levelc=AllocAligned(LEVELC_ALLOC_040,ALLOC_GRANC_040))
							{
								levelb[indexb]=((ULONG)levelc) | 0x03;	/* Link it in */
								for (i=0;i<LEVELC_SIZE_040;i++) levelc[i]=mmu->mmu_InvalidC;

								/* Now, if 68060, mark it non-cached/serialized */
								if (mmu->mmu_Flag == MMU_060)
								{
									mmu=Mark_Address(mmu,(ULONG)levelc,LEVELC_ALLOC_040,MMU_060_TYPE);
								}
							}
						}

						if (mmu && (levelb[indexb] != mmu->mmu_InvalidB))
						{
							levelc=(ULONG *)(levelb[indexb] & ~ALLOC_GRANC_040);	/* Get table pointer */

							if (levelc[indexc] == mmu->mmu_InvalidC) levelc[indexc]=(temp1 << 12);
							levelc[indexc] &= ~0x04;  /* Mask out the read-only flag */
							levelc[indexc] |= orBits;

							/* Cute trick - we make any table entry that could cause */
							/* an enforcer hit as local so that we only have to flush */
							/* the local ATC and not the global ATC... */
							if ((levelc[indexc] & 0x05)==0x01) levelc[indexc] |= 0x0400;
							else if (levelc[indexc] & 0x0400) levelc[indexc] -= 0x0400;

							/* Special case:  Check if we marked both cache and non-cache */
							/* non-cache wins every time, so force it to win! */
							if ((levelc[indexc] & 0x60) == 0x60) levelc[indexc]-=0x20;

							/* Special case:  Check if we set the 68060 MMU type user bit */
							/* If so, we set it to cached - writethrough */
							if (levelc[indexc] & 0x0800) levelc[indexc] &= ~0x60;

							rc=TRUE;	/* Operation worked... */
						}
					}

					temp1++;
				}
				break;
		}
	}

	if (!rc)
	{
		Free_MMU_Frame(mmu);
		mmu=NULL;
	}

	return(mmu);
}

/*
 * This routine will map the ROM addresses to the physical
 * addresses given here...
 */
static struct MMU_Frame *Map_ROM(struct MMU_Frame *mmu,ULONG addr)
{
ULONG	temp1;
ULONG	*level;

	mmu=Mark_Address(mmu,0x00F80000,0x00080000,VALID | CACHEABLE | WRITEPROTECT);

	/*
	 * Check if we need to do a special mapping...
	 */
	if (mmu) if (0x00F80000 != addr)
	{
		switch (mmu->mmu_Flag)
		{
			/*
			 * The 68030 case is rather simple...  ;^)
			 */
		case MMU_030:	mmu->mmu_LevelB[0xF8 / 4] = addr | 0x1D;		/* Cacheable, Writeprotected, Resident */
				mmu->mmu_LevelB[0xFC / 4] = (addr+0x00040000) | 0x1D;	/* Cacheable, Writeprotected, Resident */
				break;

			/*
			 * The 68040/68060 case is a bit harder...
			 */
		case MMU_040:
		case MMU_060:	temp1=0x00F80000 >> 12;
				/* Starting at the ROM, do the full 512K space mapping... */
				while (temp1 <= (0x00FFFFFF >> 12))
				{
					level=(ULONG *)(mmu->mmu_LevelA[temp1 >> 13] & ~ALLOC_GRANB_040);	/* LevelB */
					level=(ULONG *)(level[(temp1 >> 6) & 0x7F] & ~ALLOC_GRANC_040);		/* LevelC */
					level[temp1 & 0x3F] = addr | 0x025;	/* Local, Cacheable, Writeprotected, Resident */

					temp1++;	addr+=0x00001000;	/* Next page:  4K at a shot... */
				}
				break;
		}
	}

	return(mmu);
}

/*
 * This routine will mark the invalid addresses as invalid
 * The whole address page is marked, not just the single address
 * since this would be impossible on the current MMUs
 * It will also make the indirect frame marked as invalid.
 */
static struct MMU_Frame *Mark_Invalid(struct MMU_Frame *mmu)
{
	if (mmu)
	{
		switch (mmu->mmu_Flag)
		{
			/*
			 * The 68030 case is rather simple...  ;^)
			 */
		case MMU_030:	mmu->mmu_LevelC[0]=0xBADF00D0;
				break;

			/*
			 * The 68040/68060 case is a bit harder...
			 */
		case MMU_040:
		case MMU_060:	/*
				 * First make sure the table is build
				 */
				mmu->mmu_Indirect=((ULONG)(mmu->mmu_Magic)) | 0x044; /* invalid, write protected, non-cached, serialized */
				if (mmu=Mark_Address(mmu,0,1,0))
				{
				ULONG	*level;

					/* Now we know the tree down to the address is there... */
					level=(ULONG *)(mmu->mmu_LevelA[0] & ~ALLOC_GRANB_040);	/* Get LevelB */
					level=(ULONG *)(level[0] & ~ALLOC_GRANC_040);		/* Get LevelC */
					level[0]=0x00000044;					/* Non-cached, serialized, write protected, invalid */
					mmu->mmu_Page0=level;					/* Store Page 0 address */
				}
				break;
		}
	}
	return(mmu);
}

static struct MMU_Frame *Init_MMU_Frame(ULONG MMU_Flag)
{
struct	MMU_Frame	*mmu;
	ULONG		i;
	BOOL		rc=FALSE;

	if (mmu=AllocVec(sizeof(struct MMU_Frame),MEMF_PUBLIC|MEMF_CLEAR))
	{
		switch (mmu->mmu_Flag=MMU_Flag)
		{
			/* Initialize the 68030 frame... */
		case MMU_030:	mmu->mmu_LevelA=AllocAligned(LEVELA_ALLOC_030,0x0000000F);
				mmu->mmu_LevelB=AllocAligned(LEVELB_ALLOC_030,0x0000000F);
				mmu->mmu_LevelC=AllocAligned(LEVELC_ALLOC_030,0x0000000F);

				if ((mmu->mmu_LevelA) && (mmu->mmu_LevelB) && (mmu->mmu_LevelC))
				{
					/*
					 * Now build the base MMU table...
					 */
					for (i=0;i<LEVELA_SIZE_030;i++) mmu->mmu_LevelA[i]=(i << 24);
					for (i=0;i<LEVELB_SIZE_030;i++) mmu->mmu_LevelB[i]=(i << 18);
					for (i=0;i<LEVELC_SIZE_030;i++) mmu->mmu_LevelC[i]=(i << 10) | VALID | NONCACHEABLE;

					/*
					 * Connect up the tables.
					 */
					mmu->mmu_TC	=0x80A08680;
					mmu->mmu_CRP[0]	=0x80000002;
					mmu->mmu_CRP[1]	=(ULONG)mmu->mmu_LevelA;
					mmu->mmu_LevelA[0]=((ULONG)mmu->mmu_LevelB) | 0x02;
					mmu->mmu_LevelB[0]=((ULONG)mmu->mmu_LevelC) | 0x02;

					rc=TRUE;
				}
				break;

			/* Initialize the 68040/68060 frame... */
		case MMU_040:
		case MMU_060:	mmu->mmu_LevelA=AllocAligned(LEVELA_ALLOC_040,ALLOC_GRANA_040);
				mmu->mmu_LevelB=AllocAligned(LEVELB_ALLOC_040,ALLOC_GRANB_040);
				mmu->mmu_LevelC=AllocAligned(LEVELC_ALLOC_040,ALLOC_GRANC_040);
				mmu->mmu_Magic=AllocAligned(PAGE_SIZE_040,PAGE_SIZE_040-1);

				if ((mmu->mmu_LevelA) && (mmu->mmu_LevelB) && (mmu->mmu_LevelC) && (mmu->mmu_Magic))
				{
					/* First, set up the fake page memory to be what I want */
					for (i=0;i<(PAGE_SIZE_040/4);i++) mmu->mmu_Magic[i]=Bad_ReadValue;

					/* Now, set up the default invalid page pointers... */
					mmu->mmu_Indirect=((ULONG)(mmu->mmu_Magic)) | 0x041; /* valid, non-cached, serialized */
					mmu->mmu_InvalidC=((ULONG)&(mmu->mmu_Indirect)) | 0x02; /* Indirect to the invalid one... */
					mmu->mmu_InvalidB=((ULONG)(mmu->mmu_LevelC)) | 0x03; /* Resident table */
					mmu->mmu_InvalidA=((ULONG)(mmu->mmu_LevelB)) | 0x03; /* Resident table */

					for (i=0;i<LEVELC_SIZE_040;i++) mmu->mmu_LevelC[i]=mmu->mmu_InvalidC;
					for (i=0;i<LEVELB_SIZE_040;i++) mmu->mmu_LevelB[i]=mmu->mmu_InvalidB;
					for (i=0;i<LEVELA_SIZE_040;i++) mmu->mmu_LevelA[i]=mmu->mmu_InvalidA;

					mmu->mmu_TC=0x00008000;	/* Enable MMU with 4K pages */
					mmu->mmu_CRP[0]=(ULONG)mmu->mmu_LevelA;
					mmu->mmu_CRP[1]=(ULONG)mmu->mmu_LevelA;

					/* For simple games, make default PAGE0 point at the indirect page */
					mmu->mmu_Page0=&(mmu->mmu_Indirect);

					/* We now have a full 68040 MMU tree, everything is marked invalid... */
					rc=TRUE;

					/* Now, if 68060, mark it non-cached/serialized */
					if (mmu->mmu_Flag == MMU_060)
					{
						if (mmu) mmu=Mark_Address(mmu,(ULONG)mmu->mmu_LevelA,LEVELA_ALLOC_040,MMU_060_TYPE);
						if (mmu) mmu=Mark_Address(mmu,(ULONG)mmu->mmu_LevelB,LEVELB_ALLOC_040,MMU_060_TYPE);
						if (mmu) mmu=Mark_Address(mmu,(ULONG)mmu->mmu_LevelC,LEVELC_ALLOC_040,MMU_060_TYPE);
					}
				}
				break;
		}
	}

	if (!rc)
	{
		Free_MMU_Frame(mmu);
		mmu=NULL;
	}

	return (mmu);
}

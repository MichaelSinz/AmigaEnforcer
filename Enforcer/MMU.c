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
******* MMU *******************************************************************
*
*   NAME
*	MMU - A 68040/68060 MMU Mapping Tool
*
*   SYNOPSIS
*	MMU is a tool for 68040/060 systems to display and/or change the MMU
*	configuration contained within.  MMU will display the tables as
*	compressed as possible, noting address ranges that are mapped the
*	same and only displaying one line for the whole range.
*
*   INPUTS
*	ADDRESS/K	- Hex address to display (if not given, all)
*	SIZE/K		- Hex address size of address range
*	READWRITE	- Change address range to ReadWrite
*	READONLY	- Change address range to ReadOnly
*	VALID		- Change address range to VALID
*	INVALID		- Change address range to INVALID
*	CACHE		- Change address range to CACHE enable
*	NOCACHE		- Change address range to CACHE disable
*	COPYBACK	- Change address range to non-serialized
*	NOCOPYBACK	- Change address range to serialized
*	GLOBAL		- Change address range to GLOBAL
*	LOCAL		- Change address range to LOCAL
*	SUPERVISOR	- Change address range to SUPERVISOR-only
*	USER		- Change address range to USER access
*	VERBOSE		- Show the changes as they are made
*	NOSHOW		- Do not show the whole table
*
*   RESULTS
*	On my A3000 with a 68040 installed and Enforcer running, the
*	MMU output looks as follows:  (No options)
*
*Current 68040 MMU table setup:
*$00000000-$00000FFF->$00000000:  Local, User, Invalid, Read-Only,  Serialized
*$00001000-$001FFFFF->$00001000: Global, User,   Valid, Read/Write, Serialized
*$00200000-$00BBFFFF->$07542000:  Local, User, Invalid, Read-Only,  Serialized
*$00BC0000-$00BFFFFF->$00BC0000: Global, User,   Valid, Read/Write, Serialized
*$00C00000-$00D7FFFF->$07542000:  Local, User, Invalid, Read-Only,  Serialized
*$00D80000-$00DFFFFF->$00D80000: Global, User,   Valid, Read/Write, Serialized
*$00E00000-$00E8FFFF->$07542000:  Local, User, Invalid, Read-Only,  Serialized
*$00E90000-$00EAFFFF->$00E90000: Global, User,   Valid, Read/Write, Serialized
*$00EB0000-$00EFFFFF->$07542000:  Local, User, Invalid, Read-Only,  Serialized
*$00F00000-$00FFFFFF->$00F00000: Global, User,   Valid, Read-Only,    Copyback
*$01000000-$073FFFFF->$07542000:  Local, User, Invalid, Read-Only,  Serialized
*$07400000-$07F7FFFF->$07400000: Global, User,   Valid, Read/Write,   Copyback
*$07F80000-$FFFFFFFF->$07542000:  Local, User, Invalid, Read-Only,  Serialized
*
*   WARNING
*	This tool is a hack and does not have any safeguards on the options.
*	Use at your own risk.
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

#include	<strings.h>

#define EXECBASE (*(struct ExecBase **)4)

#define TEMPLATE  "ADDRESS/K,SIZE/K,READWRITE/S,READONLY/S,VALID/S,INVALID/S,CACHE/S,NOCACHE/S,COPYBACK/S,NOCOPYBACK/S,GLOBAL/S,LOCAL/S,SUPERVISOR/S,USER/S,VERBOSE/S,NOSHOW/S"

#define	OPT_ADDRESS	0
#define	OPT_SIZE	1
#define	OPT_READWRITE	2
#define	OPT_READONLY	3
#define	OPT_VALID	4
#define	OPT_INVALID	5
#define	OPT_CACHE	6
#define	OPT_NOCACHE	7
#define	OPT_COPYBACK	8
#define	OPT_NOCOPYBACK	9
#define	OPT_GLOBAL	10
#define	OPT_LOCAL	11
#define	OPT_SUPERVISOR	12
#define	OPT_USER	13
#define	OPT_VERBOSE	14
#define	OPT_NOSHOW	15
#define	OPT_COUNT	16

ULONG __asm GetPageEntry(register __a6 struct ExecBase *,register __a0 ULONG);
void __asm SetPageEntry(register __a6 struct ExecBase *,register __d0 ULONG,register __d1 ULONG);

ULONG toLong(char *);
void DisplayPage(struct Library *,ULONG);

#define	PAGE_SIZE	0x00001000L
#define	PAGE_MASK	0xFFFFF000L

/* Important type bits... */
#define	TYPE_BITS	0x000004E7L

ULONG cmd(void)
{
struct	ExecBase	*SysBase;
struct	Library		*DOSBase;
struct	Process		*proc;
struct	RDArgs		*rdargs=NULL;
struct	WBStartup	*msg=NULL;
struct	RDArgs		*myRDArgs=NULL;
	ULONG		rc=RETURN_FAIL;
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
	struct	Library		*Lib68040;
	struct	Library		*Lib68060;

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
			if (!(rdargs = ReadArgs(TEMPLATE, opts, NULL))) PrintFault(IoErr(),NULL);
			else if (SetSignal(0,0) & SIGBREAKF_CTRL_C) PrintFault(ERROR_BREAK,NULL);
			else rc=RETURN_OK;
		}

		if (RETURN_OK==rc) if (Lib68040 || Lib68060)
		{
			if (GetPageEntry(SysBase,~0))
			{
				/*
				 * Do the stuff here...
				 */
				if (opts[OPT_ADDRESS])
				{
				ULONG	Address;

					Address=toLong((char *)opts[OPT_ADDRESS]);

					if (opts[OPT_SIZE])
					{
					ULONG	Size;
					ULONG	Page;

						Size=toLong((char *)opts[OPT_SIZE]);

						Size += Address;
						Address &= PAGE_MASK;

						Forbid();
						while (Address < Size)
						{
							Page=GetPageEntry(SysBase,Address);

							if (opts[OPT_READWRITE])	Page &= ~0x00000004;
							if (opts[OPT_READONLY])		Page |=  0x00000004;
							if (opts[OPT_VALID])		Page = (Page & ~0x00000003) | 0x00000001;
							if (opts[OPT_INVALID])		Page = (Page & ~0x00000003) | 0x00000000;
							if (opts[OPT_CACHE])		Page &= ~0x00000040;
							if (opts[OPT_NOCACHE])		Page |=  0x00000040;
							if (opts[OPT_COPYBACK])		Page |=  0x00000020;
							if (opts[OPT_NOCOPYBACK])	Page &= ~0x00000020;
							if (opts[OPT_GLOBAL])		Page |=  0x00000400;
							if (opts[OPT_LOCAL])		Page &= ~0x00000400;
							if (opts[OPT_SUPERVISOR])	Page |=  0x00000080;
							if (opts[OPT_USER])		Page &= ~0x00000080;

							SetPageEntry(SysBase,Address,Page);

							if (opts[OPT_VERBOSE]) if (!msg)
							{
							ULONG	tmp[2];

								tmp[0]=Address;
								tmp[1]=Page & PAGE_MASK;
								VPrintf("Entry for $%08lx -> $%08lx: ",(LONG *)tmp);
								DisplayPage(DOSBase,Page);
							}

							Address+=PAGE_SIZE;

							if (SetSignal(0L,0L) & SIGBREAKF_CTRL_C)
							{
								PrintFault(ERROR_BREAK,NULL);
								Size=0;
							}
						}
						Permit();
					}
				}

				if (!opts[OPT_NOSHOW]) if (!msg)
				{
				ULONG	LastPage;
				ULONG	LastAddr=NULL;
				ULONG	Page;
				ULONG	Addr;
				ULONG	flag;
				ULONG	range=0xFFFFFFFFL;


					if (Lib68040) PutStr("\nCurrent 68040 MMU table setup:\n");
					if (Lib68060) PutStr("\nCurrent 68060 MMU table setup:\n");

					if (opts[OPT_ADDRESS]) LastAddr=toLong((char *)opts[OPT_ADDRESS]) & PAGE_MASK;
					if (opts[OPT_SIZE]) range=toLong((char *)opts[OPT_SIZE]);

					LastPage=GetPageEntry(SysBase,LastAddr);
					Addr=LastAddr+PAGE_SIZE;
					while (range)
					{
						Page=GetPageEntry(SysBase,Addr);

						flag=FALSE;
						if (range<=PAGE_SIZE) flag=TRUE;
						else if ((Page & TYPE_BITS) != (LastPage & TYPE_BITS)) flag=TRUE;
						else if ((Page & PAGE_MASK) != (LastPage & PAGE_MASK))
						{
							if (((Page & PAGE_MASK) - (LastPage & PAGE_MASK)) != (Addr - LastAddr)) flag=TRUE;
						}

						if (flag)
						{
						ULONG	tmp[3];

							tmp[0]=LastAddr;
							tmp[1]=0xFFFFFFFFL;
							if (Addr+PAGE_SIZE) tmp[1]=Addr-1;
							tmp[2]=LastPage & PAGE_MASK;
							VPrintf("$%08lx-$%08lx -> $%08lx: ",(LONG *)tmp);

							DisplayPage(DOSBase,LastPage);

							LastPage=Page;
							LastAddr=Addr;
						}

						Addr+=PAGE_SIZE;
						if (range>PAGE_SIZE) range-=PAGE_SIZE;
						else range=0;

						if (SetSignal(0L,0L) & SIGBREAKF_CTRL_C)
						{
							PrintFault(ERROR_BREAK,NULL);
							Addr=0;
							range=0;
						}
					}
				}
			}
			else if (!msg) PutStr("Requires 68040 or 68060 with active 4K MMU\n");

		}
		else if (!msg) PutStr("Requires a 68040 and 68040.library\n      or a 68060 and 68060.library\n");

		if (rdargs) FreeArgs(rdargs);
		if (myRDArgs) FreeDosObject(DOS_RDARGS,myRDArgs);

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

void DisplayPage(struct Library *DOSBase,ULONG Page)
{
	if (Page & 0x400) PutStr("Global, ");
	else PutStr(" Local, ");

	if (Page & 0x80) PutStr("Super, ");
	else PutStr("User,  ");

	if (Page & 1) PutStr("  Valid, ");
	else PutStr("Invalid, ");

	if (Page & 0x04) PutStr("Read-Only,  ");
	else PutStr("Read/Write, ");

	switch (Page & 0x60)
	{
	case 0x00:	PutStr("     Cache");	break;
	case 0x20:	PutStr("  Copyback");	break;
	case 0x40:	PutStr("Serialized");	break;
	case 0x60:	PutStr("   Nocache");	break;
	}

	PutStr("\n");
}

ULONG toLong(char *hex)
{
ULONG	val=0;
ULONG	c;

	if (*hex=='$') hex++;	/* Support $hex */
	while (*hex)
	{
		c=(ULONG)*hex;
		if ((c>='a') && (c<='f')) c-=32;
		c-='0';
		if (c>9)
		{
			c-=7;
			if (c<10) c=16;
		}
		if (c<16)
		{
			val=(val << 4) + c;
			hex++;
		}
		else val=0;
	}

	return(val);
}

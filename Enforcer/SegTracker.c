/*
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
*/

/*
******* SegTracker ************************************************************
*
*   NAME
*	SegTracker - A global SegList tracking utility
*
*   SYNOPSIS
*	A global tracking utility for disk loaded files including
*	libraries and devices.  If placed in the startup-sequence
*	right after SetPatch, it will track all disk loaded segments
*	(other than those loaded by SetPatch)
*
*   FUNCTION
*	SegTracker will patch the DOS LoadSeg(), NewLoadSeg(), and UnLoadSeg()
*	functions in order to track the SegLists that are loaded.
*	SegTracker keeps these seglist stored in a "safe" manner and
*	even handles programs which SegList split.
*
*	The first time the program is run, it installs the patches
*	and semaphore.  After that point, it just finds the semaphore
*	and uses it.
*
*	When SegTracker is installed, it will scan the ROM for ROM modules
*	and add their locations to the tracking list such that addresses
*	within those modules can be identified.  Note that the offsets
*	from the module is based on the location of the module's ROMTAG.
*	The NOROM option will prevent this feature from being installed.
*
*	By using SegTracker, it will be possible to better identify
*	where Enforcer hits come from when dealing with libraries
*	and devices.  Basically, it is a system-global Hunk-o-matic.
*
*	External programs can then pass in an address to SegTracker
*	either via the command line or via the given function pointer
*	in the SegTracker semaphore and get back results as to what
*	hunk and offset the address is at.
*
*	To work with the function directly, you need to find the
*	the semaphore of "SegTracker" using FindSemaphore().
*	The structure found will be the following:
*
*	struct  SegSem
*	{
*	struct  SignalSemaphore seg_Semaphore;
*	        SegTrack        *seg_Find;
*	};
*
*	The function pointer points to a routine that takes an address
*	and two pointers to long words for returning the Segment number
*	and Offset within the segment.  The function returns the name
*	of the file loaded.  Note that you must call this function
*	while in Forbid() and then copy the name as the seglist may
*	be UnLoadSeg'ed at any moment and the name string will then
*	no longer be in memory.
*
*	typedef	char (* __asm SegTrack(register __a0 ULONG Address,
*	                               register __a1 ULONG *SegNum,
*	                               register __a2 ULONG *Offset));
*
*	The above is for use in C code function pointer prototype
*	in SAS/C 5 and 6.
*
*   INPUTS
*	SHOW/S  - Shows all of the segments being tracked.
*
*	DUMP/S  - Displays all of the segment elements being tracked.
*
*	NOROM/S - Tells segtracker not to scan ROM when it is
*	          installed, thus not adding ROM addresses to the
*	          tracking list.
*
*	FIND/M  - Find the hex (in $xxxxx format) address in
*	          the tracked segments.  Multiple addresses
*	          can be given.
*
*	Options are not available from Workbench as they require
*	the CLI.  However, you can run SegTracker from Workbench
*	to install it.
*
*   EXAMPLE USAGE
*	\*
*	 * A simple program that will "find" given addresses in the SegLists
*	 * This program has been compiled with SAS/C 6.2 without errors or
*	 * warnings.
*	 *
*	 * Compiler options:
*	 * DATA=FARONLY PARAMETERS=REGISTER NOSTACKCHECK
*	 * NOMULTIPLEINCLUDES STRINGMERGE STRUCTUREEQUIVALENCE
*	 * MULTIPLECHARACTERCONSTANTS DEBUG=LINE NOVERSION
*	 * OPTIMIZE OPTIMIZERINLOCAL NOICONS
*	 *
*	 * Linker options:
*	 * FindSeg.o TO FindSeg SMALLCODE SMALLDATA NODEBUG LIB LIB:sc.lib
*	 *\
*	#include <exec/types.h>
*	#include <exec/execbase.h>
*	#include <exec/libraries.h>
*	#include <exec/semaphores.h>
*	#include <dos/dos.h>
*	#include <dos/dosextens.h>
*	#include <dos/rdargs.h>
*
*	#include <clib/exec_protos.h>
*	#include <pragmas/exec_sysbase_pragmas.h>
*
*	#include <clib/dos_protos.h>
*	#include <pragmas/dos_pragmas.h>
*
*	#include <string.h>
*
*	#include "FindSeg_rev.h"
*
*	#define EXECBASE (*(struct ExecBase **)4)
*
*	typedef char (* __asm SegTrack(register __a0 ULONG,
*	                               register __a1 ULONG *,
*	                               register __a2 ULONG *));
*
*	struct SegSem
*	{
*	struct SignalSemaphore seg_Semaphore;
*	       SegTrack        *seg_Find;
*	};
*
*	#define SEG_SEM "SegTracker"
*
*	#define TEMPLATE "FIND/M" VERSTAG
*
*	#define OPT_FIND  0
*	#define OPT_COUNT 1
*
*	ULONG cmd(void)
*	{
*	struct ExecBase *SysBase;
*	struct Library  *DOSBase;
*	struct RDArgs   *rdargs;
*	       ULONG    rc=RETURN_FAIL;
*	struct SegSem   *segSem;
*	       char     **hex;
*	       LONG     opts[OPT_COUNT];
*
*	  SysBase = EXECBASE;
*	  if (DOSBase = OpenLibrary("dos.library",37))
*	  {
*	    memset((char *)opts, 0, sizeof(opts));
*
*	    if (!(rdargs = ReadArgs(TEMPLATE, opts, NULL)))
*	    {
*	      PrintFault(IoErr(),NULL);
*	    }
*	    else if (CheckSignal(SIGBREAKF_CTRL_C))
*	    {
*	      PrintFault(ERROR_BREAK,NULL);
*	    }
*	    else if (segSem=(struct SegSem *)FindSemaphore(SEG_SEM))
*	    {
*	      rc=RETURN_OK;
*	      if (opts[OPT_FIND])
*	      {
*	        for (hex=(char **)opts[OPT_FIND];(*hex);hex++)
*	        {
*	        char  *p;
*	        ULONG val;
*	        ULONG tmp[4];
*	        ULONG c;
*
*	          val=0;
*	          p=*hex;
*	          if (*p=='$') p++; \* Support $hex *\
*	          while (*p)
*	          {
*	            c=(ULONG)*p;
*	            if ((c>='a') && (c<='f')) c-=32;
*	            c-='0';
*	            if (c>9)
*	            {
*	              c-=7;
*	              if (c<10) c=16;
*	            }
*
*	            if (c<16)
*	            {
*	              val=(val << 4) + c;
*	              p++;
*	            }
*	            else
*	            {
*	              val=0;
*	              p=&p[strlen(p)];
*	            }
*	          }
*
*	          \*
*	           * Ok, we need to do this within Forbid()
*	           * as segments can unload at ANY time, including
*	           * during AllocMem(), so we use a stack buffer...
*	           *
*	           *\
*	          Forbid();
*	          if (p=(*segSem->seg_Find)(tmp[0]=val,&tmp[2],&tmp[3]))
*	          {
*	          char Buffer[200];
*
*	            stccpy(Buffer,p,200);
*	            tmp[1]=(ULONG)Buffer;
*	            VPrintf("$%08lx - %s : Hunk %ld, Offset $%08lx",tmp);
*
*	            \*
*	             * Now get the SegList address by passing the
*	             * same pointer for both hunk & offset.  Note
*	             * that this is only in the newer SegTrackers
*	             * To test if this worked, check if the result
*	             * of this call is either a hunk or an offset.
*	             *\
*	            (*segSem->seg_Find)(val,&tmp[0],&tmp[0]);
*	            \*
*	             * This "kludge" is for compatibility reasons
*	             * Check if result is the same as either the hunk
*	             * or the offset.  If so, do not print it...
*	             *\
*	            if ((tmp[0]!=tmp[2]) && (tmp[0]!=tmp[3]))
*	            {
*	              VPrintf(", SegList $%08lx",tmp);
*	            }
*
*	            PutStr("\n");
*	          }
*	          else VPrintf("$%08lx - Not found\n",tmp);
*	          Permit();
*	        }
*	      }
*	    }
*	    else PutStr("Could not find SegTracker semaphore.\n");
*
*	    if (rdargs) FreeArgs(rdargs);
*	    CloseLibrary(DOSBase);
*	  }
*	  else if (DOSBase=OpenLibrary("dos.library",0))
*	  {
*	    Write(Output(),"Requires Kickstart 2.04 (37.175) or later.\n",43);
*	    CloseLibrary(DOSBase);
*	  }
*
*	  return(rc);
*	}
*
*   NOTES
*	The earlier this command is run, the better off it will be in
*	tracking disk loaded segments.  Under debug usage, you may
*	wish to run the command right *AFTER* SetPatch.
*
*	Some things may not call UnLoadSeg() to free their seglists.
*	There is no way SegTracker can follow a seglist that is not
*	unloaded via the dos.library call to UnLoadSeg().  For this
*	reason, SegTracker adds new LoadSeg() segments to the top
*	of its list.  This way, if any old segments are still on
*	the list but have been unloaded via some other method
*	they will not clash with newer segments during the find operation.
*
*	Note that the resident list is one such place where
*	UnLoadSeg() is not called to free the seglist.  Thus,
*	if something is made resident and then later unloaded
*	it will still be listed as tracked by SegTracker.
*
*	In order to support a new feature in CPR, the SegTracker function
*	got a "kludge" added to it.  If a segment is found, you can then
*	call the function again with the same address but with having
*	both pointers point to the same longword of storage.  By doing
*	this, the function will now return (in that longword) the
*	SegList pointer (CPTR not BPTR) of the file that contains
*	the address.  The reason this method was used was so it
*	would be compatible with older SegTracker versions.  In older
*	versions you would not get the result you wanted but you would
*	also not crash.  See the example above for more details on how
*	to use this feature.  The SegTracker FIND option has been
*	expanded to include this information.
*
*	Due to the fact that I am working on a design of a new set of
*	debugging tools (Enforcer/SegTracker/etc)  I do not wish to
*	expand the current SegTracker model in too many ways.
*
*   SEE ALSO
*	"Quantum Physics:  The Dreams that Stuff is made of." - Michael Sinz
*
*   BUGS
*
*******************************************************************************
*/

#include	<exec/types.h>
#include	<exec/execbase.h>
#include	<exec/tasks.h>
#include	<exec/memory.h>
#include	<exec/alerts.h>
#include	<exec/ports.h>
#include	<exec/libraries.h>
#include	<exec/semaphores.h>
#include	<exec/resident.h>
#include	<dos/dos.h>
#include	<dos/dosextens.h>
#include	<dos/rdargs.h>
#include	<devices/timer.h>
#include	<workbench/startup.h>
#include	<workbench/workbench.h>
#include	<libraries/configvars.h>

#include	<clib/exec_protos.h>
#include	<pragmas/exec_pragmas.h>

#include	<clib/dos_protos.h>
#include	<pragmas/dos_pragmas.h>

#include	<string.h>

#include	"SegTracker_rev.h"

#define EXECBASE (*(struct ExecBase **)4)

/******************************************************************************/

#define TEMPLATE	"SHOW/S,DUMP/S,NOROM/S,FIND/M" VERSTAG
#define	SEG_SEM		"SegTracker"

#define	OPT_SHOW	0
#define	OPT_DUMP	1
#define	OPT_NOROM	2
#define	OPT_FIND	3
#define	OPT_COUNT	4

#define NEWLIST(l) {((struct MinList *)l)->mlh_Head = (struct MinNode *)&(((struct MinList *)l)->mlh_Tail);\
                    ((struct MinList *)l)->mlh_Tail = NULL;\
                    ((struct MinList *)l)->mlh_TailPred = (struct MinNode *)l;}

typedef	char (* __asm SegTrack(register __a0 ULONG,register __a1 ULONG *,register __a2 ULONG *));

struct	SegSem
{
struct	SignalSemaphore	seg_Semaphore;
	SegTrack	*seg_Find;
struct	MinList		seg_List;
};

struct	SegArray
{
	ULONG	seg_Address;
	ULONG	seg_Size;
};

struct	SegNode
{
struct	MinNode		seg_Node;
	char		*seg_Name;
struct	SegArray	seg_Array[1];
};

/*
 * Some global data needed to make this all work...
 */
struct	ExecBase	*SysBase;
struct	Library		*DOSBase;
struct	SegSem		MySem;

extern	long	LVOLoadSeg;
extern	long	LVONewLoadSeg;
extern	long	LVOUnLoadSeg;
// extern	long	LVORemSegment;
extern	long	__stdargs MyLoadSeg();
extern	long	__stdargs MyNewLoadSeg();
extern	long	__stdargs MyUnLoadSeg();
// extern	long	__stdargs MyRemSegment();
extern	ULONG	OldLoadSeg;
extern	ULONG	OldNewLoadSeg;
extern	ULONG	OldUnLoadSeg;
// extern	ULONG	OldRemSegment;

char * __asm FindSeg(register __a0 ULONG Address,register __a1 ULONG *SegNum,register __a2 ULONG *Offset);

VOID	AddROM(VOID);

ULONG cmd(void)
{
struct	Process		*proc;
struct	RDArgs		*rdargs=NULL;
struct	WBStartup	*msg=NULL;
	ULONG		rc=RETURN_FAIL;
struct	SegSem		*segSem=NULL;
	char		**hex;
	LONG		opts[OPT_COUNT];

	SysBase = EXECBASE;
	NEWLIST(&(MySem.seg_List));

	proc=(struct Process *)FindTask(NULL);

	if (!(proc->pr_CLI))
	{
		WaitPort(&(proc->pr_MsgPort));
		msg=(struct WBStartup *)GetMsg(&(proc->pr_MsgPort));
	}

	if (DOSBase = OpenLibrary("dos.library",37))
	{
		memset((char *)opts, 0, sizeof(opts));

		/*
		 * Do the option parsing
		 * If from Workbench, no options...
		 * If from CLI, use ReadArgs
		 */
		if (msg) rc=RETURN_OK;
		else
		{
			/*
			 * Started from CLI so do standard ReadArgs
			 */
			if (!(rdargs = ReadArgs(TEMPLATE, opts, NULL))) PrintFault(IoErr(),NULL);
			else if (CheckSignal(SIGBREAKF_CTRL_C)) PrintFault(ERROR_BREAK,NULL);
			else rc=RETURN_OK;
		}

		if (RETURN_OK==rc)
		{
			Forbid();
			if (!(segSem=(struct SegSem *)FindSemaphore(SEG_SEM)))
			{
				InitSemaphore((struct SignalSemaphore *)&MySem);
				segSem=&MySem;
				segSem->seg_Semaphore.ss_Link.ln_Name=SEG_SEM;
				segSem->seg_Semaphore.ss_Link.ln_Pri=-127;
				segSem->seg_Find=FindSeg;
				AddSemaphore((struct SignalSemaphore *)segSem);

				/* Add the ROM modules (unless told not to)... */
				if (!opts[OPT_NOROM]) AddROM();
			}

			if (opts[OPT_SHOW])
			{
			struct	SegNode	*seg;

				PutStr("Segments being tracked:\n");
				seg=(struct SegNode *)segSem->seg_List.mlh_Head;
				while ((seg->seg_Node.mln_Succ) && (!(SetSignal(0,0) & SIGBREAKF_CTRL_C)))
				{
					PutStr("\t");
					PutStr(seg->seg_Name);
					PutStr("\n");
					seg=(struct SegNode *)(seg->seg_Node.mln_Succ);
				}
			}

			if (opts[OPT_DUMP])
			{
			struct	SegNode	*seg;
				ULONG	tmp[3];

				PutStr("Segments being tracked:\n");
				seg=(struct SegNode *)segSem->seg_List.mlh_Head;
				while ((seg->seg_Node.mln_Succ) && (!(SetSignal(0,0) & SIGBREAKF_CTRL_C)))
				{
					tmp[0]=(ULONG)seg->seg_Name;
					VPrintf("\nModule '%s'\n",(LONG *)tmp);
					tmp[0]=0;
					while (tmp[1]=seg->seg_Array[tmp[0]].seg_Address)
					{
						tmp[2]=seg->seg_Array[tmp[0]].seg_Size;
						VPrintf("\tHunk%3ld @ $%08lx Size $%08lx\n",(LONG *)tmp);
						tmp[0]++;
					}

					seg=(struct SegNode *)(seg->seg_Node.mln_Succ);
				}
			}

			if (opts[OPT_FIND]) for (hex=(char **)opts[OPT_FIND];(*hex);hex++)
			{
			char	*p;
			ULONG	val;
			ULONG	tmp[4];
			ULONG	c;

				val=0;
				p=*hex;
				if (*p=='$') p++;	/* Support $hex */
				while (*p)
				{
					c=(ULONG)*p;
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
						p++;
					}
					else
					{
						val=0;
						p=&p[strlen(p)];
					}
				}

				if (p=(*segSem->seg_Find)(tmp[0]=val,&tmp[2],&tmp[3]))
				{
				char Buffer[200];

					c=0;
					while (c<200) if (!(Buffer[c++]=*p++)) c=200;
					tmp[1]=(ULONG)Buffer;
					VPrintf("$%08lx - %s : Hunk %ld, Offset $%08lx",(LONG *)tmp);

					(*segSem->seg_Find)(val,&tmp[0],&tmp[0]);
					if ((tmp[0]!=tmp[2]) && (tmp[0]!=tmp[3]))
					{
						VPrintf(", SegList $%08lx",(LONG *)tmp);
					}

					PutStr("\n");
				}
				else VPrintf("$%08lx - Not found\n",(LONG *)tmp);
			}
			Permit();

			if (CheckSignal(SIGBREAKF_CTRL_C)) PrintFault(ERROR_BREAK,NULL);
		}

		if (rdargs) FreeArgs(rdargs);

		/*
		 * If we are the running version, install patches here...
		 * ...and disconnnect from the CLI...
		 */
		if (segSem == &MySem)
		{
			/* Install the SetFunction here */
			Forbid();
			OldLoadSeg   =(ULONG)SetFunction(DOSBase,(long)&LVOLoadSeg,(unsigned long (*)())MyLoadSeg);
			OldNewLoadSeg=(ULONG)SetFunction(DOSBase,(long)&LVONewLoadSeg,(unsigned long (*)())MyNewLoadSeg);
			OldUnLoadSeg =(ULONG)SetFunction(DOSBase,(long)&LVOUnLoadSeg,(unsigned long (*)())MyUnLoadSeg);
		//	OldRemSegment=(ULONG)SetFunction(DOSBase,(long)&LVORemSegment,(unsigned long (*)())MyRemSegment);
			Permit();

			/*
			 * Now disconnect...
			 * WB:  means clear the msg->sm_Segment BPTR...
			 * CLI: means clear the CLI->Module BPTR...
			 */
			if (msg) msg->sm_Segment=NULL;
			else ((struct CommandLineInterface *)BADDR(((struct Process *)FindTask(NULL))->pr_CLI))->cli_Module=NULL;
		}

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

/*
 * Track the loaded segment...  This will add a SegNode to
 * the semaphore for later work...
 */
VOID __asm TrackSeg(register __d0 BPTR seg,register __a0 char *Name)
{
struct	SegNode	*node;
	ULONG	*p;
	ULONG	count;

	/* Track the segment... */
	if ((seg) && (Name))
	{
		count=sizeof(struct SegArray);
		p=BADDR(seg);
		while (*p)
		{
			count+=sizeof(struct SegArray);
			p=BADDR(*p);
		}

		if (node=AllocVec(sizeof(struct SegNode)+count+1+strlen(Name),MEMF_PUBLIC|MEMF_CLEAR))
		{
			node->seg_Name=(char *)(((ULONG)node)+sizeof(struct SegNode)+count);
			strcpy(node->seg_Name,Name);
			count=0;
			p=(ULONG *)seg;
			while (p=BADDR(p))
			{
				node->seg_Array[count].seg_Address=(ULONG)p;
				node->seg_Array[count].seg_Size=p[-1]-4;
				p=(ULONG *)p[0];
				count++;
			}

			Forbid();
			AddHead((struct List *)&MySem.seg_List,(struct Node *)node);
			Permit();
		}
	}
}

/*
 * Untrack the segment that is about to be
 * unloaded.  Note that it handles partial unload
 * by looking for individual segments and
 * only when all of them have been removed
 * will it removed the tracking node.
 */
VOID __asm UnTrackSeg(register __d1 BPTR seg)
{
	ULONG		*p;
	ULONG		flag;
struct	SegArray	*array;
struct	SegNode		*node;
struct	SegNode		*tmp;

	p=(ULONG *)seg;
	while (p=BADDR(p))
	{
		Forbid();
		node=(struct SegNode *)MySem.seg_List.mlh_Head;
		while (node->seg_Node.mln_Succ)
		{
			flag=0;
			array=node->seg_Array;
			while (array->seg_Address)
			{
				if (array->seg_Address==(ULONG)p) array->seg_Size=0;
				flag |= array->seg_Size;
				array++;
			}

			tmp=(struct SegNode *)node->seg_Node.mln_Succ;

			if (!flag)
			{
				/* Segment all gone... */
				Remove((struct Node *)node);
				FreeVec(node);
			}

			node=tmp;
		}
		Permit();

		p=(ULONG *)p[0];
	}
}

/*
 * Find the segment that this address is connected to...
 */
char * __asm FindSeg(register __a0 ULONG Address,register __a1 ULONG *SegNum,register __a2 ULONG *Offset)
{
struct	SegNode	*node;
	char	*Name=NULL;
	ULONG	count;

	node=(struct SegNode *)MySem.seg_List.mlh_Head;
	while ((node->seg_Node.mln_Succ) && (!Name))
	{
		count=0;
		while (node->seg_Array[count].seg_Address)
		{
			if ((Address > node->seg_Array[count].seg_Address) &&
			    (Address < (node->seg_Array[count].seg_Address+node->seg_Array[count].seg_Size)))
			{
				*SegNum=count;
				*Offset=Address-node->seg_Array[count].seg_Address-4;
				Name=node->seg_Name;

				/*
				 * Kludge feature:  If the SegNum and Offset
				 * point to the same longword, return the
				 * SegList pointer insted of the hunk/offset
				 * pair.  This is for CPR in SAS/C++ 6.50
				 */
				if (SegNum==Offset) *SegNum=node->seg_Array[0].seg_Address;
			}
			count++;
		}
		node=(struct SegNode *)node->seg_Node.mln_Succ;
	}
	return(Name);
}

/*
 * Add the ROM modules to the tracking list...
 *
 * What we will do is search the ROM for ROMTAGs and
 * add them to the list as found...
 */
VOID AddROM(VOID)
{
struct	SegNode	*last=0;
	UWORD	*ROM;
	ULONG	index;

	/* Ok, lets find the ROM start address... */
	ROM=(UWORD *)(0xFFF80000 & ((ULONG)(EXECBASE->LibNode.lib_Node.ln_Name)));

	/* Ok, no start the search... */
	for (index=0;index<(512*1024/2);index++)
	{
		if (ROM[index]==RTC_MATCHWORD)
		{
		struct	Resident	*romTag=(struct Resident *)(&ROM[index]);

			if (romTag==romTag->rt_MatchTag)
			{
			char	*from;

				index=(((ULONG)(romTag->rt_EndSkip))-((ULONG)ROM))/2-1;
				from=romTag->rt_IdString;

				/*
				 * This simple check makes sure that there
				 * is a version/date on the module.  If not,
				 * it is assumed to be a "special" one.
				 */
				while ((*from) && (*from!='(')) from++;
				if (*from)
				{
				char	*new;

					from=romTag->rt_IdString;

					if (last) last->seg_Array[0].seg_Size=((ULONG)romTag)-last->seg_Array[0].seg_Address;

					last=AllocVec(sizeof(struct SegNode)+sizeof(struct SegArray)+strlen(from)+1+6,MEMF_CLEAR|MEMF_PUBLIC);
					if (last)
					{
						last->seg_Array[0].seg_Address=(ULONG)romTag;
						last->seg_Name=(char *)&(last->seg_Array[2]);

						new=last->seg_Name;
						strcpy(new,"ROM - ");
						while (*new) new++;

						while (*from)
						{
							if (*from>=' ') *new++=*from;
							from++;
						}

						/* Add to the list... */
						AddTail((struct List *)&MySem.seg_List,(struct Node *)last);
					}
				}
			}
		}
	}
	if (last) last->seg_Array[0].seg_Size=((ULONG)(&ROM[index]))-last->seg_Array[0].seg_Address;
}

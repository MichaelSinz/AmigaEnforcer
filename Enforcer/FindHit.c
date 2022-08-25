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
******* FindHit ***************************************************************
*
*   NAME
*	FindHit - A tool that can locate the source file and line number
*	          that a SegTracker report happened at.
*
*   SYNOPSIS
*	FindHit will read the executable file and if there is debugging
*	information in it, will try to locate the source file and line
*	number that correspond to the Enforcer hit HUNK/OFFSET.
*
*   FUNCTION
*	FindHit uses the Lattice/SAS/MetaScope standard 'LINE'
*	debug hunk to locate the closest line to the hunk/offset given.
*	Note that this can only happen if the executable has the
*	LINE debugging turned on.  (The LawBreaker program has this
*	such that you can test this yourself.)
*
*	In SAS/C 6.x, you need to compile with DEBUG=LINE or better
*	and do not use the link option of NODEBUG.
*
*	In SAS/C 5.x, you need to compile with -d1 or better.
*	Note that FindHit works with the old SAS/C 5.x 'SRC '
*	debugging information too.  This is required for -d2 or
*	higher debugging support.  However, I do not have 'SRC ' hunk
*	documentation and thus FindHit may be very specific to the
*	SAS/C 5.x version of this hunk.
*
*	In DICE (2.07 registered being the one I tried) the -d1
*	debug switch also supports the 'LINE' debug hunk and
*	works with FindHit.
*
*	In HX68 and CAPE, you need to add the DEBUG directive to
*	the assembly code program.  (See LawBreaker source)
*
*	For other languages, or other versions of the above, please
*	see the documentation that comes with the language.
*
*   INPUTS
*	FILE/A      - The executable file, with debugging information.
*
*	OFFSETS/A/M - The HEX offset (with or without leading $)
*	              If a hunk number other than the default
*	              is needed, it is expressed as hunk:offset.
*	              The default hunk is that of the last argument
*	              or hunk 0 if no hunk number has been given.
*	              For example:  12 $22 $3:12 22 4:$12 32 $0:$32
*	              will find information for:
*	              hunk $0, offset $12
*	              hunk $0, offset $22
*	              hunk $3, offset $12
*	              hunk $3, offset $22
*	              hunk $4, offset $12
*	              hunk $4, offset $32
*	              hunk $0, offset $32
*
*   EXAMPLE
*	FindHit FooBar $0342 $1:4F2 3:$1A 2C
*	badcode.c : Line 184
*	No line number information for Hunk $1, Offset $4F2
*	badcode2.c : Line 12
*	badcode2.c : Line 14
*
*	See the Enforcer documentation about issues dealing with the
*	exact location of the Enforcer hit.  The line given may
*	not be exactly where the hit happened.
*
*	The way I use this is to always have line debugging turned on
*	when I compile.  This does not change the quality of the code
*	and takes only a small amount of extra disk space.  However,
*	what I do is to link the program twice:  Once to a file called
*	program.ld which contains all of the debugging information.
*	Then, I link program.ld to program, stripping debug information.
*	The command line for SLINK or BLINK is as follows:
*
*	    BLINK program.ld TO program NODEBUG
*
*	I keep both of these on hand; with program being the one I
*	distribute and use.  When a hit happens, I can just use program.ld
*	with FindHit to get the line number and source file that it happened
*	in.  This way you can distribute your software without the debugging
*	information and still be able to use FindHit on the actual code.
*	(After all, that link command does nothing but strip symbol and
*	debug hunks)
*
*   NOTES
*	Note that this program does nothing when run from the Workbench
*	and thus does not have an icon.
*
*   SEE ALSO
*	"Quantum Physics:  The Dreams that Stuff is made of." - Michael Sinz
*
*   BUGS
*
*******************************************************************************
*/

/******

Line number debug information:

Looks like it is a debug hunk which look as follows:

0x000003F1
<size in long words>
<offset relative to current hunk>
0x4C494E45	'LINE'
<number of long words in the filename>
<file name, not NULL terminated if exactly the right length>
<line number>	<mask with 0x00FFFFFF since the upper byte is special>
<offset>
<line number>	<mask with 0x00FFFFFF since the upper byte is special>
<offset>
...

******/

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/tasks.h>
#include <exec/memory.h>
#include <exec/alerts.h>

#include <string.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>

#include <utility/tagitem.h>
#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>

#include <dos/dosextens.h>
#include <dos/datetime.h>
#include <dos/dosasl.h>
#include <dos/rdargs.h>

#define DOSLIB	   "dos.library"
#define DOSVER	   37L

/******************************************************************************/

#include	<exec/ports.h>
#include	<exec/libraries.h>
#include	<exec/semaphores.h>
#include	<exec/resident.h>

#include	<dos/doshunks.h>
#include	<dos/dos.h>
#include	<dos/dosextens.h>
#include	<dos/stdio.h>

#include	"findhit_rev.h"

/******************************************************************************/

#define	CHECKEOF(x)	if (((ULONG)&bufferpos[x]) > ((ULONG)bufferend)) { rc=ERROR_FILE_NOT_OBJECT; goto ReadError; }
#define	GETLONG	getLong=*bufferpos++; CHECKEOF(0)

#define	MASK_LINE	(0x00FFFFFF)

/*
 * This command finds the fine/line number of a SgeTracker hit...
 */

/* This is the command template. */
#define TEMPLATE  "FILE/A,OFFSETS/A/M" VERSTAG

#define	OPT_FILE	0
#define	OPT_OFFSETS	1
#define	OPT_COUNT	2

LONG cmd(void)
{
struct	Library	*SysBase = (*((struct Library **) 4));
struct	Library	*DOSBase;
struct	Process	*proc;
struct	Message	*msg=NULL;
	LONG	rc=RETURN_FAIL;

	proc=(struct Process *)FindTask(NULL);
	if (!(proc->pr_CLI))
	{
		WaitPort(&(proc->pr_MsgPort));
		msg=GetMsg(&(proc->pr_MsgPort));
	}
	else if (DOSBase = OpenLibrary(DOSLIB,DOSVER))
	{
	struct	RDArgs	*rdargs;
		LONG	opts[OPT_COUNT];

		memset((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, opts, NULL);

		if (rdargs == NULL) PrintFault(IoErr(),NULL);
		else
		{
		BPTR	fh;

			if (fh=Open((char *)opts[OPT_FILE],MODE_OLDFILE))
			{
			ULONG	getLong;
			ULONG	*buffer;
			ULONG	buffersize;

				/*
				 * Ok, so now we need to do the hard stuff
				 * for each file...
				 */
				rc=RETURN_OK;

				Seek(fh,0,OFFSET_END);
				buffersize=Seek(fh,0,OFFSET_BEGINNING);
				if (buffer=AllocVec(buffersize,MEMF_PUBLIC))
				{
					if (buffersize==Read(fh,buffer,buffersize))
					{
					ULONG	hitHunk=0;
					char	**hex;

						for (hex=(char **)opts[OPT_OFFSETS];(*hex);hex++)
						{
						ULONG	*bufferpos;
						ULONG	*bufferend;
						ULONG	tableSize;
						ULONG	firstHunk;
						ULONG	lastHunk;
						ULONG	hunk;
						ULONG	hunkType;
						ULONG	hitOffset;
						ULONG	foundOffset;
						ULONG	foundName;
						ULONG	foundLine;
						char	*p;
						ULONG	c;

							hitOffset=0;
							p=*hex;
							if (*p=='$') p++;	/* Support $hex */
							while (c=(ULONG)*p)
							{
								if (c==':')
								{
									/*
									 * So this one had a hunk too, so change the hitHunk
									 */
									hitHunk=hitOffset;
									hitOffset=0;
									p++;
									if (*p=='$') p++;	/* Support $hex */
								}
								else
								{
									if ((c>='a') && (c<='f')) c-=32;
									c-='0';
									if (c>9)
									{
										c-=7;
										if (c<10) c=16;
									}

									if (c<16)
									{
										hitOffset=(hitOffset << 4) + c;
										p++;
									}
									else
									{
										hitOffset=0;
										p=&p[strlen(p)];
									}
								}
							}

							/* Now look for the hunk/offset */
							foundOffset=0x7FFFFFFF;
							foundName=0;
							foundLine=0;

							bufferpos=buffer;
							bufferend=(ULONG *)(((ULONG)buffer)+buffersize);

							GETLONG;	/* Check for hunk header... */
							if (getLong != HUNK_HEADER) { rc=ERROR_FILE_NOT_OBJECT; goto ReadError; }

							GETLONG;	/* Check for resident lib */
							if (getLong != 0) { rc=ERROR_FILE_NOT_OBJECT; goto ReadError; }

							GETLONG;	/* Get table size */
							tableSize=getLong;

							GETLONG;	/* Get first hunk slot */
							firstHunk=getLong;

							GETLONG;	/* Last hunk slot */
							lastHunk=getLong;

							CHECKEOF(tableSize);
							bufferpos=&bufferpos[tableSize];

							hunk=firstHunk;
							while ((!rc)&&(hunk<=lastHunk))
							{
								if (bufferpos != bufferend)
								{
									GETLONG;
									hunkType=getLong & 0x3FFFFFFF;

									switch (hunkType)
									{
									case	HUNK_DEBUG:
										if (hunk==hitHunk)
										{
										ULONG	debugSize;
										ULONG	baseOffset;
										ULONG	checkName;
										ULONG	look=0;

											debugSize=bufferpos[0];
											CHECKEOF(debugSize);
											baseOffset=bufferpos[1];
											if (bufferpos[2]=='LINE')
											{
												/*
												 * Name of source file
												 * We play fast and loose
												 * here with the fact that we
												 * will be setting to 0 the upper
												 * byte of the longword after the
												 * file name (for other reasons) and
												 * thus the file name will be NULL
												 * terminated in all cases.
												 */
												checkName=(ULONG) &bufferpos[4];

												/* Skip source file name, get to lines */
												look=4+bufferpos[3];

												/*
												 * Kludge to work around a SAS/C 6.50+ strangeness...
												 * We ignore LINE debug stuff with no offset and no
												 * line offsets in the first two places.
												 */
												if ((!baseOffset) && (!bufferpos[look+1]) && (!bufferpos[look+3])) look=0;
											}
											else if ((bufferpos[2]=='SRC ') && (debugSize>16))
											{
												/*
												 * Silly SAS/C 5.x did some strange stuff
												 * It sometimes used the 'LINE' stuff and sometimes
												 * it used the 'SRC ' stuff...
												 */
												checkName=(ULONG) &bufferpos[15];

												/* Skip source file name, get to lines */
												look=15 + ((bufferpos[14]+3) >> 2);
												if (look==15) look=0;
											}

											if ((look) && (baseOffset<=hitOffset))
											{
												baseOffset=hitOffset-baseOffset;

												while ((look<debugSize) && (bufferpos[look]&=MASK_LINE))
												{
												ULONG	junk=baseOffset-bufferpos[look+1];

													if (junk < foundOffset)
													{
														foundOffset=junk;
														foundLine=bufferpos[look];
														foundName=checkName;
													}
													look+=2;
												}
											}
										}
										/* Fall down into the skip code */

									case	HUNK_CODE:
									case	HUNK_DATA:
									case	HUNK_NAME:
										GETLONG;
										CHECKEOF(getLong);
										bufferpos=&bufferpos[getLong];
										break;

									case	HUNK_RELOC32:
									case	HUNK_SYMBOL:
										while (getLong)
										{
											GETLONG;
											if (getLong)
											{
												getLong++;
												CHECKEOF(getLong);
												bufferpos=&bufferpos[getLong];
											}
										}
										break;

									case	HUNK_END:
										hunk++;
										break;

									case	HUNK_BSS:
										bufferpos++; CHECKEOF(0);
										break;

									default:	rc=ERROR_OBJECT_WRONG_TYPE; goto ReadError;
									}
								}
							}

							if (foundName && foundLine)
							{
							ULONG	tmp[2];

								tmp[0]=foundName;
								tmp[1]=foundLine;
								VPrintf("%s : Line %ld\n",(LONG *)tmp);
							}
							else
							{
							ULONG	tmp[2];

								tmp[0]=hitHunk;
								tmp[1]=hitOffset;
								VPrintf("No line number information for Hunk $%lx, Offset $%lx.\n",(LONG *)tmp);
							}
						}
					}
					else rc=IoErr();

ReadError:
					FreeVec(buffer);
				}
				else rc=IoErr();
				Close(fh);

			}
			else rc=IoErr();

			if (rc)
			{
				PrintFault(rc,NULL);
				rc=RETURN_FAIL;
			}

		FreeArgs(rdargs);
		}

		CloseLibrary(DOSBase);
	}
	else if (DOSBase=OpenLibrary(DOSLIB,0))
	{
		Write(Output(),"Requires Kickstart 2.04 (37.175) or later.\n",43);
		CloseLibrary(DOSBase);
	}

	if (msg)
	{
		Forbid();
		ReplyMsg(msg);
	}
	return(rc);
}

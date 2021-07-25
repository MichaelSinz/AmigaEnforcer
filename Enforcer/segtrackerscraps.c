#define TEMPLATE	"SHOW/S,DUMP/S,SCAN/S,FIND/M" VERSTAG

[...]

static void TrackResModules(APTR modules, STRPTR type, struct SegSem *segSem);
static void TrackDOSList(struct SegSem *segSem);
static void TrackProcesses(struct SegSem *segSem, struct List *tasks);

[...]

			if (!(segSem=(struct SegSem *)FindSemaphore(SEG_SEM)))
			{
[...]

                                /* Setup means scanning of Kickstags and DosList! */
                                opts[OPT_SCAN] = TRUE;
			} /* if */

                        if(opts[OPT_SCAN])
                        {
                            TrackResModules(SysBase->ResModules, "ROM", segSem);
                            TrackResModules(SysBase->KickTagPtr, "TAG", segSem);
                            Forbid();
                            TrackProcesses(segSem, &SysBase->TaskWait);
                            TrackProcesses(segSem, &SysBase->TaskReady);
                            Permit();
                            TrackDOSList(segSem);
                        } /* if */

[...]

VOID TrackSegSem(BPTR seg, char *Name, struct SegSem *segSem)
{
	ULONG	*p;
	ULONG	count;
struct	SegNode	*node;

	/* Track the segment... */
	if ((seg) && (Name))
	{
		count=sizeof(struct SegArray);
		p=BADDR(seg);
		while (*p)
		{
                        // We don't track any obvious garbage!
                        if((p[-1] < 16) || (p[-1] > 0x01000000))
                        {
                            return;
                        } /* if */
			count+=sizeof(struct SegArray);
			p=BADDR(*p);
		}

[...]

/*------------------------------------------------------------------------*/
static void TrackMemory(APTR mem, ULONG size, STRPTR Name, struct SegSem *segSem)
{
    ULONG   count;
struct  SegNode *node;

    /* Track the segment... */
    if ((size) && (Name))
    {
        count=sizeof(struct SegArray);

        if (node=AllocVec(sizeof(struct SegNode)+count+1+strlen(Name),MEMF_PUBLIC|MEMF_CLEAR))
        {
            node->seg_Name=(char *)(((ULONG)node)+sizeof(struct SegNode)+count);
            strcpy(node->seg_Name,Name);
            count=0;
            node->seg_Array[count].seg_Address=(ULONG)mem;
            node->seg_Array[count].seg_Size=size;

            Forbid();
            AddHead((struct List *)&segSem->seg_List,(struct Node *)node);
            Permit();
        } /* if */
    } /* if */

} /* TrackMemory */

/*------------------------------------------------------------------------*/
static void TrackResModules(APTR modules, STRPTR type, struct SegSem *segSem)
{
    UBYTE buf[80];
    ULONG *p = modules, dummy;
    UBYTE *b;

    while(p && *p)
    {
        if((*p) & (1 << 31))
        {
            /* Next table */
            p = (ULONG *)((*p) & ~(1 << 31));
        }
        else
        {
            struct Resident *res = (struct Resident *)(*p);

            Forbid();
            if(!FindSegSem((ULONG)res, &dummy, &dummy, segSem))
            {
                strcpy(buf, type);
                strcpy(buf + strlen(buf), " - ");
                strcpy(buf + strlen(buf), res->rt_IdString);
                b = buf + strlen(buf) - 1;
                while(b >= buf && (b[0] < 0x20))
                {
                    *b-- = 0;
                } /* while */
                TrackMemory(res, (ULONG)(res->rt_EndSkip) - (ULONG)res, buf, segSem);
            } /* if */
            Permit();

            p++;
        } /* if */
    } /* while */

} /* TrackResModules */

/*------------------------------------------------------------------------*/
static void TrackDOSList(struct SegSem *segSem)
{
    UBYTE buf[80];
    struct DosList *dol;
    ULONG dummy;
    APTR p;

    dol = LockDosList(LDF_READ|LDF_DEVICES);
    while((dol = NextDosEntry(dol,LDF_READ|LDF_DEVICES)) != NULL)
    {
        BPTR seg = dol->dol_misc.dol_handler.dol_SegList;

        if(seg && !dol->dol_Task)
        {
            p = BADDR(seg);

            /* We don't track garbage segments like CON or RAW! */
            if(((LONG *)p)[-1] > 4)
            {
                Forbid();
                if(!FindSegSem((ULONG)p + 4, &dummy, &dummy, segSem))
                {
                    strcpy(buf, "DOS - ");
                    strcpy(buf + strlen(buf), (char *)BADDR(dol->dol_Name) + 1);
                    TrackSegSem(seg, buf, segSem);
                } /* if */
                Permit();
            } /* if */
        } /* if */
    } /* while */

    UnLockDosList(LDF_READ|LDF_DEVICES);

} /* TrackDOSList */

/*------------------------------------------------------------------------*/
static void TrackProcesses(struct SegSem *segSem, struct List *tasks)
{
    UBYTE buf[80], buf2[80];
    struct Process *pr, *nxtpr;
    ULONG dummy;
    APTR p;

    Forbid();
    for(pr = (struct Process *)tasks->lh_Head;
        nxtpr = (struct Process *)pr->pr_Task.tc_Node.ln_Succ;
        pr = nxtpr)
    {
        if(pr->pr_Task.tc_Node.ln_Type == NT_PROCESS)
        {
            STRPTR name;
            BPTR seg;

            if(!pr->pr_TaskNum)
            {
                seg = ((BPTR *)BADDR(pr->pr_SegList))[3];
                name = pr->pr_Task.tc_Node.ln_Name;
            }
            else
            {
                seg = ((struct CommandLineInterface *)BADDR(pr->pr_CLI))->cli_Module;
                if(seg)
                {
                    name = BADDR(((struct CommandLineInterface *)BADDR(pr->pr_CLI))->cli_CommandName);
                    CopyMem(&name[1], buf2, name[0]);
                    buf2[name[0]] = 0;
                    name = buf2;
                } /* if */
            } /* if */

            if(seg)
            {
                p = BADDR(seg);

                /* We don't track garbage segments like CON or RAW! */
                if((((LONG *)p)[-1] > 16) && (((LONG *)p)[-1] < 0x01000000))
                {
                    Forbid();
                    if(!FindSegSem((ULONG)p + 4, &dummy, &dummy, segSem))
                    {
                        strcpy(buf, "PRC - ");
                        strcpy(buf + strlen(buf), name);
                        TrackSegSem(seg, buf, segSem);
                    } /* if */
                    Permit();
                } /* if */
            } /* if */
        } /* if */
    } /* for */
    Permit();

} /* TrackProcesses */

/*------------------------------------------------------------------------*/


/*  vm.c -- virtual machine management  */

#include "rts.h"

static Mutex cre_mutex;



/*
 *  Initialize virtual machine mutex.
 */
void
sr_init_vm ()
{
    INIT_LOCK (cre_mutex, "cre_mutex");
}



/*
 *  Specify location of physical machine.
 */
void
sr_locate (locn, n, host, exe)
char *locn;
int n;
String *host, *exe;
{
    struct locn_st lr;
    Pach ph;
    int size;
    static String nullstring = { STRING_OVH, 0 };

    sr_check_stk (CUR_STACK);
    if (!exe)
	exe = &nullstring;

    sr_init_net ((Ptr) NULL);		/* start up the network, if not up */
    size = (lr.text + host->length + 1 + exe->length + 1) - (char *) &lr;
    if (size > sizeof (lr))
	sr_loc_abort (locn, "strings too long in locate call");
    lr.num = n;
    memcpy (lr.text, DATA (host), host->length);
    lr.text [host->length] = '\0';
    memcpy (lr.text + host->length + 1, DATA (exe), exe->length);
    lr.text [host->length + 1 + exe->length] = '\0';
    ph = sr_remote (SRX_VM, REQ_LOCVM, (Pach) &lr, size);
    sr_free ((Ptr) ph);			/* free reply packet */
}



/*
 *  Create new virtual machine.  Return its number.
 */
Vcap
sr_crevm (locn, physm)
char *locn;
int physm;
{
    struct num_st vr;
    Pach ph;
    Vcap v;

    sr_check_stk (CUR_STACK);
    TRACE ("CREATEV", locn, 0);
    sr_init_net ((Ptr) NULL);			/* start network, if not up */
    vr.num = physm;				/* set physical machine */
    ph = sr_remote (SRX_VM, REQ_CREVM, (Pach) &vr, sizeof (vr));
						/* call srx to create the vm */
    v = ((struct num_st *) ph) -> num;		/* save virtual machine no. */
    sr_free ((Ptr) ph);				/* free reply packet */

    DEBUG (D_SRX_ACT, "sr_crevm (%ld) returning %ld", physm, v, 0);
    return v;
}



/*
 *  Create new virtual machine by name.  Return its number.
 */
Vcap
sr_crevm_name (locn, host)
char *locn;
String *host;
{
    static int num_crevm_name = 0;	/* count of creates by name */
    int n;

    LOCK (cre_mutex, "sr_crevm_name");
    num_crevm_name++;
    n = - (sr_my_vm * 1000 + num_crevm_name);
    UNLOCK (cre_mutex, "sr_crevm_name");
    sr_locate (locn, n, host, (String *) 0);
    return sr_crevm (locn, n);
}



/*
 *  Destroy virtual machine.
 */
void
sr_destvm (locn, vm)
char *locn;
int vm;
{
    struct num_st vr;
    Pach ph;

    CUR_PROC->locn = locn;		/* add locn to proc structure */

    TRACE ("DESTROYV", locn, 0);
    sr_check_stk (CUR_STACK);
    if (vm == NOOP_VM)
	return;
    if (vm == NULL_VM)
	sr_loc_abort (locn, "attempting to destroy null vm");

    sr_init_net ((Ptr) NULL);		/* start network if not up */
    vr.num = vm;			/* set virtual machine number */
    vr.ph.priority = CUR_PROC->priority;
    ph = sr_remote (SRX_VM, REQ_DESTVM, (Pach) &vr, sizeof (vr));
					/* ask srx to destroy the vm */
    sr_free ((Ptr) ph);			/* free reply packet */
}

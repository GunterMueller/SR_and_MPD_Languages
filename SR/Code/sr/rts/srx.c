/*  srx.c -- SR execution manager
 *
 *  Srx is initiated by an SR program when it first has a need.  Params are:
 *	argv[1]	    a magic string to validate the call
 *	argv[2]	    protocol version number for sanity check with caller
 *	argv[3]	    caller's path (caller's argv[0])
 *	argv[4]	    the number of job servers to create on remote vms
 *	argv[5]	    file descriptor number for tracing, or -1 if none 
 *
 *  The address of srx's listener socket is output to stdout, on the
 *  assumption that it is a pipe to initiating process (main machine).
 *
 *  Srx runs as a single process with no parallelism.
 *  It just loops responding to input messages.
 */


#include "../paths.h"
#include "rts.h"
#include <fcntl.h>
#include <signal.h>
#include <sys/param.h>

#ifdef __PARAGON__
#include <nx.h>
extern local_message_type;	/* message type == ptype of current VM */
#endif

char	version[] = VERSION;		/* SR version number */



/*  physical machine data -- default entry is head of list  */

struct pmdata {
    int num;			/* physical machine number */
    char *hostname;		/* name of host on which to create machine */
    char *exepath;		/* path to executable program on that host */
    struct pmdata *next;	/* next data node */
};

struct pmdata physm;



/*  virtual machine data  */

typedef enum { STARTING, WORKING, DYING, GONE } VMstate;

struct vmdata {		/* virtual machine data */
    int phys;			/* physical machine number */
    int pid;			/* process id */
    VMstate state;		/* current state */
    char addr[SOCK_ADDR_SIZE];	/* socket address */
    int notify;			/* machine to notify on birth or death */
    Remd rem;			/* request message pointer for acking CREVM */
    int nmsgs [1 + MAX_VM];	/* last report of messages in & out */
};

struct vmdata *vm [1 + MAX_VM];



/*  miscellaneous globals  */

char sr_net_exe_path[MAX_PATH];	/* network path of exe file */

Vcap sr_my_vm = SRX_VM;		/* VM number for sr_net_send */
int nvm = 0;			/* number of VMs started */
int ndied = 0;			/* number that have died */
int exiting = 0;		/* shutdown in progress? */

char *trc_arg;			/* trace argument */
int trc_fd;			/* trace fd */

char my_addr[SOCK_ADDR_SIZE];	/* address of srx socket */

struct idle_st sr_msg_counts;	/* messages in & out, updated by socket.c */



/*  global scratch area used for all packet I/O  */

union {
    struct pach_st header;
    struct saddr_st sock;
    struct num_st npkt;
    struct exit_st xpkt;
    struct locn_st locn;
    struct idle_st idle;
} packet;

#define PH (&packet.header)
#define ORIGIN (packet.header.origin)



/*  function declarations */

extern char *netpath ();

extern void sr_init_debug ();
extern void sr_net_start ();
extern void sr_net_more (), sr_net_send ();
extern enum ms_type sr_net_recv ();

struct pmdata *lookup ();
struct vmdata *alcvm ();
char *alloc (), *salloc ();
void callme (), locvm (), crevm (), findvm (), hello ();
void destvm (), ackdest (), exitmsg (), eof (), stopmsg (), idlemsg ();
void exe (), mort (), setloc ();
char *getname ();


static char jsbuf[10];

/*  main program  */

main (argc, argv)
int argc;
char *argv[];
{
    int i;
    char *p;
    struct vmdata *v;
    char cwd[MAX_PATH], mapfile[MAX_PATH];
    char message[100];

    /* ensure that stderr is unbuffered */
    setbuf (stderr, (char *) NULL);

    /* init debugging */
#ifndef __PARAGON__
    sr_init_debug ((char *) NULL);
#else
    sr_init_debug (argv[6]);	/* passed via argv on Paragon */
    local_message_type = 1;	/* local_message_type of SRX is 1 */
#endif

    DEBUG5 (D_GENERAL | D_SRX_ACT, "srx %s %s %s %s (pid %ld)",
	(argc > 1) ? argv[1] : "--",
	(argc > 2) ? argv[2] : "--",
	(argc > 3) ? argv[3] : "--",
	(argc > 4) ? argv[4] : "--",
	getpid ());

    /* check for valid call */
    if (argc < 6 || strcmp (argv[1], VM_MAGIC) != 0)
	sr_net_abort ("invalid call to SRX");

    strcpy (jsbuf, argv[4]);	/* save jobserver arg for exe () */

#ifndef __PARAGON__ 
    trc_arg = argv[5];		/* save trace arg */
    sscanf (trc_arg, "%d", &trc_fd);
#endif

    if (strcmp (argv[2], PROTO_VER) != 0) {
	DEBUG (D_SOCKET, "protocol argv[2] %s, but PROTO_VER %s",
	    argv[2], PROTO_VER, 0);
	sr_net_abort ("protocol version mismatch; rerun srl to fix");
    }

    /* save path of executable */
    physm.exepath = salloc (argv[3]);

#ifndef __PARAGON__	/* all binaries are already spawned on Paragon */
    /* build network path of executable */
    if (p = getenv (ENV_NETMAP))
	strcpy (mapfile, p);
    else
	sprintf (mapfile, "%s/%s", SRLIB, "srmap");

    getcwd (cwd, MAX_PATH);

    if (!netpath (argv[3], cwd, mapfile, sr_net_exe_path))
	sr_net_abort ("can't build network path for executable");

    DEBUG (D_SRX_ACT, "netpath is: %s", sr_net_exe_path, 0, 0);
#endif

    /* enter caller in vm table */
    v = alcvm ();
    v->pid = getppid ();
    v->state = STARTING;

    /* close all files except stdin, stdout, stderr, and trace file */
    for (i = 3; i < NOFILE; i++)
	if (i != trc_fd)
	    close (i);

    /* start network I/O and send address to caller */
    sr_net_start (my_addr);
#ifndef __PARAGON__		/* Paragon has no pipe to VM 0 */
    write (0, my_addr, strlen (my_addr));  /* fd 0 by agreement with net.c */
#endif

    /* connect stdin to /dev/null; then will have
     *	0. stdin  /dev/null
     *  1. stdout as inherited from caller
     *  2. stderr as inherited from caller
     *  3. socket for listening
     *  4+ available for connections to VMs
     */
    close (0);
    if (open ("/dev/null", O_RDONLY) < 0)
	sr_net_abort ("can't open /dev/null");

#ifndef __PARAGON__  
    /* set up interrupt routine to catch deaths of children */
    /* not used on Paragon: they're not SRX's children */
    signal (SIGCHLD, mort);
#endif

    /* now just loop, waiting for things to do... */
    for (;;)  {

	sr_net_recv (PH);				/* read packet header */
	if (PH->size > sizeof (packet))
	    sr_net_abort ("incoming packet too big");
	sr_net_more (PH);				/* read the rest */

	switch (PH->type) {

	    case MSG_SEOF:	eof ();		break;
	    case MSG_HELLO:	hello ();	break;
	    case MSG_EXIT:	exitmsg ();	break;
	    case MSG_STOP:	stopmsg ();	break;
	    case MSG_IDLE:	idlemsg ();	break;
	    case REQ_CALLME:	callme ();	break;
	    case REQ_CREVM:	crevm ();	break;
	    case REQ_FINDVM:	findvm ();	break;
	    case REQ_DESTVM:	destvm ();	break;
	    case ACK_DESTVM:	ackdest ();	break;
	    case REQ_LOCVM:	locvm ();	break;

	    default:
		sprintf (message, "unexpected packet type %d", PH->type);
		sr_net_abort (message);
	}
    }
}



/*  locvm () - specify location for virtual machine  */

void
locvm ()
{
    char *xfile = packet.locn.text + strlen (packet.locn.text) + 1;
    DEBUG (D_SRX_IN,"LOCATE %ld %s %s", packet.locn.num,packet.locn.text,xfile);
    setloc (packet.locn.num, packet.locn.text, xfile);
    sr_net_send (ORIGIN, ACK_LOCVM, PH, PACH_SZ);
}



/*  callme () - pass a "call me" message from one VM to another  */

void
callme ()
{
    int dest = packet.npkt.num;
    DEBUG (D_SRX_IN, "CALLME %ld from %ld", dest, ORIGIN, 0);
    packet.npkt.num = ORIGIN;
    sr_net_send (dest, REQ_CALLME, PH, sizeof (packet.npkt));
}



/*  crevm () - create virtual machine.  */

void
crevm ()
{
    int pid, pm;
    struct pmdata *p;
    struct vmdata *v;
    parameter vm_parameter;	/* for creating VM on Paragon */
    char message[100];

    v = alcvm ();
    pm = packet.npkt.num;
    DEBUG (D_SRX_IN, "CREVM %ld on %ld", nvm, pm, 0);

#ifndef __PARAGON__
    if ((pid = vfork ()) < 0)
	sr_net_abort ("can't vfork for new vm");
    if (pid == 0)
	exe (pm, nvm);		/* in the child, execute a.out */
    v->pid = pid;
#else
    /* 
     * Create new VM by sending a message to the node's distributor.
     */
    vm_parameter.phys_machine =  packet.npkt.num;
    vm_parameter.virt_machine = nvm;
    strcpy (vm_parameter.srx_addr, my_addr);
    p = lookup (pm);
    if (p->hostname) {
	if (sscanf (p->hostname, "%d", &pm) != 1) {
	    sprintf (message, "invalid VM location: %s", p->hostname);
	    sr_net_abort (message);
	    }
    }
    if (pm < 0 || pm >= numnodes ()) {
	sprintf (message, "invalid VM location: %d", pm);
	sr_net_abort (message);
	}
    csend (0, &vm_parameter, sizeof (vm_parameter), pm, 0);
#endif

    v->state = STARTING;
    v->notify = ORIGIN;		/* save info for acking when HELLO comes back */
    v->rem = PH->rem;
}



#ifndef __PARAGON__

/*  exe(pm,vn) - exec SR program to be virtual machine vn on phys machine pm  */

void
exe (pm, vn)
int pm, vn;
{
    struct pmdata *p;
    char pmbuf[10], vmbuf[10], dbbuf[10], magicbuf [sizeof (VM_MAGIC) + 2];
    char *path, *h, *t;

    sprintf (pmbuf, "%d", pm);
    sprintf (vmbuf, "%d", vn);
    sprintf (dbbuf, "%X", sr_dbg_flags);
    sprintf (magicbuf, "'%s'", VM_MAGIC);

    p = lookup (pm);
    if (p->exepath && *p->exepath)	/* get path to exe file */
	path = p->exepath;		/* use explicit path if one given */
    else
	path = sr_net_exe_path;		/* else use network path */

    if (pm == 0 && !p->hostname) {	/* exec locally */
	DEBUG (D_SRX_ACT, "[%ld] exec %s args...", vn, path, 0);
	execl (path, path, VM_MAGIC, pmbuf, vmbuf, my_addr, dbbuf, jsbuf,
	    trc_arg, (char *) NULL);
	perror (path);
    } else {				/* exec remotely via rsh */
	if (!p->hostname) {
	    if (! (h = getname (pm)))		/* get hostname */
		{ fprintf (stderr, "srx: unknown machine %d\n", pm); exit (1); }
	    p->hostname = salloc (h);		/* save for next time */
	}
	DEBUG (D_SRX_ACT, "[%ld] rsh %s -n exec %s args...", vn,
	    p->hostname, path);
	if (trc_fd == STDOUT || trc_fd == STDERR)
	    t = trc_arg;
	else
	    t = "-1";
	execl (RSHPATH, RSHPATH, p->hostname, "-n",
#ifdef __linux__
	    "--",			/* due to broken Linux getopt */
#endif
	    "exec", path,
	    magicbuf, pmbuf, vmbuf, my_addr, dbbuf, jsbuf, t, (char *) NULL);
	perror (RSHPATH);
    }
    sr_net_abort ("can't execute program");
}

#endif



/*  findvm () - find virtual machine.  */

void
findvm ()
{
    int n;
    char message[100];

    n = packet.npkt.num;
    DEBUG (D_SRX_IN, "FINDM %ld from %ld", n, ORIGIN, 0);
    switch (vm[n]->state) {
	case STARTING:
	    sprintf (message,"can't connect to vm %d -- not yet initialized",n);
	    sr_net_abort (message);
	case WORKING:
	    memcpy (packet.sock.addr, vm[n]->addr, SOCK_ADDR_SIZE);
	    sr_net_send (ORIGIN, ACK_FINDVM, PH, sizeof (packet.sock));
	    break;
	case DYING:
	case GONE:
	    sprintf (message,"can't connect to vm %d -- already terminated",n);
	    sr_net_abort (message);
    }
}



/*  eof () - process EOF pseudo-message indicating a vm has died  */

void
eof ()
{
    char message[100];

    struct vmdata *v = vm[ORIGIN];
    DEBUG (D_SRX_IN, "EOF from %ld", ORIGIN, 0, 0);
    if (v->state != GONE)  {
	sprintf (message, "lost connection to virtual machine %d", ORIGIN);
	sr_net_abort (message);
    }
    if (++ndied == nvm)			/* exit if all alone */
	{ DEBUG (D_SRX_ACT, "exiting because no VMs left", 0, 0, 0); exit (1); }
}



/*  hello () - process HELLO message
 *
 *  register the new virtual machine, and pass back acknowledgement to its
 *  creator (if any).
 */

void
hello ()
{
    struct vmdata *v = vm[ORIGIN];
    DEBUG (D_SRX_IN, "HELLO %ld at %s", ORIGIN, packet.sock.addr, 0);
    if (v->state != STARTING)
	sr_net_abort ("unexpected HELLO");
    strncpy (v->addr, packet.sock.addr, SOCK_ADDR_SIZE);
    v->state = WORKING;
    if (v->notify)  {
	PH->rem = v->rem;
	packet.npkt.num = ORIGIN;
	sr_net_send (v->notify, ACK_CREVM, PH, sizeof (packet.npkt));
	v->notify = 0;
    }
}



/*  destvm () - handle REQ_DESTVM message
 *
 *  make a note that a machine is being destroyed, and pass it the message.
 */

void
destvm ()
{
    int n;
    char message[100];

    n = packet.npkt.num;
    DEBUG (D_SRX_IN, "DESTVM %ld from %ld", n, ORIGIN, 0);
    if (vm[n]->state != WORKING) {
	sprintf (message, "can't destroy VM %d -- it's not now running", n);
	sr_net_abort (message);
    }
    vm[n]->state = DYING;
    vm[n]->notify = ORIGIN;
    vm[n]->rem = PH->rem;
    sr_net_send (n, REQ_DESTVM, PH, PACH_SZ);
}



/*  ackdest () - handle ACK_DESTVM message
 *
 *  mark the vm as gone and notify the original destroyer.
 *  the vm will kill itself after sending a final idle message.
 */

void
ackdest ()
{
    int n;

    n = ORIGIN;
    DEBUG (D_SRX_IN, "GOODBYE from %ld", n, 0, 0);
    vm[n]->state = GONE;
    sr_net_send (vm[n]->notify, ACK_DESTVM, PH, PACH_SZ);
}



/*  exitmsg () - process EXIT message
 *
 *  pass the EXIT message to all other virtual machines.
 *  give them a chance to die, then kill stragglers.
 */

void
exitmsg ()
{
    int i;

    DEBUG (D_SRX_IN, "EXIT %ld from %ld", packet.npkt.num, ORIGIN, 0);
    vm[ORIGIN]->state = GONE;		/* note that sender has died */
    ++ndied;
    exiting = 1;			/* flag shutdown in progress */
    /*
     *  Relay this message from the main VM to all others.  Note that the 
     *  packet already contains exit status and report-blocked-processes flag.
     */
    for (i = nvm; i > 0; i--)
	if ((vm[i]->state != GONE) && (i != MAIN_VM))
	    sr_net_send (i, MSG_EXIT, PH, sizeof (packet.xpkt));

    /*  give everybody a chance to die quietly; then kill 'em.  */
    if (ndied < nvm)
	sleep (5);
    DEBUG0 (D_SRX_ACT, "exitmsg calling sr_net_abort");
    sr_net_abort ((char *) NULL);
}



/*  stopmsg () - process implicit or explicit stop  */

void
stopmsg ()
{
    int code = packet.npkt.num;  /* copy before we overwrite the packet */
    DEBUG (D_SRX_IN, "STOP %ld from %ld", packet.npkt.num, ORIGIN, 0);
    packet.xpkt.code = code;
    packet.xpkt.report = 0;	/* don't report for a stop */
    sr_net_send (MAIN_VM, MSG_QUIT, PH, sizeof (packet.xpkt));
}



/*  idlemsg () - process idle notification from one vm
 *
 *  Idle messages are sent when a VM can make no further process.  If all VMs
 *  have sent idle messages, and the sends and receives for each VM (including
 *  those to and from srx) are equal, then no messages are in transit and the
 *  system is globally deadlocked.
 *
 *  Each VM (and srx) reports sends on a per-VM basis, and in place of sends to
 *  itself reports the *negative* of the total receive count.  Thus it is only
 *  necessary to add all of the slots for each destination and check for zero.
 *
 *  We don't actually check that each VM has sent an idle message, but if one
 *  of them hasn't the first iteration (for VM 0 = srx) will be out of balance
 *  and we'll return quickly.
 */
void
idlemsg ()
{
    int i, j, n;

    /* save the counts for future reference */
    memcpy ((Ptr) vm[ORIGIN]->nmsgs, (Ptr) packet.idle.nmsgs,
	sizeof (vm[ORIGIN]->nmsgs));

    /* check if the ledger is in balance for each VM */
    for (i = 0; i <= nvm; i++) {	/* for each destination: */
	if (i > 0 && vm[i]->state == GONE)
	    continue;			/* a destroyed VM is idle */
	n = sr_msg_counts.nmsgs[i];	/* start with srx count */
	for (j = 1; j <= nvm; j++)
	    n += vm[j]->nmsgs[i];	/* add counts of vms */
	if (n != 0 || (i > 0 && vm[i]->state == STARTING)) {
	    /* we're not really idle yet */
	    DEBUG(D_TERM,"IDLE from %ld failed: net %ld for VM %ld",ORIGIN,n,i);
	    return;
	}
    }

    /* we have global deadlock; tell the main VM to finalize */
    DEBUG (D_GENERAL | D_SRX_IN | D_TERM,
	"IDLE from vm %d found global deadlock", ORIGIN, 0, 0);
    packet.xpkt.code =  0;		/* exit code 0 for deadlock */
    packet.xpkt.report = 1;		/* report for deadlock */
    sr_net_send (MAIN_VM, MSG_QUIT, PH, sizeof (packet.xpkt));
}



#ifndef __PARAGON__

/*  mort (sig) - interrupt routine called when a child dies
 *
 *  Deaths during srx shutdown are merely counted.  If all VMs are gone we will
 *  exit here, but note that we *aren't* notified by interrupt of VM 1's death
 *  because it's our parent, not our child.
 *
 *  Deaths during VM startup mean failure of REQ_CREVM which must be acked.
 *
 *  Other deaths are ignored and will be caught by EOF processing after
 *  the input pipe is flushed.
 */

/*ARGSUSED*/
void
mort (sig)
{
    int i, n, s, code;
    char buf[10];

    n = wait (&s);
    for (i = 1; i <= nvm; i++)
	if (vm[i]->pid == n)
	    break;
    if (i > nvm)
	sr_net_abort ("unknown pid returned by wait ())");
    sig = s & 0x7F;
    code = s >> 8;

    if (sig != 0 && sig != SIGINT && sig != SIGQUIT && sig != SIGTERM)  {
	sprintf (buf, "vm %d", i);
	psignal ((unsigned int) sig, buf);
    } else
	DEBUG (D_SRX_ACT,"vm %ld exited with signal %ld, code %ld",i,sig,code);

    /*  Re-enable the signal.  This is needed under Sys V and derivatives. */
    signal (SIGCHLD, mort);

    if (!exiting && vm[i]->state != STARTING)
	return;				/* ignore, handle when EOF seen */

    vm[i]->state = GONE;		/* show vm as down */
    if (++ndied == nvm)			/* exit if all alone */
	{ DEBUG (D_SRX_ACT, "exiting because no VMs left", 0, 0, 0); exit (1); }
    if (exiting)
	return;				/* no further action if shutting down */

    /* if we get here we need to NAK a VM startup */
    PH->rem = vm[i]->rem;		/* init caller's reply address */
    packet.npkt.num = NULL_VM;		/* indicate failure */
    sr_net_send (vm[i]->notify, ACK_CREVM, PH, sizeof (packet.npkt));
}

#endif



/*  setloc (n, host, path) - set or change location for physical machine  */

void
setloc (n, host, path)
int n;
char *host, *path;
{
    struct pmdata *p;

    p = lookup (n);
    if (host && *host)  {
	DEBUG (D_SRX_ACT, "HOSTNAME for %ld:  %s", n, host, 0);
	if (p->hostname)
	    free (p->hostname);		/* don't need UNMALLOC for srx */
	p->hostname = salloc (host);
    }
    if (path && *path)  {
	DEBUG (D_SRX_ACT, "EXE_PATH for %ld:  %s", n, path, 0);
	if (p->exepath)
	    free (p->exepath);		/* don't need UNMALLOC for srx */
	p->exepath = salloc (path);
    }
}



/*  getname (n) - get hostname for physical machine n  */

char *
getname (n)
int n;
{
    int d[4];
    unsigned char a[4];
    struct hostent *he;

    sscanf (my_addr, "%d.%d.%d.%d", d, d + 1, d + 2, d + 3);
		a[3] = n ? n : d[3];
    n >>= 8;	a[2] = n ? n : d[2];
    n >>= 8;	a[1] = n ? n : d[1];
    n >>= 8;	a[0] = n ? n : d[0];
    he = gethostbyaddr ((char *) a, sizeof (a), AF_INET);
    if (he)
	return (char *) he->h_name;
    else
	return NULL;
}



/*  lookup (n) - find (create if necessary) entry for physical machine n.  */

struct pmdata *
lookup (n)
int n;
{
    struct pmdata *p;
    static struct pmdata z;

    for (p = &physm; p; p = p->next)
	if (p->num == n)
	    return p;
    p = (struct pmdata *) alloc (sizeof (struct pmdata));
    *p = z;
    p->num = n;
    p->next = physm.next;
    physm.next = p;
    return p;
}



/*  alcvm () -  allocate new virtual machine entry and return pointer  */

struct vmdata *
alcvm ()
{
    struct vmdata *v;
    static struct vmdata zeroes;

    if (++nvm > MAX_VM)
	sr_net_abort ("too many virtual machines");
    vm[nvm] = v = (struct vmdata *) alloc (sizeof (struct vmdata));
    *v = zeroes;		/* initialize to all zeroes */
    return v;
}



/*  alloc (n) - allocate n bytes, with success guaranteed  */

char *
alloc (n)
int n;
{
    char *s;

    s = malloc ((unsigned int) n);
    if (!s)
	sr_net_abort ("SRX out of memory");
    return s;
}



/*  salloc (s) - allocate and initialize string, with success guaranteed  */

char *
salloc (s)
char *s;
{
    return strcpy (alloc (strlen (s) + 1), s);
}



/*  sr_iowait (query, results, inout) - wait for input from a set of files */

void
sr_iowait (query, results, inout)
fd_set *query, *results;
enum io_type inout;
{
    int n;

    if (inout != INPUT)
	sr_net_abort ("srx iowait not on input");
    do {
	DEBUG (D_SELECT, "select: %08lX", query->fds_bits[0], 0, 0);
	*results = *query;
	n = select (FD_SETSIZE, results, (fd_set *) 0, (fd_set *) 0,
	    (struct timeval *) 0);
	DEBUG (D_SELECT, "selectd %08lX n=%ld", results->fds_bits[0], n, 0);
    } while (n < 0 && errno == EINTR);
    if (n < 0) {
	perror ("select");
	sr_net_abort ("select failure");
    }
}



/*  sr_net_abort (message) - kill all other processes and exit.
 *  Message is optional.  Used for network errors, srx errors, and normal exit.
 *
 *  note: we use SIGINT, not SIGKILL, because SIGKILL won't kill the
 *  far end of an rsh.
 */

void
sr_net_abort (message)
char *message;
{
    int i;

    DEBUG0 (D_SRX_ACT, "sr_net_abort here");

    if (message)				/* print message if given */
	fprintf (stderr, "srx: %s\n", message);

#ifndef __PARAGON__	/* not SRX's children on Paragon */

    for (i = 1; i <= nvm; i++)			/* kill all machines */
	if (vm[i]->state != GONE)  {
	    DEBUG (D_SRX_ACT, "kill %ld [%ld]", vm[i]->pid, i, 0);
	    kill (vm[i]->pid, SIGINT);
	}
	else {
	    DEBUG (D_SRX_ACT, "no kill %ld [%ld] (already gone)", 
		vm[i]->pid, i, 0);
	}
    if (!message)
	DEBUG (D_SRX_ACT, "srx exiting", 0, 0, 0);
    exit (1);					/* and quit */

#else  /* Paragon */

    if (message) {
	/* abnormal termination; be sure to kill all processes */
	DEBUG (D_SRX_ACT, "srx killing process group", 0, 0, 0);
	kill (0, SIGINT);		/* kill all processes in group */
    } else {
	/* normal termination */
	exit (0);
    }

#endif
}

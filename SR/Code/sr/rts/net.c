/*  net.c -- network interface routines  */

#include "rts.h"
#include <fcntl.h>

static void start_srx ();

#define BUFSIZE 100



/*
 *  Initialize network, if this hasn't already been done before.
 *  srx_addr is socket address of srx if it's already running.
 *  This routine must never be called with a mutex >= sr_queue_mutex.
 */
void
sr_init_net (srx_addr)
Ptr srx_addr;
{
    struct saddr_st as;
    Proc pr;
    char abuf[BUFSIZE];
    int i;

#ifdef __svr4__
#ifdef MULTI_SR
    if (sr_num_job_servers > 1)
	sr_abort ("virtual machines not implemented under Solaris MultiSR");
#endif
#endif

    LOCK (sr_exec_up_mutex, "sr_init_net");
    if (sr_exec_up) {
	UNLOCK (sr_exec_up_mutex, "sr_init_net");
	return;				/* network is already inited */
    }

    /* this socket stuff needs to be run as the IO server */
    BEGIN_IO (NULL);

    if (!srx_addr) {			/* is srx running? */
	for (i = 0; i < BUFSIZE; i++)
	    abuf[i] = '\0';
	start_srx (abuf, 100);		/* no, start it */
    } else
	strcpy (abuf, srx_addr);

    sr_net_start (as.addr);		/* start socket routines */
    DEBUG (D_SOCKET, "sr_init_net has as.addr %s", as.addr, 0, 0);
    sr_net_connect (SRX_VM, abuf);	/* connect to srx */
    sr_exec_up = TRUE;			/* set flag that the network's up */
    UNLOCK (sr_exec_up_mutex, "sr_init_net");
    END_IO (NULL);

    /* add detail to err messages because now there's more than one of us */
    sprintf (sr_my_label, "[vm %d] ", sr_my_vm);

    /* tell srx we are here */
    /* sr_net_send calls BEGIN_IO itself */
    as.ph.priority = 0;		/* srx.c doesn't have a real CUR_PROC */
    sr_net_send (SRX_VM, MSG_HELLO, &as.ph, sizeof (as));

    /* start network interface process */
    LOCK_QUEUE ("sr_init_net");
    pr = sr_spawn (sr_net_interface, MAX_INT, (Rinst)NULL, FALSE, 0L,0L,0L,0L);
    pr->pname = "[network interface]";
    sr_activate (pr);
    UNLOCK_QUEUE ("sr_init_net");
}



/*
 *  Fork and start srx.  Copy the address of the srx socket into abuf.  Since
 *  this is only called from sr_init_net, and when sr_exec_up_mutex is held,
 *  we need to release it before calling sr_abort, since sr_abort's
 *  progeny grab it.
 */

static void
start_srx (abuf, abufsize)
char abuf[];
int abufsize;
{

#ifndef __PARAGON__
    char *path, numbuf[50], trcbuf[10];
    int fd[2];
    int js_to_create;

    if (! (path = getenv (ENV_SRX)))
	path = sr_exec_path;	/* use path set by srl if no env variable */
    DEBUG (D_SRX_ACT, "srx path: %s", path, 0, 0);

    if (sr_trc_flag)
	sprintf (trcbuf, "%d", sr_trc_fd);
    else
	strcpy (trcbuf, "-1");

    js_to_create = sr_num_job_servers;
    /* don't need IO server if 1 JS;  see main.c on how this is set */
    if (js_to_create > 1)
	js_to_create -= NUM_IO_SERVERS;
    sprintf (numbuf, "%u", js_to_create);

    BEGIN_IO (NULL);
    fflush (stdout);
    fcntl (STDOUT, F_SETFL, O_APPEND);		/* coordinate VM writes */
    fcntl (STDERR, F_SETFL, O_APPEND);
    END_IO (NULL);

    if (pipe (fd) != 0)	{	/* make pipe for initial message from srx */
	/* pipe failed */
	sr_exec_up = TRUE; /* stop anyone else from starting, while we stop */
	UNLOCK (sr_exec_up_mutex, "start_srx");
	sr_abort ("can't open pipe for srx");
    }
    if ((sr_exec_pid = vfork ()) < 0) {
	/* fork failed */
	sr_exec_up = TRUE; /* stop anyone else from starting, while we stop */
	UNLOCK (sr_exec_up_mutex, "start_srx");
	sr_abort ("can't vfork to start srx");
    }
    if (sr_exec_pid == 0) {
	/* we're the child - execute srx */
	dup2 (fd[1], 0);  /* make pipe output fd 0 for srx, replacing stdin */
	execl (path, path,
	    VM_MAGIC, PROTO_VER, sr_argv[0], numbuf, trcbuf, (char *) NULL);
	/* execl failed if that returned */
	perror (path);
	sr_exec_up = TRUE; /* stop anyone else from starting, while we stop */
	UNLOCK (sr_exec_up_mutex, "start_srx");
	sr_abort ("can't execute srx");
    }

    /*
     *	The parent continues execution of the SR program.
     *	Read back the srx socket address over the pipe we just made.
     *	(The pipe is for synchronization as well as communication!)
     */
    close (fd[1]);
    if (read (fd[0], abuf, abufsize) == 0) {	/* read srx socket address */
	/* read_failed */
	sr_exec_up = TRUE;  /* stop anyone else from starting, while we stop */
	UNLOCK (sr_exec_up_mutex, "start_srx");
	sr_abort ("no reply from srx startup");
    }
    close (fd[0]);				/* no longer need the pipe */
    DEBUG (D_SOCKET, "start_srx read %s", abuf, 0, 0);

#else  /* if Paragon */

    /* 
     *  Fork and start srx on node 0 of the active compute partition.
     *  Therefore the address of srx is known in advance.
     *  The ptype is 1.
     */ 
    char *path, debug, numbuf[50], trcbuf[10];
    long node_0 = 0;
    long pid_0;
    char * argv[8];
    int js_to_create;

    argv[0] = "srx";
    argv[1] = VM_MAGIC;
    argv[2] = PROTO_VER;
    argv[3] = sr_argv[0];

    if (! (path = getenv (ENV_SRX)))
	path = sr_exec_path;	/* use path set by srl if no env variable */
    DEBUG (D_SRX_ACT, "srx path: %s", path, 0, 0);

    js_to_create = sr_num_job_servers;	
    /* don't need IO server if 1 JS;  see main.c on how this is set */
    if (js_to_create > 1)
	js_to_create -= NUM_IO_SERVERS;
    sprintf (numbuf, "%u", js_to_create);

    argv[4] = numbuf;
    argv[5] = "";		/* trcbuf doesn't matter; every VM checks env */
    argv[6] = getenv (ENV_DEBUG); 
    argv[7] = NULL;
    if (nx_loadve (&node_0, 1, 1, &pid_0, path, argv, NULL) < 0) {
	sr_exec_up = TRUE; /* stop anyone else from starting, while we stop */
	UNLOCK (sr_exec_up_mutex, "start_srx");
	sr_abort ("can't execute srx");
    }

    /*
     *	The parent continues execution of the SR program.
     */
    sprintf(abuf, "%d.%d", 0, 1);  /* set "node.ptype" as on other platforms */
    DEBUG (D_SOCKET, "start_srx as %s", abuf, 0, 0);
#endif

}



/*
 *  Network interface process.
 *  Read messages from other machines when they come in.
 *  In order to ensure fairness, this process should be
 *  run at the same priority as user processes.
 */
void
sr_net_interface ()
{
    Ptr pbuf;
    Pach ph, nph;
    enum ms_type t;
    Proc pr;
    int task_pri, exitcode, report;

    ph = 0;
    for (;;) {
	/* allocate space for packet header if we need a new one */
	if (!ph)  {
	    pbuf = sr_alc (-PBUF_SZ, 1);
	    ph = (Pach) pbuf;
	}

	/* get the header of the next available message */
	t = sr_net_recv (ph);

	/* if the whole packet is too big for the buffer,
	 * allocate a new one and copy the header. */
	if (ph->size > PBUF_SZ) {
	    nph = (Pach) sr_alc (-ph->size, 1);
	    *nph = *ph;
	    sr_free ((Ptr) ph);
	    ph = nph;
	}

	/* read the rest of the packet */
	sr_net_more (ph);

	task_pri = ph->priority;

	/* process the message according to its type */
	switch (t) {

	    case MSG_SEOF:
		LOCK (sr_exec_up_mutex, "sr_net_interface");
		/* ignore deaths of others, except for srx, unless
		 * it is OK for SRX to die */
		if (ph->origin == SRX_VM) {
		    sr_exec_up = FALSE;
		    if (!sr_srx_death_ok) {
			UNLOCK (sr_exec_up_mutex, "sr_net_interface");
			sr_abort ("srx died!");
		    }
		}
		else
		    UNLOCK (sr_exec_up_mutex, "sr_net_interface");
		break;

	    case MSG_QUIT:
		/* SRX got a MSG_STOP or enough MSG_IDLEs to tell
		 * the main VM to finalize and shutdown. */
		if (sr_my_vm != MAIN_VM)
		    sr_malf ("MSG_QUIT not main vm");	/* protocol error */
		pr = sr_spawn (sr_stop, CUR_PROC->priority, (Rinst) NULL, FALSE,
		    (long) ((struct exit_st *) ph)->code,
		    (long) ((struct exit_st *) ph)->report, 0L, 0L);
		sr_activate (pr);
		break;

	    case MSG_EXIT:
		if (sr_my_vm == MAIN_VM)
		    sr_malf ("MSG_EXIT for main vm");	/* protocol error */
		exitcode = ((struct exit_st *) ph) -> code;
		report = ((struct exit_st *) ph) -> report;
		DEBUG(D_GENERAL,"exit(%ld,%ld) per MSG_EXIT",exitcode,report,0);
		if (report)
		    sr_print_blocked ();
		EXIT (exitcode);
		/*NOTREACHED*/
		break;

	    case REQ_CALLME:
		pr = sr_spawn (sr_rmt_callme, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[callme]";
		sr_activate (pr);
		ph = 0;
		break;

	    case REQ_CREATE:
		pr = sr_spawn (sr_rmt_create, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[create]";
		sr_activate (pr);
		ph = 0;
		break;

	    case REQ_COUNT:
		pr = sr_spawn (sr_rmt_query, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[query]";
		sr_activate (pr);
		ph = 0;
		break;

	    case REQ_DESTOP:
		pr = sr_spawn (sr_rmt_destop, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[destop]";
		sr_activate (pr);
		ph = 0;
		break;
		
	    case REQ_DESTROY:
		pr = sr_spawn (sr_rmt_destroy, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[destroy]";
		sr_activate (pr);
		ph = 0;
		break;

	    case REQ_DESTVM:
		pr = sr_spawn (sr_rmt_destvm, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[destvm]";
		sr_activate (pr);
		ph = 0;
		break;
		
	    case REQ_INVOKE:
		pr = sr_spawn (sr_rmt_invk, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[invoke]";
		sr_activate (pr);
		ph = 0;
		break;

	    case REQ_RECEIVE:
		pr = sr_spawn (sr_rmt_rcv, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[receive]";
		sr_activate (pr);
		ph = 0;
		break;

	    case MSG_RCVCALL:
		pr = sr_spawn (sr_rcv_call, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[rcv_call]";
		sr_activate (pr);
		ph = 0;
		break;
		
	    case ACK_LOCVM:
	    case ACK_CREVM:
	    case ACK_FINDVM:
	    case ACK_COUNT:
	    case ACK_CREATE:
	    case ACK_DESTOP:
	    case ACK_DESTROY:
	    case ACK_DESTVM:
	    case ACK_CALLME:
	    case ACK_RECEIVE:
		ph->rem->reply = ph;
		V (ph->rem->wait);
		ph = 0;
		break;

	    case ACK_INVOKE:
		if (((Invb) ph)->type == REM_COCALL_IN)
		    sr_co_call_done ((Invb) ph);
		else {
		    ph->rem->reply = ph;
		    V (ph->rem->wait);
		    }
		ph = 0;
		break;

	    default:
		sr_abort ("unknown incoming message type");
		break;
	}
    }
}

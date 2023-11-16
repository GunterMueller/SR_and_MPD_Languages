/*  net.c -- network interface routines  */

#include "rts.h"
#include <fcntl.h>

static void start_mpdx ();

#define BUFSIZE 100



/*
 *  Initialize network, if this hasn't already been done before.
 *  mpdx_addr is socket address of mpdx if it's already running.
 *  This routine must never be called with a mutex >= mpd_queue_mutex.
 */
void
mpd_init_net (mpdx_addr)
Ptr mpdx_addr;
{
    struct saddr_st as;
    Proc pr;
    char abuf[BUFSIZE];
    int i;

#ifdef __svr4__
#ifdef MULTI_MPD
    if (mpd_num_job_servers > 1)
	mpd_abort ("virtual machines not implemented under Solaris MultiMPD");
#endif
#endif

    LOCK (mpd_exec_up_mutex, "mpd_init_net");
    if (mpd_exec_up) {
	UNLOCK (mpd_exec_up_mutex, "mpd_init_net");
	return;				/* network is already inited */
    }

    /* this socket stuff needs to be run as the IO server */
    BEGIN_IO (NULL);

    if (!mpdx_addr) {			/* is mpdx running? */
	for (i = 0; i < BUFSIZE; i++)
	    abuf[i] = '\0';
	start_mpdx (abuf, 100);		/* no, start it */
    } else
	strcpy (abuf, mpdx_addr);

    mpd_net_start (as.addr);		/* start socket routines */
    DEBUG (D_SOCKET, "mpd_init_net has as.addr %s", as.addr, 0, 0);
    mpd_net_connect (MPDX_VM, abuf);	/* connect to mpdx */
    mpd_exec_up = TRUE;			/* set flag that the network's up */
    UNLOCK (mpd_exec_up_mutex, "mpd_init_net");
    END_IO (NULL);

    /* add detail to err messages because now there's more than one of us */
    sprintf (mpd_my_label, "[vm %d] ", mpd_my_vm);

    /* tell mpdx we are here */
    /* mpd_net_send calls BEGIN_IO itself */
    as.ph.priority = 0;		/* mpdx.c doesn't have a real CUR_PROC */
    mpd_net_send (MPDX_VM, MSG_HELLO, &as.ph, sizeof (as));

    /* start network interface process */
    LOCK_QUEUE ("mpd_init_net");
    pr = mpd_spawn (mpd_net_interface, MAX_INT, (Rinst)NULL, FALSE,0L,0L,0L,0L);
    pr->pname = "[network interface]";
    mpd_activate (pr);
    UNLOCK_QUEUE ("mpd_init_net");
}



/*
 *  Fork and start mpdx.  Copy the address of the mpdx socket into abuf.  Since
 *  this is only called from mpd_init_net, and when mpd_exec_up_mutex is held,
 *  we need to release it before calling mpd_abort, since mpd_abort's
 *  progeny grab it.
 */

static void
start_mpdx (abuf, abufsize)
char abuf[];
int abufsize;
{

#ifndef __PARAGON__
    char *path, numbuf[50], trcbuf[10];
    int fd[2];
    int js_to_create;

    if (! (path = getenv (ENV_MPDX)))
	path = mpd_exec_path;	/* use path set by mpdl if no env variable */
    DEBUG (D_MPDX_ACT, "mpdx path: %s", path, 0, 0);

    if (mpd_trc_flag)
	sprintf (trcbuf, "%d", mpd_trc_fd);
    else
	strcpy (trcbuf, "-1");

    js_to_create = mpd_num_job_servers;
    /* don't need IO server if 1 JS;  see main.c on how this is set */
    if (js_to_create > 1)
	js_to_create -= NUM_IO_SERVERS;
    sprintf (numbuf, "%u", js_to_create);

    BEGIN_IO (NULL);
    fflush (stdout);
    fcntl (STDOUT, F_SETFL, O_APPEND);		/* coordinate VM writes */
    fcntl (STDERR, F_SETFL, O_APPEND);
    END_IO (NULL);

    if (pipe (fd) != 0)	{	/* make pipe for initial message from mpdx */
	/* pipe failed */
	mpd_exec_up = TRUE; /* stop anyone else from starting, while we stop */
	UNLOCK (mpd_exec_up_mutex, "start_mpdx");
	mpd_abort ("can't open pipe for mpdx");
    }
    if ((mpd_exec_pid = vfork ()) < 0) {
	/* fork failed */
	mpd_exec_up = TRUE; /* stop anyone else from starting, while we stop */
	UNLOCK (mpd_exec_up_mutex, "start_mpdx");
	mpd_abort ("can't vfork to start mpdx");
    }
    if (mpd_exec_pid == 0) {
	/* we're the child - execute mpdx */
	dup2 (fd[1], 0);  /* make pipe output fd 0 for mpdx, replacing stdin */
	execl (path, path,
	    VM_MAGIC, PROTO_VER, mpd_argv[0], numbuf, trcbuf, (char *) NULL);
	/* execl failed if that returned */
	perror (path);
	mpd_exec_up = TRUE; /* stop anyone else from starting, while we stop */
	UNLOCK (mpd_exec_up_mutex, "start_mpdx");
	mpd_abort ("can't execute mpdx");
    }

    /*
     *	The parent continues execution of the MPD program.
     *	Read back the mpdx socket address over the pipe we just made.
     *	(The pipe is for synchronization as well as communication!)
     */
    close (fd[1]);
    if (read (fd[0], abuf, abufsize) == 0) {	/* read mpdx socket address */
	/* read_failed */
	mpd_exec_up = TRUE;  /* stop anyone else from starting, while we stop */
	UNLOCK (mpd_exec_up_mutex, "start_mpdx");
	mpd_abort ("no reply from mpdx startup");
    }
    close (fd[0]);				/* no longer need the pipe */
    DEBUG (D_SOCKET, "start_mpdx read %s", abuf, 0, 0);

#else  /* if Paragon */

    /* 
     *  Fork and start mpdx on node 0 of the active compute partition.
     *  Therefore the address of mpdx is known in advance.
     *  The ptype is 1.
     */ 
    char *path, debug, numbuf[50], trcbuf[10];
    long node_0 = 0;
    long pid_0;
    char * argv[8];
    int js_to_create;

    argv[0] = "mpdx";
    argv[1] = VM_MAGIC;
    argv[2] = PROTO_VER;
    argv[3] = mpd_argv[0];

    if (! (path = getenv (ENV_MPDX)))
	path = mpd_exec_path;	/* use path set by mpdl if no env variable */
    DEBUG (D_MPDX_ACT, "mpdx path: %s", path, 0, 0);

    js_to_create = mpd_num_job_servers;	
    /* don't need IO server if 1 JS;  see main.c on how this is set */
    if (js_to_create > 1)
	js_to_create -= NUM_IO_SERVERS;
    sprintf (numbuf, "%u", js_to_create);

    argv[4] = numbuf;
    argv[5] = "";		/* trcbuf doesn't matter; every VM checks env */
    argv[6] = getenv (ENV_DEBUG); 
    argv[7] = NULL;
    if (nx_loadve (&node_0, 1, 1, &pid_0, path, argv, NULL) < 0) {
	mpd_exec_up = TRUE; /* stop anyone else from starting, while we stop */
	UNLOCK (mpd_exec_up_mutex, "start_mpdx");
	mpd_abort ("can't execute mpdx");
    }

    /*
     *	The parent continues execution of the MPD program.
     */
    sprintf(abuf, "%d.%d", 0, 1);  /* set "node.ptype" as on other platforms */
    DEBUG (D_SOCKET, "start_mpdx as %s", abuf, 0, 0);
#endif

}



/*
 *  Network interface process.
 *  Read messages from other machines when they come in.
 *  In order to ensure fairness, this process should be
 *  run at the same priority as user processes.
 */
void
mpd_net_interface ()
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
	    pbuf = mpd_alc (-PBUF_SZ, 1);
	    ph = (Pach) pbuf;
	}

	/* get the header of the next available message */
	t = mpd_net_recv (ph);

	/* if the whole packet is too big for the buffer,
	 * allocate a new one and copy the header. */
	if (ph->size > PBUF_SZ) {
	    nph = (Pach) mpd_alc (-ph->size, 1);
	    *nph = *ph;
	    mpd_free ((Ptr) ph);
	    ph = nph;
	}

	/* read the rest of the packet */
	mpd_net_more (ph);

	task_pri = ph->priority;

	/* process the message according to its type */
	switch (t) {

	    case MSG_SEOF:
		LOCK (mpd_exec_up_mutex, "mpd_net_interface");
		/* ignore deaths of others, except for mpdx, unless
		 * it is OK for MPDX to die */
		if (ph->origin == MPDX_VM) {
		    mpd_exec_up = FALSE;
		    if (!mpd_mpdx_death_ok) {
			UNLOCK (mpd_exec_up_mutex, "mpd_net_interface");
			mpd_abort ("mpdx died!");
		    }
		}
		else
		    UNLOCK (mpd_exec_up_mutex, "mpd_net_interface");
		break;

	    case MSG_QUIT:
		/* MPDX got a MSG_STOP or enough MSG_IDLEs to tell
		 * the main VM to finalize and shutdown. */
		if (mpd_my_vm != MAIN_VM)
		    mpd_malf ("MSG_QUIT not main vm");	/* protocol error */
		pr = mpd_spawn(mpd_stop, CUR_PROC->priority, (Rinst)NULL, FALSE,
		    (long) ((struct exit_st *) ph)->code,
		    (long) ((struct exit_st *) ph)->report, 0L, 0L);
		mpd_activate (pr);
		break;

	    case MSG_EXIT:
		if (mpd_my_vm == MAIN_VM)
		    mpd_malf ("MSG_EXIT for main vm");	/* protocol error */
		exitcode = ((struct exit_st *) ph) -> code;
		report = ((struct exit_st *) ph) -> report;
		DEBUG(D_GENERAL,"exit(%ld,%ld) per MSG_EXIT",exitcode,report,0);
		if (report)
		    mpd_print_blocked ();
		EXIT (exitcode);
		/*NOTREACHED*/
		break;

	    case REQ_CALLME:
		pr = mpd_spawn (mpd_rmt_callme, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[callme]";
		mpd_activate (pr);
		ph = 0;
		break;

	    case REQ_CREATE:
		pr = mpd_spawn (mpd_rmt_create, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[create]";
		mpd_activate (pr);
		ph = 0;
		break;

	    case REQ_COUNT:
		pr = mpd_spawn (mpd_rmt_query, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[query]";
		mpd_activate (pr);
		ph = 0;
		break;

	    case REQ_DESTOP:
		pr = mpd_spawn (mpd_rmt_destop, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[destop]";
		mpd_activate (pr);
		ph = 0;
		break;
		
	    case REQ_DESTROY:
		pr = mpd_spawn (mpd_rmt_destroy, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[destroy]";
		mpd_activate (pr);
		ph = 0;
		break;

	    case REQ_DESTVM:
		pr = mpd_spawn (mpd_rmt_destvm, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[destvm]";
		mpd_activate (pr);
		ph = 0;
		break;
		
	    case REQ_INVOKE:
		pr = mpd_spawn (mpd_rmt_invk, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[invoke]";
		mpd_activate (pr);
		ph = 0;
		break;

	    case REQ_RECEIVE:
		pr = mpd_spawn (mpd_rmt_rcv, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[receive]";
		mpd_activate (pr);
		ph = 0;
		break;

	    case MSG_RCVCALL:
		pr = mpd_spawn (mpd_rcv_call, task_pri, (Rinst) NULL, FALSE,
			(long) ph, 0L, 0L, 0L);
		pr->pname = "[rcv_call]";
		mpd_activate (pr);
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
		    mpd_co_call_done ((Invb) ph);
		else {
		    ph->rem->reply = ph;
		    V (ph->rem->wait);
		    }
		ph = 0;
		break;

	    default:
		mpd_abort ("unknown incoming message type");
		break;
	}
    }
}

/*  socket.c -- common socket I/O for runtime system and SRX
 *
 *  These routines call sr_net_abort (message) in case of trouble.
 */

#include "rts.h"
#include <fcntl.h>

#ifdef __PARAGON__
#include <nx.h>
extern int local_message_type = 2;      /* msg type of local VM == ptype */
static int last_node;			/* last node from which was received */
static int last_ptype;			/* ptype from last received message */
#endif

static void syserr ();

extern Vcap sr_my_vm;
extern struct idle_st sr_msg_counts;

static int lfd;				/* listener fd */
static int mfd [1 + MAX_VM];		/* machine to fd mapping */
static int fdm [FD_SETSIZE];		/* fd to machine mapping */
static Mutex mfd_fdm_mutex;		/* mutex for mfd, fdm */


/*
 *  The set of input files.  We depend on one thread being able to change this
 *  up to the time select () is actually called, even if the other thread has
 *  already called sr_iowait ().
 */

static fd_set waitset;			/* set of fd's to check for input*/
static int maxfd;			/* maximum fd to check */
static int currfd;			/* current fd being checked */

static Mutex wait_ready_set_mutex;	/* mutex for waitset */
static Mutex maxfd_mutex;		/* mutex for maxfd */
static Mutex currfd_mutex;		/* mutex for currfd */
static Mutex send_mutex;		/* mutex for sending (& counting) */


/*
 *  Initialize socket routines.  Create and bind a stream socket in the Internet
 *  domain.  Copy socket address into abuf.
 */
void
sr_net_start (abuf)
char abuf[];
{
    char host [HOST_NAME_LEN];
    struct hostent *hp;
    struct sockaddr_in sin;
    unsigned char *cp;

    BEGIN_IO (NULL);

#ifndef __PARAGON__

    /* get network address of our host */
    gethostname (host, sizeof (host));

    if ((hp = gethostbyname (host)) == NULL)
	syserr ("gethostbyname");

    if (hp->h_addrtype != AF_INET)
	syserr ("host addr type not INET");
	
    /* get socket and look for a port number */
    if ((lfd = socket (hp->h_addrtype, SOCK_STREAM, 0)) < 0)
	syserr ("socket creation");

    memset ((Ptr) &sin, 0, sizeof (struct sockaddr_in));
    memcpy ((Ptr) &sin.sin_addr, hp->h_addr, hp->h_length);
    sin.sin_family = hp->h_addrtype;
    sin.sin_port = htons (IPPORT_RESERVED);

    while (bind(lfd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) < 0) {
	/* all of the following errors can be recovered from */
	/* (though it's unclear why EBADF, e.g., occurs at all) */
	if (errno != EADDRINUSE && errno != EACCES && errno != EBADF)
	    syserr ("bind");
	if ((sin.sin_port = htons (ntohs (sin.sin_port) + 1)) > 16383)
	    sr_net_abort ("no port available for open_socket");
    }

    if (fcntl (lfd, F_SETFD, 1) == -1)	/* set close-on-exec */
	syserr ("close-on-exec");
    if (listen (lfd, MAX_VM) < 0)	/* prepare to accept conns */
	syserr ("listen");
    cp = (unsigned char *) &sin.sin_addr;
    sprintf (abuf, "%d.%d.%d.%d.%d", cp[0], cp[1], cp[2], cp[3], sin.sin_port);
    DEBUG (D_SOCKET, "listen (%ld)  %s", lfd, abuf, 0);

    FD_SET (lfd, &waitset);
    currfd = maxfd = lfd;

#else  /* if Paragon */
    /* 
     * get network address of our host
     * only included to be compatible to other SR platforms
     * format: node.message__type
     */
    sprintf (abuf, "%d.%d", mynode (), local_message_type);
    DEBUG (D_SOCKET, "our host %s", abuf, 0, 0);
#endif

    INIT_LOCK (mfd_fdm_mutex, "mfd_fdm_mutex");
    INIT_LOCK (wait_ready_set_mutex, "wait_ready_set_mutex");
    INIT_LOCK (currfd_mutex, "currfd_mutex");
    INIT_LOCK (maxfd_mutex, "maxfd_mutex");
    INIT_LOCK (send_mutex, "send_mutex");
    END_IO (NULL);
}



/*
 *  Is machine n known?
 */
Bool
sr_net_known (n)
int n;
{
    return mfd[n] != 0;
}



/*
 *  Connect to machine n at the given internet socket.
 */
void
sr_net_connect (n, address)
int n;
char *address;
{
    unsigned char *cp;
    int i, na[4], port, fd;
    struct sockaddr_in sin;
    int node, mess_type;

    BEGIN_IO (NULL);

    if (sr_net_known (n))
	sr_net_abort ("attempt to establish duplicate connection");

#ifndef __PARAGON__

    /* construct network address in structure */
    sscanf (address, "%d.%d.%d.%d.%d", na + 0, na + 1, na + 2, na + 3, &port);
    memset ((Ptr) &sin, 0, sizeof (sin));
    cp = (unsigned char *) &sin.sin_addr;
    for (i = 0; i < 4; i++)
	*cp++ = (unsigned char) na[i];
    sin.sin_family = AF_INET;
    sin.sin_port = (unsigned short) port;

    /* create socket */
    if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	syserr ("socket creation");

    /* connect to socket */
    DEBUG (D_SOCKET, "connect (%ld) %s  (vm %ld)", fd, address, n);
    if (connect (fd, (struct sockaddr *) &sin, sizeof (sin)) < 0)
	syserr ("connect");

    /* add to set of known machines and fds */
    LOCK (mfd_fdm_mutex, "sr_net_connect");
    mfd[n] = fd;
    fdm[fd] = n;
    if (fd > maxfd)
	maxfd = fd;
    if (fd > FD_SETSIZE) {
	UNLOCK (mfd_fdm_mutex, "sr_net_connect");
	sr_net_abort ("fd too big to select");
    }
    FD_SET (fd, &waitset);
    UNLOCK (mfd_fdm_mutex, "sr_net_connect");

#else

    /*
     * add to set of known machines and fds 
     * There is no need on a paragon to establish a connection before using it 
     */
    DEBUG (D_SOCKET, "connect %s  (vm %ld)", address, n, 0);
    LOCK (mfd_fdm_mutex, "sr_net_connect");
    sscanf (address, "%d.%d", &node, &mess_type);
    mfd[n] = node * 10000 + mess_type;		/* encode msg type and node */
    UNLOCK (mfd_fdm_mutex, "sr_net_connect");

#endif

    END_IO (NULL);
}



/*
 *  Send a packet to an already-known machine.
 *  Buffer begins with a standard packet header.
 *  Origin (from global "sr_my_vm"), type, dest, and size are added.
 */
void
sr_net_send (dest, type, ph, size)
int dest;
enum ms_type type;
Pach ph;
int size;
{
    char *addr;
    int fd, n, rem;

    BEGIN_IO (NULL);

    ph->origin = sr_my_vm;
    ph->dest = dest;
    ph->size = size;
    ph->type = type;

    fd = mfd[dest];
    DEBUG5 (D_SENT, "send to %ld:   %13s, n=%ld prio %ld, fd/mtype=%ld",
	dest, sr_msgname (type), size, ph->priority, fd % 10000);

    if (!fd)
	if (dest == sr_my_vm)
	    sr_net_abort ("sr_net_send to self");
	else
	    sr_net_abort ("sr_net_send to unknown destination");

    LOCK (send_mutex, "sr_net_send");
    sr_msg_counts.nmsgs[dest]++;	/* include in counts if being sent */

#ifndef __PARAGON__
    rem = ph->size;
    addr = (char *) ph;

    while (rem > 0) {
	n = write (fd, addr, rem);
	if (n < 0) {
	    UNLOCK (send_mutex, "sr_net_send");
	    syserr ("sr_net_send");
	}
	rem -= n;
	addr += n;
    }

#else

    csend (fd % 10000, (char *) ph, PACH_SZ, fd / 10000, fd % 10000);
    if ((ph->size - PACH_SZ) > 0)
	csend (fd % 10000, ((char *) ph) + PACH_SZ, ph->size - PACH_SZ,
	    fd / 10000, fd % 10000);

#endif

    UNLOCK (send_mutex, "sr_net_send");

    END_IO (NULL);
}



/*
 *  Read the next available packet (from anyone).
 *  Return the message type, or MSG_SEOF if EOF is read.
 *
 *  The different machines are polled in round-robin fashion.
 *  New connections are accepted transparently as part of the loop.
 */
enum ms_type
sr_net_recv (ph)
Pach ph;
{

#ifndef __PARAGON__

    int n, result, my_currfd;
    static struct sockaddr sockbuff;
    static size_t buffsize;
    static fd_set readyset;	/* fd's with input available.  Fortunately,
				 * the only place this is assigned to is
				 * already protected. */

    /*  if we enter as non-IO Server we need to become it, and also
     *  unbecome it when we return */

    BEGIN_IO (NULL);

    for (;;)	{

	/*
	 *  Continue looping from previous call; currfd is the last
	 *  fd tried.  When currfd reaches the listener socket:
	 *	-- refresh the select data
	 *	-- accept a new connection if offered
	 */
	LOCK (currfd_mutex, "sr_net_recv");
	if (++currfd > maxfd)
	    currfd = 0;
	my_currfd = currfd;
	UNLOCK (currfd_mutex, "sr_net_recv");

	if (my_currfd == lfd)  {
	    sr_iowait (&waitset, &readyset, INPUT);  /* wait for some input */

	    /* accept a new connection if one is available */
	    LOCK (wait_ready_set_mutex, "sr_net_recv1");
	    if (FD_ISSET (lfd, &readyset)) {
		buffsize = sizeof (sockbuff);
		n = accept (my_currfd, &sockbuff, &buffsize);
		DEBUG (D_SOCKET, "accept (%ld) => %ld", my_currfd, n, 0);
		if (n < 0)
		    syserr ("accept");
		LOCK (maxfd_mutex, "sr_net_recv");
		if (n > maxfd)
		    maxfd = n;
		UNLOCK (maxfd_mutex, "sr_net_recv");
		if (n > FD_SETSIZE)
		    sr_net_abort ("fd too big to select");
		FD_SET (n, &waitset);
	    }
	    UNLOCK (wait_ready_set_mutex, "sr_net_recv");

	} else {
	    LOCK (wait_ready_set_mutex, "sr_net_recv");
	    result = FD_ISSET (my_currfd, &readyset);
	    if (!result) {
		UNLOCK (wait_ready_set_mutex, "sr_net_recv");
	    } else {
		/* read packet */

		n = read (my_currfd, (Ptr) ph, PACH_SZ);
		if (n != PACH_SZ)
		    if (n == 0) {
			/* got EOF -- fake an EOF packet */
			DEBUG (D_RCVD, "from %ld: EOF", fdm[my_currfd], 0, 0);
			close (my_currfd);
			FD_CLR (my_currfd, &waitset);
			ph->size = PACH_SZ;
			ph->origin = fdm[my_currfd];
			ph->dest = sr_my_vm;
			LOCK (mfd_fdm_mutex, "sr_net_recv");
			fdm[my_currfd] = 0;
			mfd[ph->origin] = 0;
			UNLOCK (mfd_fdm_mutex, "sr_net_recv");
			UNLOCK (wait_ready_set_mutex, "sr_net_recv");
			return ph->type = MSG_SEOF;
		    } else if (n < 0) {
			DEBUG (D_SOCKET, "read (%ld) [from %ld], errno=%ld",
			    my_currfd, fdm[my_currfd], errno);
			if (errno == ECONNRESET) {
			    /* one of our cohorts died. ignore here --
			     * could be a shutdown in progress */
			    close (my_currfd);
			    LOCK (mfd_fdm_mutex, "sr_net_recv");
			    mfd[fdm[my_currfd]] = 0;
			    fdm[my_currfd] = 0;
			    UNLOCK (mfd_fdm_mutex, "sr_net_recv");
			    FD_CLR (my_currfd, &waitset);
			    UNLOCK (wait_ready_set_mutex, "sr_net_recv");
			    continue;
			}
			syserr ("packet read");
		    } else {
			sr_net_abort ("packet truncated");
		    }

		sr_msg_counts.nmsgs[sr_my_vm]--;	/* count received msg */
		    /* (only this process alters that entry; so no locking) */
		DEBUG (D_RCVD, "from %ld: %13s, n=%ld", ph->origin,
			sr_msgname (ph->type), ph->size);
		LOCK (mfd_fdm_mutex, "sr_net_recv");
		if (fdm[my_currfd] != ph->origin)
		    if (fdm[my_currfd] == 0)  {
			fdm[my_currfd] = ph->origin;
			if (mfd[ph->origin] != 0)
			    sr_net_abort ("duplicate connection detected");
			mfd[ph->origin] = my_currfd;
		    } else {
			DEBUG (D_RCVD, "  fdm[%ld]=%ld; mfd[origin]=%ld",
			    my_currfd, fdm[my_currfd], mfd[ph->origin]);
			sr_net_abort ("misdelivered mail");
		    }
		UNLOCK (mfd_fdm_mutex, "sr_net_recv");
		UNLOCK (wait_ready_set_mutex, "sr_net_recv");
		END_IO (NULL);

		return ph->type;
	    }
	}
    }

#else

    /* 
     * Check if a message is waiting.  If nothing is waiting, block:
     * if SRX block on crecvx, else block by adding process to iowait list.
     */
#ifndef SRX
	if (iprobex (local_message_type, -1, -1, msginfo) == 0)
	    sr_iowait (NULL, NULL, INPUT);
#endif 
    BEGIN_IO (NULL);
	DEBUG (D_RCVD, "wait receive %u", local_message_type, 0, 0); 
	crecvx (local_message_type, (Ptr) ph, PACH_SZ, -1, -1, msginfo);
	last_node = infonode ();	/* remember senders node of last msg */
	last_ptype = infoptype ();	/* remember senders ptype of last msg */
	DEBUG (D_RCVD, "from %ld:   %13s, s=%ld", ph->origin ,
	sr_msgname (ph->type), ph->size);
	sr_msg_counts.nmsgs[sr_my_vm]--;
	if (!(mfd[ph->origin]))		/* if sender unknown, add to mfd list */
	    mfd[ph->origin] = last_node * 10000 + last_ptype;
	return ph->type;

    END_IO (NULL);

#endif

}



/*
 *  Read the rest of a message for which we have only the header.
 */
void
sr_net_more (ph)
Pach ph;
{
    int fd, n, rem;
    char *addr;

    rem = ph->size - PACH_SZ;
    if (rem == 0)
	return;
    else if (rem < 0)
	sr_net_abort ("sr_net_more: bad size");

    BEGIN_IO (NULL);

#ifndef __PARAGON__

    fd = mfd[ph->origin];
    if (!fd)
	sr_net_abort ("sr_net_more: unknown origin");
    addr = (char *) ph + PACH_SZ;

    while (rem > 0) {
	n = read (fd, addr, rem);
	if (n < 0) {
	    if (errno == EINTR)
		continue;
	    else
		syserr ("sr_net_more");
	}
	if (n == 0)
	    sr_net_abort ("EOF in mid-message");
	rem -= n;
	addr += n;
    }

#else

    crecvx (local_message_type, (char *) ph + PACH_SZ,
	rem, last_node, last_ptype, msginfo);
#endif

    END_IO (NULL);

}



/*
 *  Diagnose an error from a system call.
 *  Format a message and call sr_net_abort.
 */
static void
syserr (message)
char *message;
{
    char s[100];

    sprintf (s, "%s: %s", message ? message : "network I/O", strerror(errno));
    sr_net_abort (s);
}

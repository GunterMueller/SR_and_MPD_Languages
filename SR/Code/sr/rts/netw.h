/*  netw.h -- data structures and macros for intermachine communication  */



/*  Version number for the protocol used between srx and the runtime system.
 *  Increment this whenever the interface changes; then srx can detect
 *  connection attempts from mismatched executables, instead of simply
 *  malfunctioning.  */

#define PROTO_VER "v94f"



/*  other general definitions */

#define PACH_SZ (sizeof (struct pach_st))
#define PBUF_SZ 1024		/* size of buffer for incoming packets */

#define VM_MAGIC "{SR}"		/* magic word for argv[1] on exec calls */

#define SRX_VM 0		/* virtual machine number for srx */
#define MAIN_VM	1		/* virtual machine number for main machine */
#define MAX_VM 253		/* maximum VM number (limited by max files) */

#define SOCK_ADDR_SIZE	24	/* size of socket address as a string */



/*  remote request message descriptor */

struct remd_st {
    Sem wait;			/* semaphore to wait for reply on */
    Pach reply;			/* incoming reply message */
};



/*  number message -- REQ_CALLME, REQ_CREVM, REQ_DESTVM, REQ_FINDVM,
 *			ACK_CREVM, ACK_COUNT  */
struct num_st {
    struct pach_st ph;		/* packet header */
    int num;			/* exit code or phys or virt machine num */
};


/*  two-number message -- MSG_QUIT, MSG_EXIT */
struct exit_st {
    struct pach_st ph;		/* packet header */
    int code;			/* exit status */
    int report;			/* report blocked processes? */
};


/*  remote resource message -- REQ_CREATE, REQ_DESTROY  */

struct rres_st {
    struct pach_st ph;		/* packet header */
    Rcap rc;			/* resource capability */
};


/*  remote opcap message -- REQ_COUNT, REQ_RECEIVE, REQ_DESTOP  */

struct ropc_st {
    struct pach_st ph;		/* packet header */
    Ocap oc;			/* resource capability */
    Bool elseflag;		/* NZ if else present on receive */
};



/*  socket address message -- MSG_HELLO, ACK_FINDVM, REQ_LOCVM  */

struct saddr_st {
    struct pach_st ph;		/* packet header */
    char addr[SOCK_ADDR_SIZE];	/* socket address */
};



/*  location message -- REQ_LOCVM  */

struct locn_st {
    struct pach_st ph;		/* packet header */
    int num;			/* physical machine number */
    char text[1000];		/* hostname, then path  (may be shorter) */
};



/*  idle message -- MSG_IDLE  */

struct idle_st {
    struct pach_st ph;		/* packet header */
    int nmsgs[MAX_VM+1];	/* msg counts */
				/* nmsgs[i] is count of msgs sent to VM i */
				/* nmsgs[my_vm] is negative of total received */
};



/* parameter array for starting a VM on Intel Paragon */

typedef struct {
    int phys_machine;
    int virt_machine;
    char srx_addr[30];
} parameter;

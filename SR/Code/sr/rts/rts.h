/*  rts.h -- master include file for the runtime system  */



enum io_type { INPUT, OUTPUT };

typedef struct memh_st *Memh;		/* memory block header */
typedef struct oper_st *Oper;		/* operation entry */
typedef struct class_st *Class;		/* operation class structure */
typedef struct pool_str *Pool;		/* runtime structure pool */

/* initial values of sequence numbers */
#define INIT_SEQ_CO  ((unsigned short)0x4001)	/* co */
#define INIT_SEQ_OP  ((unsigned short)0x8001)	/* op */
#define INIT_SEQ_RES ((unsigned short)0x1001)	/* resource */

#define MEMH_SZ (SRALIGN(sizeof(struct memh_st)))	/* size of memblk hdr */

#define RTS_WARN(s)  (sr_message ("warning", s))

#define RTS
#include "../srmulti.h"		/* for Solaris, must precede system includes */
#include "../gen.h"
#include "multimac.h"

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>

#ifndef errno		/* sometimes it's a macro */
extern int errno;	/* sometimes it's not defined by errno.h */
#endif


/* EXIT is defined for MultiSR, but the srx files also need it. */
#ifndef MULTI_SR
#undef EXIT             /* zap previous MultiSR definition */
#define EXIT(code)      exit(code)
#endif

#define RUNTIME
#include "../sr.h"

#include "../config.h"
#include "debug.h"
#include "procsem.h"
#include "netw.h"
#include "oper.h"
#include "res.h"

#include "funcs.h"
#include "macros.h"

#include "globals.h"



/*  select () stuff --
 *  would be cleaner if the various systems had consistent include files */

#include <sys/file.h>
#ifdef _AIX
#include <sys/select.h>
#endif

#ifndef FD_SETSIZE	/* 4.3BSD defines these */
#define FD_SETSIZE	32
#define FD_SET(n,p)	((p)->fds_bits[0] |= (1<< (n)))
#define FD_CLR(n,p)	((p)->fds_bits[0] &= ~ (1<< (n)))
#define FD_ISSET(n,p)	((p)->fds_bits[0] & (1<< (n)))
#endif /* FD_SETSIZE */



/*  runtime error messages  */

typedef enum {
#define RUNERR(sym,num,msg) sym=num,
#include "../runerr.h"
#undef RUNERR
E_DUMMY
} Runerr;

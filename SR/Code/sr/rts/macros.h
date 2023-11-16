/*  macros.h -- macro definitions  */


/*  Trace runtime event.  */

#define TRACE(label,locn,v) (sr_trc_flag ? sr_trace(label,locn,(Ptr)v) : 0)



/*  Insert a node at the head of a single-headed doubly-linked list.
 *  This and delete are not protected, since they are used within
 *  the protection of mem_mutex, except for one instance of insert
 *  in resource creation (and it is safe there).
 */
#define	insert(node,head,next,last) { \
    node->next = head; \
    node->last = NULL; \
    if (head != NULL) \
	head->last = node; \
    head = node; \
}



/*  Remove a node from a single-headed doubly-linked list.  */

#define	delete(node,head,next,last) { \
    if (node->last == NULL) { \
	if ((head = node->next) != NULL) \
	    head->last = NULL; \
    } else if ((node->last->next = node->next) != NULL) \
	node->next->last = node->last; \
    else node->last->next = NULL; \
}



/*  Platform-dependent I/O.  */

#ifdef __linux__

#define _pptr _IO_write_ptr
#define _gptr _IO_read_ptr
#define _egptr _IO_read_end
#define _pbase _IO_write_base

#if defined(_BITS_TYPES_H) && ! defined(__USE_XOPEN)
#define fds_bits __fds_bits
#endif /* _BITS_TYPES_H */

#define _S_NO_READS 4		/* Reading not allowed */
#define _S_NO_WRITES 8		/* Writing not allowd */
#define ISOPEN(f) \
    (((f)->_flags & (_S_NO_READS|_S_NO_WRITES)) != (_S_NO_READS|_S_NO_WRITES))
#define INCH(l,f) \
    (((f->_gptr)<(f->_egptr))?(int)(*(unsigned char *)(f)->_gptr++):sr_inchar(l,f))
#define OUCH(l,f,c) \
    ((((f->_pptr)-(f->_pbase))>0)?(*(f)->_pptr++=(c)):outchar(l,f,c))

#else

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)

#define ISOPEN(f) \
    ((fp)->_flags & (__SRD | __SWR | __SRW))
#define INCH(l,f) \
    (--(f)->_r>=0?(int)(*(unsigned char*)(f)->_p++):sr_inchar(l,f))
#define OUCH(l,f,c) \
    (--(f)->_w>=0?(*(f)->_p++=(c)):outchar(l,f,c))

#else				/* 7th edn Unix descendents */

#define ISOPEN(f) \
    ((fp)->_flag & (_IOREAD | _IOWRT | _IORW))
#define INCH(l,f) \
    (--(f)->_cnt>=0?(int)(*(unsigned char*)(f)->_ptr++):sr_inchar(l,f))
#define OUCH(l,f,c) \
    (--(f)->_cnt>=0?(*(f)->_ptr++=(c)):outchar(l,f,c))

#endif
#endif


/*
 *  Return rv if fp is the noop file.
 *  Abort if the null file, or if the pointer is null.
 */
#define CHECK_FILE(locn, fp, rv)	{ \
    if (! (fp)) \
	sr_loc_abort (locn, "null pointer for file descriptor"); \
    if ((fp) == (File) NOOP_FILE) \
	return rv; \
    if ((fp) == (File) NULL_FILE) \
	sr_loc_abort (locn, "operation attempted on null file"); \
    if (!ISOPEN(fp)) \
	sr_loc_abort (locn, "file not open"); \
}

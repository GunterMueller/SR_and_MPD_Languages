/*  funcs.h -- declarations of RTS Procedures  */
/*  (varargs functions declared in sr.h are commented out)  */

/* main.c */
extern	int	main ();
extern	void	sr_abort ();
extern	char*	sr_fmt_locn ();
extern	void	sr_malf ();
extern	void	sr_message ();
/* extern int	sr_runerr (); */
extern	void	sr_net_abort ();
extern	void	sr_stk_corrupted ();
extern	void	sr_stk_overflow ();
extern	void	sr_stk_underflow ();
extern	void	sr_stop ();
extern	void	sr_loc_abort ();

/* alloc.c */
extern	Ptr	sr_alc ();
extern	String*	sr_alc_string ();
extern	Ptr	sr_alloc ();
extern	void	sr_locked_free ();
extern	void	sr_free ();
extern	void	sr_init_mem ();
extern	void	sr_res_free ();

/* array.c */
extern	Array*	sr_acopy ();
extern	int	sr_acount ();
extern	Ptr	sr_astring ();
extern	Array*	sr_aswap ();
extern	String*	sr_chgstr ();
extern	Ptr	sr_clone ();
/* extern Array *sr_init_array (); */
/* extern Ptr	sr_slice (); */
extern	Ptr	sr_sslice ();
extern	Array*	sr_strarr ();

/* co.c */
extern	void	sr_co_call ();
extern	void	sr_co_call_done ();
extern	void	sr_co_end ();
extern	void	sr_co_send ();
extern	void	sr_co_start ();
extern	Ptr	sr_co_wait ();
extern	void	sr_init_co ();

/* conv.c */
extern	int	sr_boolval ();
extern	Array*	sr_chars ();
extern	int	sr_charval ();
extern	int	sr_cvbool ();
extern	int	sr_cvint ();
extern	Ptr	sr_fmt_arr ();
extern	Ptr	sr_fmt_bool ();
extern	Ptr	sr_fmt_char ();
extern	Ptr	sr_fmt_int ();
extern	Ptr	sr_fmt_ptr ();
extern	Ptr	sr_fmt_real ();
extern	int	sr_intval ();
extern	Ptr	sr_ptrval ();
extern	Real	sr_realval ();

/* debug.c */
extern	int	sr_bugout ();
extern	void	sr_debug ();
extern	int	sr_get_debug ();
extern	void	sr_init_debug ();
extern	char*	sr_msgname ();
extern	void	sr_set_debug ();

/* event.c */
extern	int	sr_age ();
extern	int	sr_evcheck ();
extern	Bool	sr_evio_list_empty ();
extern	void	sr_init_event ();
extern	void	sr_iowait ();
extern	void	sr_nap ();
extern	Bool	sr_nap_list_empty ();

/* invoke.c */
extern	Invb	sr_dup_invb ();
extern	void	sr_finished_input ();
extern	void	sr_finished_proc ();
extern	Ptr	sr_forward ();
extern	void	sr_init_semop ();
extern	Ptr	sr_invoke ();
extern	void	sr_rej_inv ();
extern	Invb	sr_reply ();

/* io.c */
extern	int	sr_begin_io ();
extern	void	sr_close ();
extern	int	sr_end_io ();
extern	void	sr_flush ();
extern	int	sr_get_carray ();
extern	int	sr_get_string ();
extern	int	sr_inchar ();
extern	void	sr_init_io ();
extern	File	sr_open ();
/* extern void	sr_printf (); */
/* extern int	sr_read (); */
extern	Bool	sr_remove ();
extern	int	sr_seek ();
extern	int	sr_where ();

/* iop.c */
extern	Bool	sr_cap_ck ();
extern	void	sr_iaccess ();
extern	void	sr_invk_iop ();
extern	void	sr_reaccess ();
extern	void	sr_rm_iop ();

/* math.c */
/* extern int	sr_imax (); */
/* extern int	sr_imin (); */
extern	int	sr_imod ();
extern	void	sr_init_random ();
extern	int	sr_itoi ();
extern	Real	sr_random ();
/* extern Real	sr_rmax (); */
/* extern Real	sr_rmin (); */
extern	Real	sr_rmod ();
extern	Real	sr_round ();
extern	Real	sr_rtoi ();
extern	Real	sr_rtor ();
extern	void	sr_seed ();

/* misc.c */
extern	int	sr_arg_bool ();
extern	int	sr_arg_carray ();
extern	int	sr_arg_char ();
extern	Ptr	sr_gswap ();
extern	int	sr_arg_int ();
extern	int	sr_arg_ptr ();
extern	int	sr_arg_real ();
extern	int	sr_arg_string ();
/* extern Ptr	sr_cat (); */
extern	void	sr_dispose ();
extern	Ptr	sr_gswap ();
extern	void	sr_init_misc ();
extern	Ptr	sr_new ();
extern	int	sr_numargs ();
extern	String*	sr_sswap ();
extern	int	sr_strcmp ();

/* net.c */
extern	void	sr_init_net ();
extern	void	sr_net_interface ();

/* oper.c */
extern	Ptr	sr_chk_myinv ();
extern	void	sr_dest_op ();
extern	Ptr	sr_get_anyinv ();
extern	void	sr_init_class ();
extern	void	sr_init_oper ();
extern	void	sr_kill_inops ();
extern	void	sr_kill_resops ();
extern	Ptr	sr_make_class ();
extern	Ptr	sr_make_inops ();
extern	Ptr	sr_make_arraysem ();
extern	void	sr_make_proc ();
extern	Ptr	sr_make_semop ();
extern	Ocap	sr_new_op ();
extern	int	sr_query_iop ();
extern	Ptr	sr_receive ();

/* pool.c */
extern	Ptr	sr_addpool ();		/* allocate an element */
extern	void	sr_delpool ();		/* deallocate an element */
extern	void	sr_eachpool ();		/* iterate over allocated elements */
extern	Pool	sr_makepool ();		/* make a new pool */

/* process.c */
extern	void	sr_activate ();
extern	void	sr_enqueue ();
extern	void	sr_init_proc ();
extern	void	sr_kill ();
extern	void	sr_loop_resched ();
extern	void	sr_missing_children ();
extern	int	sr_mypri ();
extern	void	sr_print_blocked ();
extern	void	sr_reschedule ();
extern	void	sr_scheduler ();
extern	void	sr_setpri ();
extern	Proc	sr_spawn ();

/* remote.c */
extern	void	sr_init_rem ();
extern	Pach	sr_remote ();
extern	void	sr_rmt_callme ();
extern	void	sr_rmt_create ();
extern	void	sr_rmt_destop ();
extern	void	sr_rmt_destroy ();
extern	void	sr_rmt_destvm ();
extern	void	sr_rmt_invk ();
extern	void	sr_rmt_query ();
extern	void	sr_rmt_rcv ();
extern	void	sr_rcv_call ();

/* res.c */
extern	Ptr	sr_alloc_rv ();
extern	void	sr_create_global ();
extern	void	sr_create_res ();
extern	Ptr	sr_create_resource ();
extern	void	sr_dest_all ();
extern	void	sr_destroy ();
extern	void	sr_destroy_globals ();
extern	void	sr_finished_final ();
extern	void	sr_finished_init ();
extern	void	sr_init_res ();
extern	Ptr	sr_literal_rcap ();

/* scan.c */
/* extern int	sr_scanf (); */

/* semaphore.c */
extern	void	P ();
extern	void	V ();
extern	void	sr_init_sem ();
extern	void	sr_kill_sem ();
extern	Sem	sr_make_sem ();
extern	void	sr_init_arraysem ();

/* socket.c */
extern	void	sr_net_connect ();
extern	Bool	sr_net_known ();
extern	void	sr_net_more ();
extern	enum	ms_type	sr_net_recv ();
extern	void	sr_net_send ();
extern	void	sr_net_start ();

/* trace.c */
extern	void	sr_init_trace ();
extern	int	sr_trace ();

/* vm.c */
extern	Vcap	sr_crevm ();
extern	Vcap	sr_crevm_name ();
extern	void	sr_destvm ();
extern	void	sr_init_vm ();
extern	void	sr_locate ();

/* ../csw/asm.s */
extern	void	sr_build_context ();
extern	void	sr_check_stk ();
extern	void	sr_chg_context ();

/* ../multi/multi.c */
extern	void	sr_init_multiSR ();
extern	void	sr_jobserver_first ();
extern	void	sr_create_jobservers ();

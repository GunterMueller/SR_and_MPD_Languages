/*  funcs.h -- declarations of RTS Procedures  */
/*  (varargs functions declared in mpd.h are commented out)  */

/* main.c */
extern	int	main ();
extern	void	mpd_abort ();
extern	char*	mpd_fmt_locn ();
extern	void	mpd_malf ();
extern	void	mpd_message ();
/* extern int	mpd_runerr (); */
extern	void	mpd_net_abort ();
extern	void	mpd_stk_corrupted ();
extern	void	mpd_stk_overflow ();
extern	void	mpd_stk_underflow ();
extern	void	mpd_stop ();
extern	void	mpd_loc_abort ();

/* alloc.c */
extern	Ptr	mpd_alc ();
extern	String*	mpd_alc_string ();
extern	Ptr	mpd_alloc ();
extern	void	mpd_locked_free ();
extern	void	mpd_free ();
extern	void	mpd_init_mem ();
extern	void	mpd_res_free ();

/* array.c */
extern	Array*	mpd_acopy ();
extern	int	mpd_acount ();
extern	Ptr	mpd_astring ();
extern	Array*	mpd_aswap ();
extern	String*	mpd_chgstr ();
extern	Ptr	mpd_clone ();
/* extern Array *mpd_init_array (); */
/* extern Ptr	mpd_slice (); */
extern	Ptr	mpd_sslice ();
extern	Array*	mpd_strarr ();

/* co.c */
extern	void	mpd_co_call ();
extern	void	mpd_co_call_done ();
extern	void	mpd_co_end ();
extern	void	mpd_co_send ();
extern	void	mpd_co_start ();
extern	Ptr	mpd_co_wait ();
extern	void	mpd_init_co ();

/* conv.c */
extern	int	mpd_boolval ();
extern	Array*	mpd_chars ();
extern	int	mpd_charval ();
extern	int	mpd_cvbool ();
extern	int	mpd_cvint ();
extern	Ptr	mpd_fmt_arr ();
extern	Ptr	mpd_fmt_bool ();
extern	Ptr	mpd_fmt_char ();
extern	Ptr	mpd_fmt_int ();
extern	Ptr	mpd_fmt_ptr ();
extern	Ptr	mpd_fmt_real ();
extern	int	mpd_intval ();
extern	Ptr	mpd_ptrval ();
extern	Real	mpd_realval ();

/* debug.c */
extern	int	mpd_bugout ();
extern	void	mpd_debug ();
extern	int	mpd_get_debug ();
extern	void	mpd_init_debug ();
extern	char*	mpd_msgname ();
extern	void	mpd_set_debug ();

/* event.c */
extern	int	mpd_age ();
extern	int	mpd_evcheck ();
extern	Bool	mpd_evio_list_empty ();
extern	void	mpd_init_event ();
extern	void	mpd_iowait ();
extern	void	mpd_nap ();
extern	Bool	mpd_nap_list_empty ();

/* invoke.c */
extern	Invb	mpd_dup_invb ();
extern	void	mpd_finished_input ();
extern	void	mpd_finished_proc ();
extern	Ptr	mpd_forward ();
extern	void	mpd_init_semop ();
extern	Ptr	mpd_invoke ();
extern	void	mpd_rej_inv ();
extern	Invb	mpd_reply ();

/* io.c */
extern	int	mpd_begin_io ();
extern	void	mpd_close ();
extern	int	mpd_end_io ();
extern	void	mpd_flush ();
extern	int	mpd_get_carray ();
extern	int	mpd_get_string ();
extern	int	mpd_inchar ();
extern	void	mpd_init_io ();
extern	File	mpd_open ();
/* extern void	mpd_printf (); */
/* extern int	mpd_read (); */
extern	Bool	mpd_remove ();
extern	int	mpd_seek ();
extern	int	mpd_where ();

/* iop.c */
extern	Bool	mpd_cap_ck ();
extern	void	mpd_iaccess ();
extern	void	mpd_invk_iop ();
extern	void	mpd_reaccess ();
extern	void	mpd_rm_iop ();

/* math.c */
/* extern int	mpd_imax (); */
/* extern int	mpd_imin (); */
extern	int	mpd_imod ();
extern	void	mpd_init_random ();
extern	int	mpd_itoi ();
extern	Real	mpd_random ();
/* extern Real	mpd_rmax (); */
/* extern Real	mpd_rmin (); */
extern	Real	mpd_rmod ();
extern	Real	mpd_round ();
extern	Real	mpd_rtoi ();
extern	Real	mpd_rtor ();
extern	void	mpd_seed ();

/* misc.c */
extern	int	mpd_arg_bool ();
extern	int	mpd_arg_carray ();
extern	int	mpd_arg_char ();
extern	Ptr	mpd_gswap ();
extern	int	mpd_arg_int ();
extern	int	mpd_arg_ptr ();
extern	int	mpd_arg_real ();
extern	int	mpd_arg_string ();
/* extern Ptr	mpd_cat (); */
extern	void	mpd_dispose ();
extern	Ptr	mpd_gswap ();
extern	void	mpd_init_misc ();
extern	Ptr	mpd_new ();
extern	int	mpd_numargs ();
extern	String*	mpd_sswap ();
extern	int	mpd_strcmp ();

/* net.c */
extern	void	mpd_init_net ();
extern	void	mpd_net_interface ();

/* oper.c */
extern	Ptr	mpd_chk_myinv ();
extern	void	mpd_dest_op ();
extern	Ptr	mpd_get_anyinv ();
extern	void	mpd_init_class ();
extern	void	mpd_init_oper ();
extern	void	mpd_kill_inops ();
extern	void	mpd_kill_resops ();
extern	Ptr	mpd_make_class ();
extern	Ptr	mpd_make_inops ();
extern	Ptr	mpd_make_arraysem ();
extern	void	mpd_make_proc ();
extern	Ptr	mpd_make_semop ();
extern	Ocap	mpd_new_op ();
extern	int	mpd_query_iop ();
extern	Ptr	mpd_receive ();

/* pool.c */
extern	Ptr	mpd_addpool ();		/* allocate an element */
extern	void	mpd_delpool ();		/* deallocate an element */
extern	void	mpd_eachpool ();	/* iterate over allocated elements */
extern	Pool	mpd_makepool ();	/* make a new pool */

/* process.c */
extern	void	mpd_activate ();
extern	void	mpd_enqueue ();
extern	void	mpd_init_proc ();
extern	void	mpd_kill ();
extern	void	mpd_loop_resched ();
extern	void	mpd_missing_children ();
extern	int	mpd_mypri ();
extern	void	mpd_print_blocked ();
extern	void	mpd_reschedule ();
extern	void	mpd_scheduler ();
extern	void	mpd_setpri ();
extern	Proc	mpd_spawn ();

/* remote.c */
extern	void	mpd_init_rem ();
extern	Pach	mpd_remote ();
extern	void	mpd_rmt_callme ();
extern	void	mpd_rmt_create ();
extern	void	mpd_rmt_destop ();
extern	void	mpd_rmt_destroy ();
extern	void	mpd_rmt_destvm ();
extern	void	mpd_rmt_invk ();
extern	void	mpd_rmt_query ();
extern	void	mpd_rmt_rcv ();
extern	void	mpd_rcv_call ();

/* res.c */
extern	Ptr	mpd_alloc_rv ();
extern	void	mpd_create_global ();
extern	void	mpd_create_res ();
extern	Ptr	mpd_create_resource ();
extern	void	mpd_dest_all ();
extern	void	mpd_destroy ();
extern	void	mpd_destroy_globals ();
extern	void	mpd_finished_final ();
extern	void	mpd_finished_init ();
extern	void	mpd_init_res ();
extern	Ptr	mpd_literal_rcap ();

/* scan.c */
/* extern int	mpd_scanf (); */

/* semaphore.c */
extern	void	P ();
extern	void	V ();
extern	void	mpd_init_sem ();
extern	void	mpd_kill_sem ();
extern	Sem	mpd_make_sem ();
extern	void	mpd_init_arraysem ();

/* socket.c */
extern	void	mpd_net_connect ();
extern	Bool	mpd_net_known ();
extern	void	mpd_net_more ();
extern	enum	ms_type	mpd_net_recv ();
extern	void	mpd_net_send ();
extern	void	mpd_net_start ();

/* trace.c */
extern	void	mpd_init_trace ();
extern	int	mpd_trace ();

/* vm.c */
extern	Vcap	mpd_crevm ();
extern	Vcap	mpd_crevm_name ();
extern	void	mpd_destvm ();
extern	void	mpd_init_vm ();
extern	void	mpd_locate ();

/* ../csw/asm.s */
extern	void	mpd_build_context ();
extern	void	mpd_check_stk ();
extern	void	mpd_chg_context ();

/* ../multi/multi.c */
extern	void	mpd_init_multiMPD ();
extern	void	mpd_jobserver_first ();
extern	void	mpd_create_jobservers ();

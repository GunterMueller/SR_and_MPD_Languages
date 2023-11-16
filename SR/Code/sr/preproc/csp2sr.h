/*  csp2sr.h -- macro definitions for csp2sr  */

#if defined(__STDC__) || defined(__sgi) || defined(_AIX)
#define PASTE(a,b) a##b
#define PASTE3(a,b,c) a##b##c
#define PASTE4(a,b,c,d) a##b##c##d
#define LITERAL(x) #x
#else
#define IDENT(s) s
#define PASTE(a,b) IDENT(a)b
#define PASTE3(a,b,c) PASTE(a,b)c
#define PASTE4(a,b,c,d) PASTE3(a,b,c)d
#define LITERAL(x) "x"
#endif

/* generate near-comments to help relate the generated code to the
 * input code.  postprocessing by sed(1) turns these into true comments
 * (which we can't do here according to the rules of ANSI C).
 */
#define SLINE /%--__LINE__--%/

/* np is number of processes.
 * processes are assigned numbers 1, 2, ...
 * allocate important structures after seen all processes
 * so don't need to bound max number of processes.
 * however, do bound max number of process decls.
 * nt is number of types.
 * operation types are assigned numbers 1, 2, ...
 * request record contains:
 *   with    -- process id with whom trying to comunicate
 *   io      -- input (true) or output (false)
 *   ioty    -- type of message
 *   arm     -- arm number within if or do stmt
 *   q1, q2  -- quantifier values
 *              (so can use those values in "user" code)
 */
#define _program(name) \
SLINE \
resource name(); \
  const csp_implicit := TERMINATIONDISC = T_IM; \
  type csp_rr = rec(with: int; io: bool; ioty: int; \
                    arm, q1, q2: int; \
                    pre, nex: ptr csp_rr); \
  op csp_match(who: int); \
  op csp_die(who: int); \
  op csp_reinit(who: int); \
  op csp_insert(who, with: int; io: bool; ioty, arm, q1, q2: int); \
  op csp_delete(p: ptr csp_rr); \
  sem csp_err := 1; \
  V(csp_err); P(csp_err)

/* coordinator terminates itself when no csp process is still active
 * so it isn't output as a "blocked process".
 */
#define _program_end \
SLINE \
  proc csp_reinit(who); \
    var p := csp_rhead[who].nex; \
    do p != @csp_rtail[who] -> \
      var q := p^.nex; \
      free(p); \
      p := q; \
    od; \
    csp_rhead[who].pre := csp_rtail[who].nex := null; \
    csp_rhead[who].nex := @csp_rtail[who]; \
    csp_rtail[who].pre := @csp_rhead[who]; \
  end; \
  proc csp_insert(who, with, io, ioty, arm, q1, q2); \
    var p: ptr csp_rr; \
    p := new(csp_rr); \
    p^.with := with; p^.io := io; p^.ioty := ioty; \
    p^.arm := arm; p^.q1 := q1; p^.q2 := q2; \
    p^.pre := csp_rtail[who].pre; \
    p^.nex := p^.pre^.nex; \
    p^.pre^.nex := csp_rtail[who].pre := p; \
  end; \
  proc csp_delete(p); \
    p^.pre^.nex := p^.nex; \
    p^.nex^.pre := p^.pre; \
    free(p); \
  end; \
  process csp_coordinator; \
    var status[csp_np-1]: enum(ALIVE,BLOCKED,DEAD) \
        := ([csp_np-1] ALIVE); \
    var active := csp_np-1; \
    do active > 0 -> \
      in csp_match(who) -> \
        var some := true; \
        var rp: ptr csp_rr; \
        /* first throw out requests for process that have died. */ \
        if csp_implicit -> \
          rp := csp_rhead[who].nex; \
          do rp != @csp_rtail[who] -> \
            var other := rp^.with; \
            var rq := rp^.nex; \
            if status[other] = DEAD -> \
              csp_delete(rp); \
            fi; \
            rp := rq; \
          od; \
          if csp_rhead[who].nex = @csp_rtail[who] -> \
            send csp_reply[who](0,0,0,0); \
            some := false; \
          fi; \
        fi; \
        if some -> \
          var matched := false; \
          rp := csp_rhead[who].nex; \
          do rp != @csp_rtail[who] -> \
            var other := rp^.with; \
            if status[other] = BLOCKED -> \
              if csp_rhead[who].nex = @csp_rtail[who] -> \
                write("oops: empty other"); \
                stop(1); \
              fi; \
              var sp: ptr csp_rr; \
              sp := csp_rhead[other].nex; \
              do sp != @csp_rtail[other] -> \
                if   (sp^.with = who) & \
                     (rp^.io xor sp^.io) & \
                     (rp^.ioty = sp^.ioty) -> \
                     send csp_reply[who](other,rp^.arm,rp^.q1,rp^.q2); \
                     send csp_reply[other](who,sp^.arm,sp^.q1,sp^.q2); \
                     status[other] := ALIVE; \
                     active++; \
                     matched := true; \
                     exit; \
                fi; \
                sp := sp^.nex; \
              od; \
              if matched -> exit; fi; \
            fi; \
            rp := rp^.nex; \
          od; \
          if not matched -> \
            status[who] := BLOCKED; \
            active--; \
          fi; \
        fi; \
      [] csp_die(who) -> \
        status[who] := DEAD; \
        active--; \
        if csp_implicit -> \
          /* for implicit termination only \
           * remove who from all pending requests. \
           * if pending set now empty, return 0. \
           */ \
          fa k := 1 to csp_np-1 st status[k] = BLOCKED -> \
            var rp: ptr csp_rr; \
            rp := csp_rhead[k].nex; \
            do rp != @csp_rtail[k] -> \
              var rq := rp^.nex; \
              if rp^.with = who -> \
                csp_delete(rp); \
              fi; \
              rp := rq; \
            od; \
            if csp_rhead[k].nex = @csp_rtail[k] -> \
              send csp_reply[k](0,0,0,0); \
              status[k] := ALIVE; \
              active++; \
            fi; \
          af; \
        fi; \
      ni; \
    od; \
  end; \
end

#define _specs \
SLINE \
  var csp_np := 1, csp_nt := 1; \
  type csp_pdecl = rec(l1, u1, l2, u2: int); \
  const csp_maxpdecls := 200; \
  var csp_npdecls := 0; \
  var csp_pdecls[csp_maxpdecls]: csp_pdecl;

/* now allocate space for stuff that depends on number of processes. */
#define _specs_end \
SLINE \
  type csp_idx = rec(i1, i2: int); \
  op csp_reply[csp_np-1](with, arm, q1, q2: int); \
  var csp_pidx[csp_np-1]: csp_idx; \
  var csp_k := 1; \
  fa csp_p := 1 to csp_npdecls -> \
    fa csp_i1 := csp_pdecls[csp_p].l1 to csp_pdecls[csp_p].u1, \
       csp_i2 := csp_pdecls[csp_p].l2 to csp_pdecls[csp_p].u2 -> \
      csp_pidx[csp_k++] := csp_idx(csp_i1,csp_i2); \
    af; \
  af; \
  var csp_rhead[csp_np-1]: csp_rr; \
  var csp_rtail[csp_np-1]: csp_rr; \
  fa csp_i := 1 to csp_np-1 -> \
    csp_rhead[csp_i].pre := csp_rtail[csp_i].nex := null; \
    csp_rhead[csp_i].nex := @csp_rtail[csp_i]; \
    csp_rtail[csp_i].pre := @csp_rhead[csp_i]; \
  af;

#define _save_pidx(name, l_1, u_1, l_2, u_2) \
csp_pdecls[++csp_npdecls] := csp_pdecl(l_1,u_1,l_2,u_2); \
const PASTE4(csp_,name,_,l1) := l_1; \
const PASTE4(csp_,name,_,u1) := u_1; \
const PASTE4(csp_,name,_,l2) := l_2; \
const PASTE4(csp_,name,_,u2) := u_2;

/* only for debugging csp2sr */
#define _dump_pidx \
fa csp_i1 := 1 to csp_np-1 -> \
  write(csp_i1, csp_pidx[csp_i1].i1, csp_pidx[csp_i1].i2); \
af;


#define _process_spec2(name, l_1, u_1, l_2, u_2) \
SLINE \
var name[l_1:u_1,l_2:u_2]: int; \
fa csp_i1 := l_1 to u_1, csp_i2 := l_2 to u_2 -> \
  name[csp_i1,csp_i2] := csp_np++; \
af; \
_save_pidx(name,l_1,u_1,l_2,u_2)

#define _process_spec1(name, l_1, u_1) \
SLINE \
var name[l_1:u_1]: int; \
fa csp_i := l_1 to u_1 -> \
  name[csp_i] := csp_np++; \
af; \
_save_pidx(name,l_1,u_1,1,1)

#define _process_spec(name) \
SLINE \
var name := csp_np++; \
_save_pidx(name, 1, 1, 1, 1)

/* note: op names must be globally unique.
 * could relax that by pasting process name
 * but that would complicate invocation.
 * specifically, would need to separate bounds
 * from process names so can do pasting.
 */
#define _port(pname,oname,ospec) \
SLINE \
op oname \
[ PASTE4(csp_,pname,_,l1) : PASTE4(csp_,pname,_,u1) , \
  PASTE4(csp_,pname,_,l2) : PASTE4(csp_,pname,_,u2) ] \
ospec; \
const PASTE(csp_type_,oname) := csp_nt++;

/* assign each process its own process id. */
#define _process_body2(name, v1, v2) \
SLINE \
process PASTE(csp_,name) \
    ( v1 := PASTE4(csp_,name,_,l1) to PASTE4(csp_,name,_,u1) , \
     v2 :=  PASTE4(csp_,name,_,l2) to PASTE4(csp_,name,_,u2) ) ; \
  var csp_pid := name[v1,v2]

#define _process_body1(name, v1) \
SLINE \
process PASTE(csp_,name) \
    ( v1 := PASTE4(csp_,name,_,l1) to PASTE4(csp_,name,_,u1) ) ;\
  var csp_pid := name[v1]

#define _process_body(name) \
SLINE \
process PASTE(csp_,name); \
  var csp_pid := name

#define _process_end \
SLINE \
  send csp_die(csp_pid); \
end

/* input stmt; pname is process name, e.g., B, B[3], B[i,j], of sender.
 * ouse is operation of receiver; args are its parameters.
 * use temp in case pname eval has side effects (though it shouldn't)
 * csp_arm, csp_q1, csp_q2 aren't used here.
 *
 * for consistent output & to make vsuite pass, ensure no blank
 * before pname arg when *calling* this macro.
 */
#define _stmt_iq2(v1,l1,u1, v2,l2,u2, pname,ouse,args) \
SLINE \
begin; \
  var csp_with, csp_arm, csp_q1, csp_q2: int; \
  csp_reinit(csp_pid); \
  fa v1 := l1 to u1, v2 := l2 to u2 -> \
    csp_insert( csp_pid, pname, true, PASTE(csp_type_,ouse), 0, 0, 0 ); \
  af; \
  send csp_match(csp_pid); \
  receive csp_reply[csp_pid](csp_with,csp_arm,csp_q1,csp_q2); \
  if csp_with = 0 -> \
    P(csp_err); \
    write("cannot match input stmt (user error)", \
        LITERAL(pname), LITERAL(ouse), LITERAL(args)); \
    stop(1); \
    V(csp_err); \
  fi; \
  var csp_t: csp_idx; \
  csp_t := csp_pidx[csp_pid]; \
  receive ouse [csp_t.i1, csp_t.i2] args; \
end;

#define _stmt_iq1(v1,l1,u1, pname,ouse,args) \
_stmt_iq2(v1,l1,u1,csp_v2,1,1,pname,ouse,args)

#define _stmt_i(pname,ouse,args) \
_stmt_iq2(csp_v1,1,1,csp_v2,1,1,pname,ouse,args)

/* output stmt; pname is process name, e.g., B, B[3], B[i,j], of receiver.
 * ouse is operation of receiver; args are its parameters.
 * use temp in case pname eval has side effects (though it shouldn't)
 * csp_arm, csp_q1, csp_q2 aren't used here.
 */
#define _stmt_oq2(v1,l1,u1, v2,l2,u2, pname,ouse,args) \
SLINE \
begin; \
  var csp_with, csp_arm, csp_q1, csp_q2: int; \
  csp_reinit(csp_pid); \
  fa v1 := l1 to u1, v2 := l2 to u2 -> \
    csp_insert( csp_pid, pname, false, PASTE(csp_type_,ouse), 0, 0, 0 ); \
  af; \
  send csp_match(csp_pid); \
  receive csp_reply[csp_pid](csp_with,csp_arm,csp_q1,csp_q2); \
  if csp_with = 0 -> \
    P(csp_err); \
    write("cannot match output stmt (user error)", \
        LITERAL(pname), LITERAL(ouse), LITERAL(args)); \
    stop(1); \
    V(csp_err); \
  fi; \
  var csp_t: csp_idx; \
  csp_t := csp_pidx[csp_with]; \
  send ouse [csp_t.i1, csp_t.i2] args; \
end;

#define _stmt_oq1(v1,l1,u1, pname,ouse,args) \
_stmt_oq2(v1,l1,u1, csp_v2,1,1, pname,ouse,args)

#define _stmt_o(pname,ouse,args) \
_stmt_oq2(csp_v1,1,1, csp_v2,1,1, pname,ouse,args)

/* csp if stmt -- i.e., containing i/o commands in guards.
 * generate prologue and epilogue for rest of stmt.
 * _if generates "begin; if true -> if true"
 * since guards require "->" after guard
 * and _fi always generate "fi; fi; end".
 * that's needed to include the stmt list code (i.e., "user" code) within if.
 */
#define _if \
SLINE \
begin; \
  var csp_with, csp_q1, csp_q2: int; \
  var csp_arm := 0; \
  var csp_gfound := false, csp_iofound := false; \
  csp_reinit(csp_pid); \
  fa csp_pass := 1 to 2 -> \
    var csp_arm_cnt := 0; \
    begin; \
      if true -> \
        if true -> \

#define _fi \
SLINE \
  _fi_od; \
end;

#define _fi_od \
        fi; \
      fi; \
    end; \
    if csp_pass = 1 & not csp_gfound & csp_iofound -> \
      send csp_match(csp_pid); \
      receive csp_reply[csp_pid](csp_with,csp_arm,csp_q1,csp_q2); \
      if csp_with = 0 -> exit; fi; \
    fi; \
  af;

#define _do \
SLINE \
do true -> \
  _if;

#define _od \
SLINE \
    _fi_od; \
    if not csp_gfound & csp_arm = 0 -> exit; fi; \
  end; \
od; 



/* code for input and output same except input generates a receive
 * output generates a send.
 * cannot combine easily (e.g., generate if to do receive or send)
 * since args need to be present in both cases.
 */
#define _guard_iq2(v1,l1,u1, v2,l2,u2, guard,pname,ouse,args) \
SLINE \
      fi; \
    fi; \
  end; \
  begin; \
    var csp_my_arm := ++csp_arm_cnt; \
    if csp_pass = 1 & not csp_gfound -> \
      fa v1 := l1 to u1, v2 := l2 to u2 st guard -> \
        csp_iofound := true; \
        csp_insert( csp_pid, pname, true, PASTE(csp_type_,ouse), csp_my_arm, v1, v2 ); \
      af; \
    [] csp_pass = 2 & csp_arm = csp_my_arm -> \
      var csp_t: csp_idx; \
      csp_t := csp_pidx[csp_pid]; \
      receive ouse [csp_t.i1, csp_t.i2] args; \
      var v1 := csp_q1; var v2 := csp_q2; \
      if true

#define _guard_iq1(v1,l1,u1, guard,pname,ouse,args) \
_guard_iq2(v1,l1,u1, csp_v2,1,1, guard, pname,ouse,args)

#define _guard_i(guard,pname,ouse,args) \
_guard_iq2(csp_v1,1,1, csp_v2,1,1, guard, pname,ouse,args)


#define _guard_oq2(v1,l1,u1, v2,l2,u2, guard,pname,ouse,args) \
SLINE \
      fi; \
    fi; \
  end; \
  begin; \
    var csp_my_arm := ++csp_arm_cnt; \
    if csp_pass = 1 & not csp_gfound -> \
      fa v1 := l1 to u1, v2 := l2 to u2 st guard -> \
        csp_iofound := true; \
        csp_insert( csp_pid, pname, false, PASTE(csp_type_,ouse), csp_my_arm, v1, v2 ); \
      af; \
    [] csp_pass = 2 & csp_arm = csp_my_arm -> \
      var csp_t: csp_idx; \
      csp_t := csp_pidx[csp_with]; \
      send ouse [csp_t.i1, csp_t.i2] args; \
      var v1 := csp_q1; var v2 := csp_q2; \
      if true

#define _guard_oq1(v1,l1,u1, guard,pname,ouse,args) \
_guard_oq2(v1,l1,u1, csp_v2,1,1, guard, pname,ouse,args)

#define _guard_o(guard,pname,ouse,args) \
_guard_oq2(csp_v1,1,1, csp_v2,1,1, guard, pname,ouse,args)

/* guards with no I/O commands.
 * allow quantifiers here too.
 * check if guard is true on first pass;
 * if so, then execute statement list and get out.
 */
#define _guard_q2(v1,l1,u1, v2,l2,u2, guard) \
SLINE \
      fi; \
    fi; \
  end; \
  begin; \
    var csp_my_arm := ++csp_arm_cnt; \
    if csp_pass = 1 & not csp_gfound -> \
      fa v1 := l1 to u1, v2 := l2 to u2 st guard -> \
        csp_q1 := v1; csp_q2 := v2; \
        csp_arm := csp_my_arm; \
        csp_gfound := true; \
        exit; \
      af; \
    [] csp_pass = 2 & csp_arm = csp_my_arm -> \
      var v1 := csp_q1; var v2 := csp_q2; \
      if true

#define _guard_q1(v1,l1,u1, guard) \
_guard_q2(v1,l1,u1, csp_v2,1,1, guard)

#define _guard(guard) \
_guard_q2(csp_v1,1,1, csp_v2,1,1, guard)


/* reset line since this .h file comes before other source.
 * so numbers given by SLINE come out ok.
 */
#line 1

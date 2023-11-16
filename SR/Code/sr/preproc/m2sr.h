/*  m2sr.h -- macro definitions for m2sr  */

#if defined(__STDC__) || defined(__sgi) || defined(_AIX)
#define PASTE(a,b) a##b
#define LITERAL(x) #x
#else
#define IDENT(s) s
#define PASTE(a,b) IDENT(a)b
#define LITERAL(x) "x"
#endif

#ifndef SIGNALDISC
#define SIGNALDISC SC
#endif

/* generate near-comments to help relate the generated code to the
 * input code.  postprocessing by sed(1) turns these into true comments
 * (which we can't do here according to the rules of ANSI C).
 */
#define SLINE /%--__LINE__--%/


#define _monitor(name) SLINE global name


/* note: always define signal_all and urgentq (and increment m_n_urgentq) but
 *   only SC uses signal_all
 *   only SU uses urgentq (others do increment m_n_urgentq though)
 * note: always send/receive from mutex and urgentq
 * so that don't get errors from compiler
 * in (unlikely) case that monitor has no procedures.
 */
#define _body(x) SLINE body x; \
type m_cv_type = rec( \
                     m_wait: cap(return_to_caller:cap(); rank: int); \
                     m_wait_ranks: cap(rank : int); \
                     m_signal: cap() returns proc_waiting:bool; \
                     m_signal_all: cap(); \
                     m_empty: cap() returns results:bool; \
                     m_minrank: cap() returns rank:int; \
                     m_print: cap(); ); \
op m_condvar( name:string[*]) returns m_cv : m_cv_type; \
sem m_mutex := 1; \
op m_urgentq(); \
var m_n_urgentq := 0; \
V(m_mutex); P(m_mutex); \
V(m_urgentq); P(m_urgentq)

#define _condvar(x) \
SLINE \
  var x: m_cv_type; \
  x := m_condvar(LITERAL(x))

/* for 1 or 2 dimensions, include subscripts as part of name
 * passed to m_condvar that is used for printing.
 * any subscript type is ok, but subscripts are printed as ints.
 */
#define _condvar1(x,s) \
SLINE \
  var x [s] : m_cv_type; \
  fa i := lb(x) to ub(x) -> \
    var si: string[20]; \
    sprintf(si, "[%d]", int(i)); \
    x[i] := m_condvar( LITERAL(x) || si ) \
  af

#define _condvar2(x,s,t) \
SLINE \
  var x [s,t]: m_cv_type; \
  fa i := lb(x) to ub(x), \
     j := lb(x,2) to ub(x,2) -> \
    var sij: string[40]; \
    sprintf(sij, "[%d,%d]", int(i), int(j)); \
    x[i,j] := m_condvar( LITERAL(x) || sij ) \
  af

/* first send and receive is just to make sure that op is used;
 * otherwise would get error from compiler.
 * SC and SX don't use return value,
 * but it is expedient to be consistent with others.
 * so, the generated code assigns the return value to the variable m_bozo;
 * and generates an assignment that uses m_bozo
 * to suppress unused warning from compiler.
 * (m_bozo only needed for SC and SX.)
 */
#define _proc(x) SLINE \
proc x \
  op m_return_from_wait(); \
  send m_return_from_wait(); \
  receive m_return_from_wait(); \
  receive m_mutex(); \
  var m_bozo: bool; m_bozo := m_bozo

/* could use SU code for _proc_end for all, but don't. */
#if SIGNALDISC==SU
#define _proc_end SLINE \
  if m_n_urgentq > 0 -> \
    m_n_urgentq--; \
    send m_urgentq(); \
  [] else -> \
    send m_mutex(); \
  fi; \
end
#else
#define _proc_end SLINE \
  send m_mutex(); \
  end
#endif

#if SIGNALDISC==SC
#define _pri_wait(x,y) \
SLINE \
  send x.m_wait(m_return_from_wait,y); \
  send x.m_wait_ranks(y); \
  send m_mutex(); \
  receive m_return_from_wait(); \
  receive m_mutex();

#define _signal(x) \
SLINE \
  m_bozo := x.m_signal();

#define _signal_all(x) \
SLINE \
  x.m_signal_all();
#endif

#if SIGNALDISC==SX
#define _pri_wait(x,y) \
SLINE \
  send x.m_wait(m_return_from_wait,y); \
  send x.m_wait_ranks(y); \
  send m_mutex(); \
  receive m_return_from_wait(); \

/* enclose in if statement in attempt to avoid unreachable code
 * messages that come in case _signal comes right before _proc_end.
 */
#define _signal(x) \
SLINE \
  if true -> \
    if not x.m_signal() -> \
      send m_mutex(); \
    fi; \
    return; \
  fi;

#define _signal_all(x) \
SLINE \
  ERROR "_signal_all is not defined for signal and exit discipline"
/* quotes around _signal_all needed to prevent infinite looping! */
#endif

#if SIGNALDISC==SW
#define _pri_wait(x,y) \
SLINE \
  send x.m_wait(m_return_from_wait,y); \
  send x.m_wait_ranks(y); \
  send m_mutex(); \
  receive m_return_from_wait();

#define _signal(x) \
SLINE \
  if x.m_signal() -> \
    receive m_mutex(); \
  fi;

#define _signal_all(x) \
SLINE \
  ERROR "_signal_all is not defined for signal and wait discipline"
/* quotes around _signal_all needed to prevent infinite looping! */
#endif

#if SIGNALDISC==SU
#define _pri_wait(x,y) \
SLINE \
  send x.m_wait(m_return_from_wait,y); \
  send x.m_wait_ranks(y); \
  if m_n_urgentq > 0 -> \
    m_n_urgentq--; \
    send m_urgentq(); \
  [] else -> \
    send m_mutex(); \
  fi; \
  receive m_return_from_wait();

#define _signal(x) \
SLINE \
  if x.m_signal() -> \
    receive m_urgentq() \
  fi;

#define _signal_all(x) \
SLINE \
  ERROR "_signal_all is not defined for signal and urgent wait discipline"
/* quotes around _signal_all needed to prevent infinite looping! */
#endif

#ifndef _signal
SLINE \
  ERROR bad signaldisc(SIGNALDISC) in m2sr.h
#endif

/* no SLINE on _wait since it invokes _pri_wait, which has an SLINE. */
#define _wait(x) _pri_wait(x,0)

#define _empty(x) SLINE x.m_empty()
#define _minrank(x) SLINE x.m_minrank()
#define _print(x) SLINE x.m_print()

/* only SC uses signal_all; no others should invoke it.
 *
 * idea for handling signal_all is:
 *   first awaken any and all waiting processes
 *   (m_wait and ?m_signal_all arm once for each waiting process).
 *   then grab the signal_all (m_signal_all arm once).
 *
 * (could use forward instead of some sends below...)
 *
 * code takes advantage of fact that order of invocations
 * of m_wait_ranks does not matter since always access "by rank"
 * except when just scanning list.
 *
 * for printing, as print each, copy it to order operation; then copy back
 * order of m_wait_ranks changed, but that does not
 * matter since always grab using by rank.
 * (instead of copying back, could use an array and swap "current" index.)
 */
#define _monitor_end SLINE \
  proc m_condvar(name) returns m_cv; \
    op m_wait(return_to_caller:cap(); rank: int); \
    op m_wait_ranks(rank : int); \
    op m_signal() returns proc_waiting:bool; \
    op m_signal_all(); \
    op m_empty() returns results:bool; \
    op m_minrank() returns rank:int; \
    op m_print(); \
    m_cv.m_wait := m_wait; \
    m_cv.m_wait_ranks := m_wait_ranks; \
    m_cv.m_signal := m_signal; \
    m_cv.m_signal_all := m_signal_all; \
    m_cv.m_empty := m_empty; \
    m_cv.m_minrank := m_minrank; \
    m_cv.m_print := m_print; \
    reply; \
    do true -> \
      in m_wait(return_to_caller, rank) and ?m_signal != 0 by rank -> \
        in m_wait_ranks(rank) by rank ->  ni; \
        in m_signal() returns pw -> pw := true; m_n_urgentq++; ni; \
        send return_to_caller(); \
      [] m_signal() returns pw and ?m_wait = 0 -> \
        pw := false; \
      [] m_wait(return_to_caller, rank) and ?m_signal_all != 0 by rank -> \
         in m_wait_ranks(rank) by rank ->  ni; \
         send return_to_caller(); \
      [] m_signal_all() and ?m_wait = 0 -> \
         skip \
      [] m_empty() returns results -> \
         results := (?m_wait = 0); \
      [] m_minrank() returns ret -> \
         if ?m_wait_ranks != 0 -> \
           in m_wait_ranks( rank) by rank -> \
             send m_wait_ranks( rank); \
             ret := rank; \
           ni \
         [] else -> \
           write("\t**** minrank called on empty Q ****"); \
           flush(stdout); \
           ret := 999999; \
         fi \
      [] m_print() -> \
        writes("\t**** Printing for ", name, " **** "); \
        var x := ?m_wait; \
        writes(x, " waiting process(es)"); \
        if x > 0 ->; \
          var r, minr: int; var same:= true; \
          receive m_wait_ranks(r); minr := r; \
          send m_wait_ranks(minr); \
          fa i := 2 to x -> \
            receive m_wait_ranks(r); \
            send m_wait_ranks(r); \
            same := same and (r = minr); \
          af; \
          if same -> \
            writes( ", all with rank ", r ); \
          [] else -> \
            op order(rank:int); \
            write( ", with ranks:" ); \
            fa i := 1 to x -> \
              in m_wait_ranks( rank) by rank -> \
                writes(" ", rank); \
                send order( rank) \
              ni; \
            af; \
            fa i := 1 to x -> \
              in order( rank) -> \
                send m_wait_ranks( rank) \
              ni; \
            af; \
          fi; \
        fi; \
        write(); \
        flush(stdout); \
      ni; \
    od; \
  end; \
end

/* reset line since this .h file comes before other source.
 * so numbers given by SLINE come out ok.
 */
#line 1

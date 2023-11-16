/*  ccr2sr.h -- macro definitions for ccr2sr  */

#if defined(__STDC__) || defined(__sgi) || defined(_AIX)
#define PASTE(a,b) a##b
#else
#define IDENT(s) s
#define PASTE(a,b) IDENT(a)b
#endif

/* generate near-comments to help relate the generated code to the
 * input code.  postprocessing by sed(1) turns these into true comments
 * (which we can't do here according to the rules of ANSI C).
 */
#define SLINE /%--__LINE__--%/

/* creating the pasted names is not really needed,
 * but that evolved from an earlier version;
 * makes code a little more readable?
 */

#define _resource(name) \
SLINE \
  global name; \
    sem PASTE(r_e,name) := 1; \
    sem PASTE(r_d,name), PASTE(r_g,name); \
    var PASTE(r_nd,name) := 0, PASTE(r_ng,name) := 0;
#define _resource_end(name) \
SLINE \
  end name

#define _resource1(name,l1,u1) \
SLINE \
  global name; \
    sem PASTE(r_e,name)[l1:u1] := ([(u1)-(l1)+1] 1); \
    sem PASTE(r_d,name)[l1:u1], PASTE(r_g,name)[l1:u1]; \
    var PASTE(r_nd,name)[l1:u1] := ([(u1)-(l1)+1] 0); \
    var PASTE(r_ng,name)[l1:u1] := ([(u1)-(l1)+1] 0);
#define _resource_end1(name) \
SLINE \
  end name

#define _resource2(name,l1,u1,l2,u2) \
SLINE \
  global name; \
    sem PASTE(r_e,name)[l1:u1,l2:u2] := ([(u1)-(l1)+1]([(u2)-(l2)+1] 1)); \
    sem PASTE(r_d,name)[l1:u1,l2:u2], PASTE(r_g,name)[l1:u1,l2:u2]; \
    var PASTE(r_nd,name)[l1:u1,l2:u2] := ([(u1)-(l1)+1]([(u2)-(l2)+1] 0)); \
    var PASTE(r_ng,name)[l1:u1,l2:u2] := ([(u1)-(l1)+1]([(u2)-(l2)+1] 0));
#define _resource_end2(name) \
SLINE \
  end name

/* instead of inline code below,
 * could use one procedure for "EXIT1; DELAY"
 * and one for "EXIT2" in Andrews's text.
 */
#define _region(name,when) \
SLINE \
  begin; \
  import name; \
  P(PASTE(r_e,name)); \
  do not ( when ) -> \
    PASTE(r_nd,name)++; \
    if PASTE(r_ng,name) > 0 -> V(PASTE(r_g,name)); \
    [] else -> V(PASTE(r_e,name)); \
    fi; \
    P(PASTE(r_d,name)); PASTE(r_nd,name)--; \
    PASTE(r_ng,name)++; \
    if PASTE(r_nd,name) > 0 -> V(PASTE(r_d,name)); \
    [] else -> V(PASTE(r_g,name)); \
    fi; \
    P(PASTE(r_g,name)); PASTE(r_ng,name)--; \
  od;
#define _region_end(name) \
SLINE \
  if PASTE(r_nd,name) > 0 -> V(PASTE(r_d,name)); \
  [] else -> if PASTE(r_ng,name) > 0 -> V(PASTE(r_g,name)); \
             [] else -> V(PASTE(r_e,name)); \
             fi; \
  fi; \
  end;

#define _region1(name,v1,when) \
SLINE \
  begin; \
  import name; \
  P(PASTE(r_e,name)[v1]); \
  do not ( when ) -> \
    PASTE(r_nd,name)[v1]++; \
    if PASTE(r_ng,name)[v1] > 0 -> V(PASTE(r_g,name)[v1]); \
    [] else -> V(PASTE(r_e,name)[v1]); \
    fi; \
    P(PASTE(r_d,name)[v1]); PASTE(r_nd,name)[v1]--; \
    PASTE(r_ng,name)[v1]++; \
    if PASTE(r_nd,name)[v1] > 0 -> V(PASTE(r_d,name)[v1]); \
    [] else -> V(PASTE(r_g,name)[v1]); \
    fi; \
    P(PASTE(r_g,name)[v1]); PASTE(r_ng,name)[v1]--; \
  od;
#define _region_end1(name,v1) \
SLINE \
  if PASTE(r_nd,name)[v1] > 0 -> V(PASTE(r_d,name)[v1]); \
  [] else -> if PASTE(r_ng,name)[v1] > 0 -> V(PASTE(r_g,name)[v1]); \
             [] else -> V(PASTE(r_e,name)[v1]); \
             fi; \
  fi; \
  end;

#define _region2(name,v1,v2,when) \
SLINE \
  begin; \
  import name; \
  P(PASTE(r_e,name)[v1,v2]); \
  do not ( when ) -> \
    PASTE(r_nd,name)[v1,v2]++; \
    if PASTE(r_ng,name)[v1,v2] > 0 -> V(PASTE(r_g,name)[v1,v2]); \
    [] else -> V(PASTE(r_e,name)[v1,v2]); \
    fi; \
    P(PASTE(r_d,name)[v1,v2]); PASTE(r_nd,name)[v1,v2]--; \
    PASTE(r_ng,name)[v1,v2]++; \
    if PASTE(r_nd,name)[v1,v2] > 0 -> V(PASTE(r_d,name)[v1,v2]); \
    [] else -> V(PASTE(r_g,name)[v1,v2]); \
    fi; \
    P(PASTE(r_g,name)[v1,v2]); PASTE(r_ng,name)[v1,v2]--; \
  od;
#define _region_end2(name,v1,v2) \
SLINE \
  if PASTE(r_nd,name)[v1,v2] > 0 -> V(PASTE(r_d,name)[v1,v2]); \
  [] else -> if PASTE(r_ng,name)[v1,v2] > 0 -> V(PASTE(r_g,name)[v1,v2]); \
             [] else -> V(PASTE(r_e,name)[v1,v2]); \
             fi; \
  fi; \
  end;

/* reset line since this .h file comes before other source.
 * so numbers given by SLINE come out ok.
 */
#line 1

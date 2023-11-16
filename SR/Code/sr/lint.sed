# sed(1) script for deleting unwanted lint messages

# we know we're ignoring some return values
/ returns value which is .*s ignored/d

# meaningless message from calling malloc
/warning: possible pointer alignment problem/d

# don't worry about unused functions from ../util
/defined.*util\.c.*but never used/d

# these are inconsistent on different systems
/^exit value declared inconsistently/d
/^sprintf value declared inconsistently/d
/^wait, arg. 1 used inconsistently/d
/^signal, arg. 2 used inconsistently/d
/^select, arg. [234] used inconsistently/d

# these come from lex and yacc
/^.usr.lib.yaccpar/d
/unused in function yylex/d
/unused in function yyparse/d
/^yy.* never used/d
/^yy.* not defined/d
/^free, arg. 1 used inconsistently.*yaccpar/d
/^free, arg. 1 used inconsistently.*grammar.y/d

# these are supplied by the C library
/^errno .* not defined/d
/^optarg .* not defined/d
/^optind .* not defined/d
/^sin .* not defined/d
/^cos .* not defined/d
/^pow .* not defined/d
/^ceil .* not defined/d
/^floor .* not defined/d

# when lint is run with -x -h, get many of these:
/warning: null effect/d


# == sr subdirectory ==

# lint doesn't seem to understand varargs
/^cprintf, arg. 1 used inconsistently.*/d
/^cprintf: variable # of args*/d

# ignore unused definitions in print.c
/^print\.c.*_names unused/d
/_names .*ed( .*), but n/d

# not really a problem, just assumes C default promotions
/^freepst, arg. 2 used inconsistently/d
/^iolist, arg. 3 used inconsistently/d
/^literal, arg. 4 used inconsistently/d
/^lookup, arg. 2 used inconsistently/d

# not a problem, int vs unsigned int
/^err, arg. 1 used inconsistently/d
/^readspec, arg. 2 used inconsistently/d


# == csw subdirectory == 

# context switch routines appear undefined
/^sr_build_context .*, but not defined/d
/^sr_chg_context .*, but not defined/d

# error handlers appear unused
/^sr_stk_.* never used/d

# test routine is never really used
/^dstack .* never used/d


# == rts subdirectory ==

/^sr_spawn, arg. . used inconsistently/d

# runtime primitives show up as never used
/^sr_.* defined.*, but never used/d

# this just defines a constant in the exe file
/^version .* never used/d

# globals defined by srl show up as undefined
/^sr_.* used.*, but not defined/d

# problems due to ref to undefined (opaque) pool structure
/^sr_makepool value used inconsistently/d
/^sr_addpool, arg. 1 used inconsistently/d
/^sr_delpool, arg. 1 used inconsistently/d
/^sr_eachpool, arg. 1 used inconsistently/d

# not really a problem, just assumes C default promotions
/^sr_net_send, arg. 1 used inconsistently/d

# lint doesn't seem to understand VARARGS
/^sr_runerr, arg. . used inconsistently/d
/^sr_init_array, arg. . used inconsistently/d

# these are used under MultiSR
/_mutex unused/d
/_mutex .* never used/d
/old_status set but not used/d
/clap unused in function sr_kill_inops/d
/op set but not used in function sr_kill_inops/d

# Sequent MultiSR stuff
/parallel\/parallel.h/d
/^cpus_online .* never used/d
/^m_myid .* not defined/d
/^m_numprocs .* not defined/d


# == srwin subdirectory ==

/^SRWin_.*but never used/d
/^WinFont.*but never used/d
/^X.* not defined/d
/^XCreateWindow, arg. 8 used inconsistently.*srwin.c/d
/^calloc, arg. 1 used inconsistently.*srwin.c/d
/^memset, arg. 3 used inconsistently.*srwin.c/d
/^memset value declared inconsistently.*srwin.c/d

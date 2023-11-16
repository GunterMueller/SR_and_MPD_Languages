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


# == mpd subdirectory ==

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
/^mpd_build_context .*, but not defined/d
/^mpd_chg_context .*, but not defined/d

# error handlers appear unused
/^mpd_stk_.* never used/d

# test routine is never really used
/^dstack .* never used/d


# == rts subdirectory ==

/^mpd_spawn, arg. . used inconsistently/d

# runtime primitives show up as never used
/^mpd_.* defined.*, but never used/d

# this just defines a constant in the exe file
/^version .* never used/d

# globals defined by mpdl show up as undefined
/^mpd_.* used.*, but not defined/d

# problems due to ref to undefined (opaque) pool structure
/^mpd_makepool value used inconsistently/d
/^mpd_addpool, arg. 1 used inconsistently/d
/^mpd_delpool, arg. 1 used inconsistently/d
/^mpd_eachpool, arg. 1 used inconsistently/d

# not really a problem, just assumes C default promotions
/^mpd_net_send, arg. 1 used inconsistently/d

# lint doesn't seem to understand VARARGS
/^mpd_runerr, arg. . used inconsistently/d
/^mpd_init_array, arg. . used inconsistently/d

# these are used under MultiMPD
/_mutex unused/d
/_mutex .* never used/d
/old_status set but not used/d
/clap unused in function mpd_kill_inops/d
/op set but not used in function mpd_kill_inops/d

# Sequent MultiMPD stuff
/parallel\/parallel.h/d
/^cpus_online .* never used/d
/^m_myid .* not defined/d
/^m_numprocs .* not defined/d


# == mpdwin subdirectory ==

/^MPDWin_.*but never used/d
/^WinFont.*but never used/d
/^X.* not defined/d
/^XCreateWindow, arg. 8 used inconsistently.*mpdwin.c/d
/^calloc, arg. 1 used inconsistently.*mpdwin.c/d
/^memset, arg. 3 used inconsistently.*mpdwin.c/d
/^memset value declared inconsistently.*mpdwin.c/d

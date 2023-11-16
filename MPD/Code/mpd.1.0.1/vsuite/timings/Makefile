#  Makefile for MPD operation benchmarks
#
#  This directory holds the tests used to obtain the timing results reported
#  in Appendix E of the SR book.  For best results, use a uniprocessor version
#  of MPD built with "CFLAGS=-O -DNDEBUG".
#
#  To calculate timings, type "make MPD=path" giving the path to the version
#  of MPD to use.  Results appear in report.out.  
#
#  If a machine is too fast to get good timings as set up, increase NTIMES.
#
#  The Script files in the subdirectories are for use by mpdv.

#  The .sh files are protected as "not executable" to keep them from
#  being removed by cleanup scripts.


SHELL=/bin/sh
MPD=mpd
NTIMES=10000
NLOOPS=5


#  rerun everything from scratch

timings:  build run report


#  compute timing results and report on report.out.

report report.out: run.out
	sh report.sh >report.out

run run.out: build.out
	sh run.sh $(NTIMES) $(NLOOPS)

build build.out:
	sh build.sh $(MPD)


#  clean up files

clean:
	rm -rf *.out */*.out */MPDinter

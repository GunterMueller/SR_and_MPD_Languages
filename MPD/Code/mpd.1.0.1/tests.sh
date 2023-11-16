#!/bin/sh
#
#  list vsuite directories containing VALID MPD programs
#
#  we handle each top-level subdirectory separately, in a loop,
#  to reduce the pipeline delays vs. collecting and sorting everything

cd vsuite

for i in *
do 
  find $i -name Script -print | sort | xargs grep -l '^0 mpd ' | sed s./Script..
done

#!/bin/sh
#
#  list vsuite directories containing VALID SR programs
#
#  we handle each top-level subdirectory separately, in a loop,
#  to reduce the pipeline delays vs. collecting and sorting everything

cd vsuite

for i in *
do 
   find $i -name Script -print | sort | xargs grep -l '^0 sr ' | sed s./Script..
done

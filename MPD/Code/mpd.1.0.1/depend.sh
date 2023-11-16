: add dependencies of C files to the Makefile
: [this only works on some systems, e.g. OSF/1, not Solaris nor Linux]
set -e
sed '1,/^# >>DO NOT DELETE THIS LINE<< #/!d' Makefile	>Makefile.new
echo '#   "make depend" was last run:  '`date`		>> Makefile.new
cc -M *.c | sed 					>> Makefile.new '
    /usr.include/d
    /:$/d
    s|:[ 	]*|: |
    s|: ./|: |
'
mv Makefile Makefile.bak
mv Makefile.new Makefile

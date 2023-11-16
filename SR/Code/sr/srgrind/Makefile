##	Makefile for the SR troff grinder, srgrind.



SHELL = /bin/sh



all:	srgrind

srgrind:  ../paths.sh srgrind.sh
	echo "#!/bin/sh"					 >srgrind
	echo "#"						>>srgrind
	echo "#  srgrind - format SR file for troff"		>>srgrind
	cat ../paths.sh srgrind.sh				>>srgrind
	chmod +x srgrind

clean:

depend:

lint:

tags:

TAGS:

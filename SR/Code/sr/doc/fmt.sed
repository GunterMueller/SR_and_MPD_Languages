# sed(1) script for preprocessing troff source

s/@\([^@]*\)@/\\\&\\*C\1\\fP\\\&/g
#
s/#[a-zA-Z][a-zA-Z0-9_.]*/\\fI&\\fP/g
s/\\fI#/\\fI/g
s/%[a-zA-Z][a-zA-Z0-9_.]*/\\fB&\\fP/g
s/\\fB%/\\fB/g
s/\$[a-zA-Z][a-zA-Z0-9_.]*/\\fR&\\fP/g
s/\\fR\$/\\fR/g
s/``/\\*(lq/g
s/''/\\*(rq/g
s/|1/\\ka/g
s/ \/1/\\h'|\\nau'/g
s/\/1/\\h'|\\nau'/g
s/|2/\\kb/g
s/ \/2/\\h'|\\nbu'/g
s/|3/\\kc/g
s/ \/3/\\h'|\\ncu'/g

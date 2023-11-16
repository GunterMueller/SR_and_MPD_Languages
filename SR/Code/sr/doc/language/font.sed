s/#[a-zA-Z][a-zA-Z0-9_.]*/\\fI&\\fP/g
s/\\fI#/\\fI/g
s/#\\fP/\\fP/g
s/%[a-zA-Z][a-zA-Z0-9_.]*/\\fB&\\fP/g
s/\\fB%/\\fB/g
s/%\\fP/\\fP/g
s/@[^@]*@/\\\&\\*C&\\fP/g
s/\\\*C@/\\*C/g
s/@\\fP/\\fP/g
s/\[]/\\h'2p'[\\h'-4u']/g
# s/++/\\(pl/g
# s/--/\\(mi/g
# s/->/\\(->/g
# s/<-/\\(<-/g
# s/>=/\\(>=/g
# s/<=/\\(<=/g
# s/!=/\\(!=/g
s/\[\[[^]]*]]/\\s-2\\d&\\u\\s+2/g
s/\[\[//g
s/]]//g
s/{{[^}]*}}/\\s-2\\u&\\d\\s+2/g
s/{{//g
s/}}//g
s/``/\\*(lq/g
s/''/\\*(rq/g
s/FORALL/\\*(qa/g
s/EXISTS/\\*(qe/g
s/\^not/\\fB\\u_\\d\\h'-0.5p'\\s-6l\\s+6\\fP\\h'.2n'/g
s/\^and/\\fB\\v'-2p'\\s-5\/\\h'-8u'\\e\\s+5\\v'+2p'\\fP/g
s/\^or/\\fB\\v'-2p'\\s-5\\e\\h'-8u'\/\\s+5\\v'+2p'\\fP/g
s/=>/=\\h'-1.2n'>/g
s/<<>>/<\\h'-1.2n'=\\h'-.6n'>/g

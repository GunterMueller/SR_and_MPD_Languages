BEGIN  { 	printf("/***** Built by Makefile -- DO NOT EDIT *****/\n\n")
		printf("/*SUPPRESS 592*/  /*(for Saber C)*/\n")
		printf("/*SUPPRESS 763*/  /*(for CodeCenter)*/\n") }

/enum e_/  {	s = substr($0,index($0,"_")+1)
		s = substr(s,0,index(s," ")-1)
		printf("static char * %s_names[] = {\n",s)
		running = 1
		next
		}

/^[ 	]*[A-Z][A-Z]*_/	{
		if (running) print ("    \"" $1 "\",")
		next
		}
	
/^[ 	]*}/  {	if (running) print "};"
		running = 0
		}

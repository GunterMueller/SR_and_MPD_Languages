/*  funcs.h -- declarations of external C functions  */

#include "../gen.h"
#include "../util.h"

/* main.c */
extern	void	srl_warn	PARAMS ((char *s));
extern	void	srl_error	PARAMS ((char *s));

/* gen.c */
extern	void	gen_config	PARAMS ((NOARGS));
extern	void	gen_exe		PARAMS ((NOARGS));

/* limits.c */
extern	void	setlimit	PARAMS ((int c, char *value));
extern	void	showlimits	PARAMS ((NOARGS));
extern	void	writelimits	PARAMS ((FILE *f));

/* resource.c */
extern	void	resource	PARAMS ((char *name));

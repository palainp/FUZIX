/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include <stdlib.h>
#include	"defs.h"

static STRING *copyargs();
static DOLPTR	dolh;

CHAR	flagadr[10];

CHAR	flagchar[] = {
	'x',	'n',	'v',	't',	's',	'i',	'e',	'r',	'k',	'u',	0
};
INT	flagval[]  = {
	execpr,	noexec,	readpr,	oneflg,	stdflg,	intflg,	errflg,	rshflg,	keyflg,	setflg,	0
};

/* ========	option handling	======== */


INT	options(argc,argv)
	STRING		*argv;
	INT		argc;
{
	REG STRING	cp;
	REG STRING	*argp=argv;
	REG STRING	flagc;
	STRING		flagp;

	if (argc>1 && *argp[1]=='-'
	) {	cp=argp[1];
		flags &= ~(execpr|readpr);
		WHILE *++cp
		DO	flagc=flagchar;

			WHILE *flagc && *flagc != *cp DO flagc++ OD
			if (*cp == *flagc
			) {	flags |= flagval[flagc-flagchar];
			} else if (*cp=='c' && argc>2 && comdiv==0
			) {	comdiv=argp[2];
				argp[1]=argp[0]; argp++; argc--;
			} else {	failed(argv[1],badopt);
			;}
		OD
		argp[1]=argp[0]; argc--;
	;}

	/* set up $- */
	flagc=flagchar;
	flagp=flagadr;
	WHILE *flagc
	DO if (flags&flagval[flagc-flagchar]
	   ) { *flagp++ = *flagc;
	   ;}
	   flagc++;
	OD
	*flagp++=0;

	return(argc);
}

void	setargs(argi)
	STRING		argi[];
{
	/* count args */
	REG STRING	*argp=argi;
	REG INT		argn=0;

	WHILE Rcheat(*argp++)!=ENDARGS DO argn++ OD

	/* free old ones unless on for loop chain */
	freeargs(dolh);
	dolh=(DOLPTR)copyargs(argi,argn);	/* sets dolv */
	assnum(&dolladr,dolc=argn-1);
}

DOLPTR
freeargs(blk)
	DOLPTR		blk;
{
	REG STRING	*argp;
	REG DOLPTR	argr=0;
	REG DOLPTR	argblk;

	if (argblk=blk
	) {	argr = argblk->dolnxt;
		if ((--argblk->doluse)==0
		) {	FOR argp=(STRING *)argblk->dolarg; Rcheat(*argp)!=ENDARGS; argp++
			DO free(*argp) OD
			free(argblk);
		;}
	;}
	return(argr);
}

static STRING *	copyargs(from, n)
	STRING		from[];
{
	REG STRING *	np=(STRING *)alloc(sizeof(STRING*)*n+3*BYTESPERWORD);
	REG STRING *	fp=from;
	REG STRING *	pp=np;

	((DOLPTR)np)->doluse=1;	/* use count */
	np=(STRING *)((DOLPTR)np)->dolarg;
	dolv=np;

	WHILE n--
	DO *np++ = make(*fp++) OD
	*np++ = ENDARGS;
	return(pp);
}

clearup()
{
	/* force `for' $* lists to go away */
	WHILE argfor=freeargs(argfor) DONE

	/* clean up io files */
	WHILE pop() DONE
}

DOLPTR	useargs()
{
	if (dolh
	) {	dolh->doluse++;
		dolh->dolnxt=argfor;
		return(argfor=dolh);
	} else {	return(0);
	;}
}

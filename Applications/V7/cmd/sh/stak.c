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

#include	"defs.h"

STKPTR		stakbot=nullstr;



/* ========	storage allocation	======== */

STKPTR	getstak(asize)
	INT		asize;
{	/* allocate requested stack */
	REG STKPTR	oldstak;
	REG INT		size;

	size=round(asize,BYTESPERWORD);
	oldstak=stakbot;
	staktop = stakbot += size;
	return(oldstak);
}

STKPTR	locstak()
{	/* set up stack for local use
	 * should be followed by `endstak'
	 */
	if(brkend-stakbot<BRKINCR
	) {	setbrk(brkincr);
		if(brkincr < BRKMAX
		) {	brkincr += 256;
		;}
	;}
	return(stakbot);
}

STKPTR	savstak()
{
	assert(staktop==stakbot);
	return(stakbot);
}

STKPTR	endstak(argp)
	REG STRING	argp;
{	/* tidy up after `locstak' */
	REG STKPTR	oldstak;
	*argp++=0;
	oldstak=stakbot; stakbot=staktop=(STKPTR)round(argp,BYTESPERWORD);
	return(oldstak);
}

void	tdystak(x)
	REG STKPTR 	x;
{
	/* try to bring stack back to x */
	WHILE ADR(stakbsy)>ADR(x)
	DO free(stakbsy);
	   stakbsy = stakbsy->word;
	OD
	staktop=stakbot=max(ADR(x),ADR(stakbas));
	rmtemp(x);
}

stakchk()
{
	if((brkend-stakbas)>BRKINCR+BRKINCR
	) {	setbrk(-BRKINCR);
	;}
}

STKPTR	cpystak(x)
	STKPTR		x;
{
	return(endstak(movstr(x,locstak())));
}

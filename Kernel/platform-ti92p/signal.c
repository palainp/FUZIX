#include <kernel.h>
#include "globals.h"

void gsignal(int pgrp, int sig)
{
	struct proc *p;
	int x;
	
	if (pgrp == 0)
		return;
	mask(&G.calloutlock);
	list_for_each_entry(p, &G.proc_list, p_list)
		if (p->p_pgrp == pgrp)
			procsignal(p, sig);
	unmask(&G.calloutlock);
}

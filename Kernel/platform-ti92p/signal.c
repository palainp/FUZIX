#include <kernel.h>
#include "globals.h"

/*
 * This arranges for func(arg) to be called in time/HZ seconds.
 * The callout array is sorted in order of times as a delta list.
 * End of the callout array is marked with a sentinel entry (c_func == NULL).
 */
int timeout(void (*func)(void *), void *arg, long time)
{
	struct callout *c1, *c2;
	long t;
	int err = 0;
	int x;
	
	t = time;
	c1 = &G.callout[0];
	x = spl7();
	
	//kprintf("timeout: adding a timeout in %ld ticks\n", time);
	while (c1->c_func != NULL && c1->c_dtime <= t) {
		t -= c1->c_dtime;
		++c1;
	}
	
	c2 = c1;
	
	/* find the sentinel (NULL entry) */
	while (c2->c_func != NULL)
		++c2;
	
	/* any room to put this new entry? */
	if (c2 >= &G.callout[NCALL-1]) {
		err = -1;
		goto out;
	}
	
	if (c1->c_func)
		c1->c_dtime -= t;
	
	/* move entries upward to make room for this new entry */
	do {
		c2[1] = c2[0];
		--c2;
	} while (c2 >= c1);
	
	c1->c_dtime = t;
	c1->c_func = func;
	c1->c_arg = arg;
	
out:
	splx(x);
	return err;
}

/*
 * Remove a pending callout.
 * Return zero if no timeout was removed, or non-zero if one was removed.
 */
int untimeout(void (*func)(void *), void *arg)
{
	struct callout *cp;
	int x;
	int canhastimeout = 0;
	x = spl7();
	for (cp = &G.callout[0]; cp < &G.callout[NCALL]; ++cp) {
		if (cp->c_func == func && cp->c_arg == arg) {
			canhastimeout = 1;
			if (cp < &G.callout[NCALL-1] && cp[1].c_func)
				cp[1].c_dtime += cp[0].c_dtime;
			while (cp < &G.callout[NCALL-1]) {
				*cp = *(cp+1);
				++cp;
			}
			G.callout[NCALL-1].c_func = NULL;
			break; /* remove only the first timeout */
		}
	}
	splx(x);
	return canhastimeout;
}

///* schedule a task until after all interrupts are handled */
//int defer(void (*func)(void *), void *arg)
//{
//	return timeout(func, arg, 0);
//}
//
///* unschedule a deferred task */
//int undefer(void (*func)(void *), void *arg)
//{
//	return untimeout(func, arg);
//}

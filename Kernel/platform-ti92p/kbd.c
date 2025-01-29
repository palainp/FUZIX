#include "globals.h"

/* set the bell status and set a timeout to remove the bell status after
 * some time */
void bell(struct tty *ttyp)
{
	untimeout(unbell, ttyp);
	G.vt.bell = 1;
	showstatus();
	timeout(unbell, ttyp, BELLTIMEOUT);
}

void showstatus(void)
{
	/* TI-92+: */
	/* scroll-lock compose hand capslock shift diamond 2nd busy bell */
	/* 8           7       6    5        4     3       2   1    0    */
	
	int batt;
	//int x = spl7();
	mask(&G.calloutlock);
	int mod = G.vt.key_mod | G.vt.key_mod_sticky;
	
	batt = G.batt_level - 3;
	if (batt < 0) batt = 0;
	
	drawmod(0, G.vt.bell ? STATUS_BELL : STATUS_BATT0+batt);
	drawmod(1, G.cpubusy ? STATUS_BUSY : STATUS_NONE);
	drawmod(2, mod & KEY_2ND ? STATUS_2ND : STATUS_NONE);
	drawmod(3, mod & KEY_DIAMOND ? STATUS_DIAMOND : STATUS_NONE);
	drawmod(4, mod & KEY_SHIFT ? STATUS_SHIFT : STATUS_NONE);
	drawmod(5, G.vt.caps_lock ? STATUS_CAPSLOCK : STATUS_NONE);
#if CALC_HAS_QWERTY_KBD
	drawmod(6, G.vt.hand_lock ? STATUS_HANDLOCK :
	           mod & KEY_HAND ? STATUS_HAND : STATUS_NONE);
#else
	drawmod(6, G.vt.alpha_lock ? STATUS_ALPHALOCK :
	           mod & KEY_ALPHA ? STATUS_ALPHA : STATUS_NONE);
#endif
	drawmod(7, G.vt.compose ? STATUS_COMPOSE1 : G.vt.key_compose ? STATUS_COMPOSE2 : STATUS_NONE);
	drawmod(8, G.vt.scroll_lock ? STATUS_SCROLLLOCK : STATUS_NONE);
	drawmod(9, STATUS_LINK0 + (G.link.rxtx & 3));
	unmask(&G.calloutlock);
	//splx(x);
}

void kbinit()
{
	timeout(scankb, NULL, 1);
}

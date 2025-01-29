#include "globals.h"

void flushtty(struct tty *tp)
{
	/* FIXME */
	qclear(&tp->t_rawq.q);
	qclear(&tp->t_canq.q);
}

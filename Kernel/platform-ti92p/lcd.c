#include "globals.h"
#include "lcd.h"

int lcd_set_contrast(int cont)
{
	int ch;
	if (cont < 0)
		cont = 0;
	else if (CONTRASTMAX < cont)
		cont = CONTRASTMAX;
#if CALC_HAS_LARGE_SCREEN
	ch = CONTRAST_VMUL | ~cont;
#else
	ch = CONTRAST_VMUL | cont;
#endif
	CONTRASTPORT = ch;
	G.contrast = cont;
	return cont;
}

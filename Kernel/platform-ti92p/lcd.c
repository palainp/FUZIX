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

int lcd_inc_contrast()
{
	return lcd_set_contrast(G.contrast+1);
}

int lcd_dec_contrast()
{
	return lcd_set_contrast(G.contrast-1);
}

int lcd_reset_contrast()
{
	return lcd_set_contrast(G.contrast);
}

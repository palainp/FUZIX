#ifndef _LCD_H
#define _LCD_H

#define LCD_MEM		((volatile char *)0x4c00)
#define LCD_INCY	30

/*
 * Bit Usage on 0x60001d
 *  7 HW1: Voltage multiplier enable. Keep set (=1).
 *  4 HW1: Screen disable (power down).
 *    HW2: LCD contrast bit 4 (MSb).
 *  3-0 LCD contrast bits 3-0 (bit 3 is MSb on HW1).
 *
 */
#define CONTRASTMAX	31
#define CONTRASTPORT	(*(volatile char *)0x60001d)
#define CONTRAST_VMUL	0100
#define CONTRAST_FIELD	0017


/* TI92 plus config */
#define CALC_HAS_QWERTY_KBD 1
#define CALC_HAS_LARGE_SCREEN 1
#define CALC_FLASH_BASE ((void *)0x400000)
#define LCD_WIDTH	240
#define LCD_HEIGHT	128

#define	NUMCELLROWS	20
#define	NUMCELLCOLS	60

int lcd_set_contrast(int cont);

#endif

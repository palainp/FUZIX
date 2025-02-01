#include <kernel.h>
#include "globals.h"
#include "lcd.h"
#include "heap.h"

void vtinit(void);


/*
 * Note 1: to keep realtime in sync when the calc powers down, we need to keep
 * track of the delta between realtime and seconds right before power down.
 * Some code to show what I mean:
 *
 * void power_off()
 * {
 * 	// other stuff that should be done before power-down (eg, sync())
 * 	delta = realtime.tv_sec - seconds;
 * 	power_down() // the low-level power-down routine
 * 	realtime.tv_sec = seconds + delta;
 * }
 *
 * If seconds doesn't increment while it is powered off (that is, if the calc
 * is powered off for less than a second), there will be some time loss
 * (<1 second) in the realtime clock. In fact, there will be time loss whenever
 * there is less time until the next "seconds" increment after power-on than
 * before power-off. On the flip side, there will be time gain when there is
 * more time until the next increment after power-on than at power-off.
 * However, these time gains and losses are symmetrical (each should happen 50%
 * of the time, theoretically), so any drift because of this should be
 * negligible (plus we are on a graphing calculator with probably inaccurate
 * hardware clock(s) anyway).
 */
void cpupoweroff()
{
	int x = spl7();
	long oldrt = realtime.tv_sec - G.seconds;
	splx(x);
	
	G.onkey = 0;
	G.powerstate = 1;
	
	LCD_ROW_SYNC = 0b00111100; // turn off row sync
	LCD_CONTRAST |= (1<<4);   // disable screen (hw1)
	LCD_CONTROL &= ~(1<<1);  // shut down LCD (hw2)
	
	while (!G.onkey)
		cpuidle(INT_3|INT_4);
	
	LCD_CONTROL |= (1<<1);
	LCD_CONTRAST &= ~(1<<4);
	LCD_ROW_SYNC = 0b00100001;
	lcd_reset_contrast(); // contrast was reset when 0x60001d (LCD_CONTRAST) was modified
	
	G.powerstate = 0;
	
	spl7();
	realtime.tv_sec = oldrt + G.seconds;
	splx(x);
}



int start(void)
{
    size_t i;
    char hello[] = "hello world.\n";

    lcd_set_contrast(CONTRASTMAX/2);
    meminit();

    for(i=0; i<13; ++i)
        kputchar(hello[i]);

	G.seconds = realtime.tv_sec;
	uptime.tv_sec = uptime.tv_nsec = 0;
    return 0;
}

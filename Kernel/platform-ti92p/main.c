#include <kernel.h>
#include "globals.h"
#include "lcd.h"
#include "heap.h"

void vtinit(void);

int start(void)
{
    di();
    lcd_set_contrast(CONTRASTMAX/2);
    meminit();
//    vtinit();

    return 0;
}

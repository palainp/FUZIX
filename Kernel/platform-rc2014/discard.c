#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <ds12885.h>
#include <devide.h>
#include <devsd.h>
#include <blkdev.h>
#include <ppide.h>
#include <rc2014.h>
#include <vt.h>
#include <netdev.h>
#include <zxkey.h>
#include <ps2kbd.h>
#include <ps2mouse.h>
#include <graphics.h>
#include "z180_uart.h"

/* Everything in here is discarded after init starts */

static void nap(void)
{
}

extern uint8_t fontdata_6x8[];

static const char *vdpname = "TMS9918A";	/* Could be 28 or 29 */

static uint8_t probe_tms9918a(void)
{
	uint16_t ct = 0;
	uint8_t v;
	uint8_t *fp;

	/* Try turning it on and looking for a vblank */
	tms9918a_reset();

	/* Should see the top bit go high */
	do {
		v = tms9918a_ctrl & 0x80;
	} while(--ct && !(v & 0x80));

	if (ct == 0)
		return 0;

	nap();

	/* Reading the F bit should have cleared it */
	v = tms9918a_ctrl;
	if (v & 0x80)
		return 0;

	ct = 0;
	/* Now try and version detect : the TMS9918A IRQ must be off here */
	while(--ct && (tms9918a_ctrl & 0x80) == 0);
	if (ct == 0)
		return 0;

	/* On a VDP this selects register 2 for status reads, on a TMS9918A
	  we just wrote all over register 7 */
	tms9918a_ctrl = 0x02;
	tms9918a_ctrl = 0x8F;
	/* Read either status or S#2 */
	if (tms9918a_data & 0x40) {
		/* We have a VDP9938/9958 */
		tms9918a_ctrl = 1;
		tms9918a_ctrl = 0x8F;	/* Status register 1 please */
		v = tms9918a_data;	/* Version bits for the 9958 */
		tms9918a_ctrl = 0;
		tms9918a_ctrl = 0x8F;	/* Put the normal status register back */
		v >>= 1;		/* VDP id bits */
		/* Strictly speaking 9958 could be something higher.. */
		if (v & 0x1F)  {
			vdpname = "VDP9958";
			v = HW_VDP_9958;
		} else {
			vdpname = "VDP9938";
			v = HW_VDP_9938;
		}
	} else
		v = HW_VDP_9918A;

	/* We have a TMS9918A, load up the fonts */
	ct = 0;

	tms9918a_ctrl = 0x00;
	tms9918a_ctrl = 0x40 | 0x00;	/* Console 0 */
	while(ct++ < 4096) {
		tms9918a_data = ' ';
		nap();
	}

	/* Load the font into 3C00-3FFF */
	fp = fontdata_6x8;
	tms9918a_ctrl = 0x00;
	tms9918a_ctrl = 0x40 | 0x3C;	/* Base of font stash */
	for (ct = 0; ct < 256; ct++) {
		tms9918a_data = 0;
		nap();
	}
	while(ct++ < 1024) {
		tms9918a_data = *fp++ << 2;
		nap();
	}
	return v;
}

static uint8_t probe_16x50(uint8_t p)
{
	uint8_t r;
	uint8_t lcr = in(p + 3);
	out(p + 3, lcr | 0x80);
	out(p + 1, 0xAA);
	if (in(p + 1) != 0xAA) {
		out(p + 3, lcr);
		return 0;
	}
	out (p + 3, lcr);
	if (in(p + 1) == 0xAA)
		return 0;

	out (p + 2, 0xE7);
	r = in(p + 2);
	if (r & 0x40) {
		/* Decode types with FIFO */
		if (r & 0x80) {
			if (r & 0x20)
				return 7;
			return 5;	/* 16550A */
		}
		/* Should never find this real ones were discontinued
		   very early due to a hardware bug */
		return 0;
	} else {
		/* Decode types without FIFO */
		out(p + 7, 0x2A);
		if (in (p + 7) == 0x2A)
			return 4;
		return 8;
	}
}

/* Look for a QUART at 0xBA */

#define QUARTREG(x)	((uint16_t)(((x) << 11) | 0xBA))

#define MRA	0x00
#define CRA	0x02
#define IMR1	0x05
#define CRB	0x0A
#define IVR1	0x0C
#define CRC	0x12
#define ACR2	0x14
#define IMR2	0x15
#define CTU2	0x16
#define CTL2	0x17
#define CRD	0x1A
#define IVR2	0x1C
#define STCT2	0x1E	/* Read... */

static uint8_t probe_quart(void)
{
	uint8_t c = in16(QUARTREG(IVR1));

	c++;
	/* Make sure we they don't appear to affect one another */
	out16(QUARTREG(IVR1), c);

	if(in16(QUARTREG(IVR1)) != c)
		return 0;

	if(in16(QUARTREG(MRA)) == c)
		return 0;

	/* Ok now check IVR2 also works */
	out16(QUARTREG(IVR2), c + 1);
	if(in16(QUARTREG(IVR1)) != c)
		return 0;
	if(in16(QUARTREG(IVR2)) != c + 1)
		return 0;
	/* OK initialize things so we don't make a nasty mess when we
	   get going. We don't want interrupts for anything but receive
	   at this point. We can add line change later etc */
	out16(QUARTREG(IMR1), 0x22);
	out16(QUARTREG(IMR2), 0x22);
	/* Ensure active mode */
	out16(QUARTREG(CRA), 0xD0);
	/* Clocking */
	out16(QUARTREG(CRC), 0xD0);
	/* Reset the channels */
	out16(QUARTREG(CRA), 0x20);
	out16(QUARTREG(CRA), 0x30);
	out16(QUARTREG(CRA), 0x40);
	out16(QUARTREG(CRB), 0x20);
	out16(QUARTREG(CRB), 0x30);
	out16(QUARTREG(CRB), 0x40);
	out16(QUARTREG(CRC), 0x20);
	out16(QUARTREG(CRC), 0x30);
	out16(QUARTREG(CRC), 0x40);
	out16(QUARTREG(CRD), 0x20);
	out16(QUARTREG(CRD), 0x30);
	out16(QUARTREG(CRD), 0x40);
	/* We need to set ACR1/ACR2 once we do rts/cts and modem lines right */
	return 1;
}

/* Our counter counts clock/16 so 460800 clocks/second */

static void quart_clock(void)
{
	/* Timer, clock / 16 */
	out16(QUARTREG(ACR2), 0x70);	/* Adjust for RTS/CTS too */
	/* 10 ticks per second */
	out16(QUARTREG(CTL2), 11520 & 0xFF);
	out16(QUARTREG(CTU2), 11520 >> 8);
	/* Timer interrupt also wanted */
	out16(QUARTREG(IMR2), 0x22 | 0x08);
	/* Timer on */
	in16(QUARTREG(STCT2));
	/* Tell the quart driver to do do timer ticks */
	timer_source = TIMER_QUART;
	kputs("quart clock enabled\n");
}

__sfr __at 0xA0 sc26c92_mra;
__sfr __at 0xA2 sc26c92_cra;
__sfr __at 0xA4 sc26c92_acr;
__sfr __at 0xA6 sc26c92_ctu;
__sfr __at 0xA7 sc26c92_ctl;
__sfr __at 0xA5 sc26c92_imr;
__sfr __at 0xA8 sc26c92_mrb;
__sfr __at 0xAA sc26c92_crb;
__sfr __at 0xAE sc26c92_start;
__sfr __at 0xAF sc26c92_stop;

static uint8_t probe_sc26c92(void)
{
	volatile uint8_t dummy;

	/* These dummy reads for timer control reply FF */
	if (sc26c92_start != 0xFF || sc26c92_stop != 0xFF)
		return 0;

	sc26c92_acr = 0x30;		/* Count using the 7.37MHz clock */
	dummy = sc26c92_start;		/* Set it running */
	if (sc26c92_ctl == sc26c92_ctl)	/* Reads should show different */
		return 0;		/* values */
	dummy = sc26c92_stop;		/* Stop the clock */
	if (sc26c92_ctl != sc26c92_ctl)	/* and now same values */
		return 0;
	/* Ok looks like an SC26C92  */
	sc26c92_cra = 0x10;		/* MR1 always */
	sc26c92_cra = 0xB0;		/* MR0 on a 26C92, X bit control on a 88C681 */
	sc26c92_mra = 0x00;		/* We write MR1/MR2 or MR0/1... */
	sc26c92_mra = 0x01;
	sc26c92_cra = 0x10;		/* MR1 */
	sc26c92_imr = 0x22;
	if (sc26c92_mra & 0x01) {
		/* SC26C92 */
		sc26c92_crb = 0xB0;	/* Fix up MR0B, MR0A was done in the probe */
		sc26c92_mrb = 0x00;
		sc26c92_acr = 0x80;	/* ACR on counter off */
		return 1;
	}
	/* 88C681 */
	sc26c92_acr = 0x00;
	return 2;
}

static void sc26c92_timer(void)
{
	volatile uint8_t dummy;
	if (sc26c92_present == 1)		/* SC26C92 */
		sc26c92_acr = 0xB0;		/* Counter | ACR = 1 */
	else					/* 88C681 */
		sc26c92_acr = 0x30;		/* Counter | ACR = 0 */
	sc26c92_ctl = 46080 & 0xFF;
	sc26c92_ctu = 46080 >> 8;
	dummy = sc26c92_start;
	sc26c92_imr = 0x32;		/* Timer and both rx/tx */
	timer_source = TIMER_SC26C92;
}

void init_hardware_c(void)
{
	extern struct termios ttydflt;

	ramsize = 512;
	procmem = 512 - 80;

	ef9345_present = ef9345_probe();
	if (ef9345_present) {
		shadowcon = 1;
		ef9345_init();
		vt_twidth = 80;
		vt_tright = 79;
		vtinit();
		/* TODO: ef9345 as vblank ?? */
	}

	/* The TMS9918A and KIO clash */
	if (!kio_present && !shadowcon) {
		tms9918a_present = probe_tms9918a();
		if (tms9918a_present) {
			shadowcon = 1;
			timer_source = TIMER_TMS9918A;
			tms9918a_reload();
			vtinit();
		}
	}

	/* Set the right console for kernel messages */
	if (z180_present) {
		z180_setup(!ctc_port);
		register_uart(Z180_IO_BASE, &z180_uart0);
		register_uart(Z180_IO_BASE + 1, &z180_uart1);
		rtc_port = 0x0C;
		rtc_shadow = 0x0C;
		timer_source = TIMER_Z180;
	}
	if (kio_present) {
		register_uart(0x88, &kio_uart);
		register_uart(0x8C, &kio_uart);
	}
	/* ROMWBW favours the Z180, 16x50 then SIO then ACIA */
	if (u16x50_present) {
		register_uart(0xA0, &ns16x50_uart);
		if (probe_16x50(0xA8))
			register_uart(0xA8, &ns16x50_uart);
	}
	if (sio_present) {
		register_uart(0x80, &sio_uart);
		register_uart(0x82, &sio_uartb);
	}
	if (acia_present)
		register_uart(0xA0, &acia_uart);
	/* TODO: sc26c92 as boot probe if added to ROMWBW */
}

__sfr __at 0xBC copro_ack;
__sfr __banked __at 0xFFBC copro_boot;	/* INT, NMI reset high */
__sfr __banked __at 0xBC copro_reset;	/* reset low */

static uint8_t probe_copro(void)
{
	uint8_t i = 0;
	uint8_t c;

	copro_reset = 0x00;		/* Force a reset */
	while(i < 255)
		i++;

	copro_boot = 0x00;

	i = 0;
	while(i < 255 && copro_ack != 0xAA) {
		i++;
	}
	if (i == 255)
		return 0;

	copro_boot = 0xFF;
	i = 0;
	while(i < 255 && copro_ack == 0xAA) {
		i++;
	}
	if (i == 255)
		return 0;
	/* Now read the banner and print it */
	while(1) {
		c = copro_ack;
		/* Ack the byte */
		copro_boot = 0x00;
		if (c == 0)
			break;
		/* While we print the char the coprocessor will get the next
		   one ready - and it will beat us to the next step */
		kputchar(c);
		c = copro_ack;
		copro_boot = 0x80;
		if (c == 0)
			break;
		kputchar(c);
	}
	kputchar('\n');
	return 1;
}

void vdu_setup(void)
{
	if (shadowcon) {
		/* Add the consoles */
		uint8_t n = 0;
		shadowcon = 0;
		do {
			if (ef9345_present)
				insert_uart(0x44, &ef_uart);
			else
				insert_uart(0x98, &tms_uart);
			n++;
		} while(n < 4 && nuart <= NUM_DEV_TTY);
	}
}

__sfr __at 0xED z512_ctrl;

/*
 *	Do the main memory bank and device set up
 */
void pagemap_init(void)
{
	uint8_t i, m;

	/* RC2014 512/512K has RAM in the top 512 KiB of physical memory
	 * corresponding pages are 32-63 (page size is 16 KiB)
	 * Pages 32-34 are used by the kernel
	 * Page 35 is the common area for init
	 * Page 36 is the disk cache
	 * Pages 37 amd 38 are the second kernel bank
	 */
	for (i = 32 + 7; i < 64; i++)
		pagemap_add(i);

	/* finally add the common area */
	pagemap_add(32 + 3);

#ifdef CONFIG_RTC_DS1302	
	/* Could be at 0xC0 or 0x0C */
	ds1302_init();
	inittod();
	if (!ds1302_present) {
		rtc_port = 0x0C;
		ds1302_init();
	}
#endif

	quart_present = probe_quart();
	/* Further ports we register at this point */
	if (quart_present) {
		register_uart(0x00BA, &quart_uart);
		register_uart(0x40BA, &quart_uart);
		register_uart(0x80BA, &quart_uart);
		register_uart(0xC0BA, &quart_uart);
		/* If we don't have a TMS9918A then the QUART is the next
		   best clock choice */
		if (timer_source == TIMER_NONE)
			quart_clock();
	}

	if (sio1_present) {
		register_uart(0x84, &sio_uart);
		register_uart(0x86, &sio_uartb);
	}


	if (ctc_port) {
		if (timer_source == TIMER_NONE)
			timer_source = TIMER_CTC;
		else {
			/* Turn off our CTC interrupts */
			out(ctc_port + 2, 0x43);
			out(ctc_port + 3, 0x43);
		}
		kprintf("Z80 CTC detected at 0x%2x.\n", ctc_port);
	}
	/* Prefer CTC to DS12885 timer */
#ifdef CONFIG_RTC_DS12885
	ds12885_init();
	inittod();
	if (ds12885_present) {
		kprintf("DS12885 detected at 0x%2x.\n", rtc_port);
		if (timer_source == TIMER_NONE) {
			ds12885_set_interval();
			timer_source = TIMER_DS12885;
			kputs("DS12885 timer enabled\n");
		} else {
			ds12885_disable_interval();
		}
	}
#endif

	if (tms9918a_present)
		kprintf("%s detected at 0x98.\n", vdpname);
	if (ef9345_present)
		kputs("EF9345 detected at 0x44.\n");

	if (!acia_present)
		sc26c92_present = probe_sc26c92();

	if (sc26c92_present == 1) {
		register_uart(0x00A0, &sc26c92_uart);
		register_uart(0x00A8, &sc26c92_uart);
		if (timer_source == TIMER_NONE)
			sc26c92_timer();
		kputs("SC26C92 detected at 0xA0.\n");
	}
	if (sc26c92_present == 2) {
		register_uart(0x00A0, &xr88c681_uart);
		register_uart(0x00A8, &xr88c681_uart);
		if (timer_source == TIMER_NONE)
			sc26c92_timer();
		kputs("XR88C681 detected at 0xA0.\n");
	}

	/* Complete the timer set up */
	if (timer_source == TIMER_NONE)
		kputs("Warning: no timer available.\n");
	else
		plt_tick_present = 1;

	dma_present = !probe_z80dma();
	if (dma_present)
		kputs("Z80DMA detected at 0x04.\n");

#ifdef CONFIG_RTC_DS1302
	if (ds1302_present)
		kprintf("DS1302 detected at 0x%2x.\n", rtc_port);
#endif

	/* Boot the coprocessor if present (just one for now) */
	copro_present = probe_copro();
	if (copro_present)
		kputs("Z80 Co-processor at 0xBC\n");

	if (ps2port_init())
		ps2_type = PS2_DIRECT;
	else
		ps2_type = PS2_BITBANG;	/* Try bitbanger */

	ps2kbd_present = ps2kbd_init();
	if (ps2kbd_present) {
		kprintf("PS/2 Keyboard at 0x%2x\n", ps2_type == PS2_DIRECT ? 0x60 : 0xBB);
		if (!zxkey_present && shadowcon) {	/* TOOD: || ef9345 - test shadowcon ? */
			/* Add the consoles */
			kputs("Switching to video output.\n");
			vdu_setup();
		}
	}
	ps2mouse_present = ps2mouse_init();
	if (ps2mouse_present) {
		kprintf("PS/2 Mouse at 0x%2x\n", ps2_type == PS2_DIRECT ? 0x60 : 0xBB);
		/* TODO: wire to input layer and interrupt */
	}
	if (fpu_detect())
		kputs("AMD9511 FPU at 0x42\n");
	/* Devices in the C0-FF range cannot be used with Z180 */
	if (!z180_present) {
		i = 0xC0;
		while(i) {
			if (
#ifdef CONFIG_RTC_DS1302
				!ds1302_present ||
#endif
#ifdef CONFIG_RTC_DS12885
				!ds12885_present ||
#endif
				rtc_port != i) {
				if (m = probe_16x50(i)) {
					register_uart(i, &ns16x50_uart);
					/* Can't be a Z80-512K if there is a
					   UART at 0xE8 */
					if (i == 0xE8)
						z512_present = 0;
				}
			}
			i += 0x08;
		}
		/* Now check for Z80-512K if still possible */
		if (z512_present) {
			z512_ctrl = 7;
			if (z512_ctrl != 7)
				z512_present = 0;
			z512_ctrl = 0;
			if (z512_ctrl != 0)
				z512_present = 0;
		}
	}
	display_uarts();
}

void map_init(void)
{
}

uint8_t plt_param(unsigned char *p)
{
	/* If we have a keyboard then the TMS9918A or EF9345 becomes a real tty
	   and we make it the primary console */
	if (strcmp(p, "zxkey") == 0 && !zxkey_present && !ps2kbd_present) {
		zxkey_present = 1;
		zxkey_init();
		vdu_setup();
		return 1;
	}
	return 0;
}


void device_init(void)
{
#ifdef CONFIG_IDE
	devide_init();
#ifdef CONFIG_PPIDE
	ppide_init();
#endif
#endif
#ifdef CONFIG_SD
	pio_setup();
	devsd_init();
#endif
#ifdef CONFIG_NET
	sock_init();
#endif
}

/* Until we tidy the conditional build of floppy up provide a dummy symbol: FIXME */

#ifndef CONFIG_FLOPPY
uint8_t devfd_dtbl;
#endif

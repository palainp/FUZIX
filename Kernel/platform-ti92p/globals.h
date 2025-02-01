#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <kernel.h>

#include "heap.h"
#include "lcd.h"
#include "tty.h"
#include "kbd.h"
#include "param.h"
#include "glyph.h"
#include "list.h"

#define FLASH_CACHE_SIZE 32
#define HEAPSIZE 512
#define HEAPBLOCKSIZE 16


struct callout {
	long c_dtime;
	void *c_arg;
	void (*c_func)(void *);
};

/* [TMR] [x> */
struct timespec {
	long  tv_sec;  /* seconds */
	long  tv_nsec; /* nanoseconds */
};

struct state {
	void (*entry)(struct tty *);
	void (*event)(int ch, struct tty *);
	void (*exit)(struct tty *);
};


struct globals {
	long seconds; /* XXX: see entry.s */
	struct timespec _realtime;

	/* all RAM below here can (should) be cleared on boot. see start.s */
	char exec_ram[60]; /* XXX: see flash.s */
	char fpram[9*16+5*4]; /* XXX: see fpuemu.s */
	int onkey; /* set to 1 when ON key is pressed. see entry.s */
	int powerstate; /* set to 1 when power is off. see entry.s */

	unsigned long loadavg[3];
	/*
	 * Note: we *could* use realtime_mono as the uptime clock and get rid
	 * of the uptime variable. Both clocks count up the same way.
	 */
	struct timespec _uptime;
	long _loadavtime;

	struct callout callout[NCALL];
	masklock calloutlock;

	struct list_head avbuf_list; /* list of available buf */
	int numbufs;

	int contrast;

	/* dev_vt static variables */
	struct {
		unsigned char xon;
		unsigned char nullop;
		int privflag;
		char intchars[2+1];
		char *intcharp;
		int params[16];
		unsigned char numparams;
		unsigned char cursorvisible;
		unsigned char tabstops[(60+7)/8];
		struct state const *vtstate;

		const struct glyph *designatedcharsets[4]; // G0..G3
		struct glyph activecharset[256];
		int activecharsets[2]; // GL, GR

		unsigned char margintop, marginbottom;
		struct pos {
			int row, column;
		} pos;
		struct tty vt[1];
		
		char key_array[KEY_NBR_ROW];
		short key_mod, key_mod_sticky;
		short on_key;
		short key_compose;
		unsigned char caps_lock;
		union {
			unsigned char alpha_lock;
			unsigned char hand_lock;
		};
		unsigned char compose;
		unsigned char key_repeat; /* repeat enabled? */
		unsigned char key_repeat_delay;
		unsigned char key_repeat_start_delay;
		unsigned char key_repeat_counter;
		short key_previous;
		char gr; /* graphics rendition */
		int lock;
		int scroll_lock;
		int bell;
	} vt;
	struct {
		int third;
		int planeindex;
		int fs;
		void *currentplane;
		void *grayplanes[2];
		void *planes[3];
		int grayinitialized;
	} lcd;
	int cpubusy;
	
	int batt_level;

	/* temp/debugging variables */
	int whereami;
	int spin;
	/* end temp/debugging variables */

	/* heap static variables (this must be last!) */
	struct {
		int heapsize;
		struct heapentry heaplist[HEAPSIZE];
		char heap[0][HEAPBLOCKSIZE];
	} heap;
};

# if 0
extern struct globals G;
extern int ioport;
extern long walltime;

extern int updlock;
# else

#define G (*(struct globals *)0x5c00)
#define realtime G._realtime
#define realtime_mono G._realtime_mono
#define timeadj  G._timeadj
#define ioport   G._ioport
#define updlock  G._updlock
#define current  G._current
#define loadavtime G._loadavtime
#define uptime   G._uptime

# endif


// interrupt mask values for cpuidle
#define INT_1 0x01
#define INT_2 0x02
#define INT_3 0x04
#define INT_4 0x08
#define INT_5 0x10
#define INT_ALL 0x1f

void cpuidle(int intmask);


void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t);
void bell(struct tty *);
void showstatus(void);
void kbinit(void);
void gsignal(int, int);
void flushtty(struct tty *);

int timeout(void (*func)(void *), void *arg, long time);
int untimeout(void (*func)(void *), void *arg);
int defer(void (*func)(void *), void *arg);
int undefer(void (*func)(void *), void *arg);

#endif

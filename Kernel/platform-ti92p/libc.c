#include "globals.h"
#include <kernel.h>

int memcmp(const void* a, const void* b, size_t n)
{
  const uint8_t *ap = a;
  const uint8_t *bp = b;
  while(n--) {
    if (*ap < *bp)
      return -1;
    if (*ap != *bp)
      return 1;
    ap++;
    bp++;
  }
  return 0;
}

int ffsl(long n)
{
	static const char table[] = {
		 0, 1, 26, 2, 23, 27, 32, 3,
		16, 24, 30, 28, 11,  0, 13, 4,
		7, 17,  0, 25, 22, 31, 15, 29,
		10, 12, 6,  0, 21, 14, 9, 5,
		20, 8, 19, 18
	};
	int m = (unsigned long)(n ^ (n - 1)) % 37;
	return table[m];
}

int ffs(int n)
{
	return ffsl(n);
}

void kputchar(uint_fast8_t ch)
{
	struct tty *tp = &G.vt.vt[0]; /* XXX */
	ch &= 0xff;
	G.vt.vtstate->event(ch, tp);
}

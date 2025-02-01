#ifndef _HELPERS_H_
#define _HELPERS_H_

/* useful macro for iterating through each process */
//#define EACHPROC(p)	((p) = G.proclist; (p); (p) = (p)->p_next)

static inline void nop() { asm volatile ("nop"); }
static inline void halt() { asm volatile ("stop #0x2700"); }
static inline void splx(int x)
{
	asm volatile ("move %0,%%d0; move %%d0,%%sr"
	              : /* no output */
	              : "gr"(x)
	              : "d0");
}
static inline int spl(int x)
{
	int ret;
	asm volatile ("move %%sr,%%d0; move %%d0,%0 ; move %1*256+0x2000,%%sr"
	              : "=r"(ret)
	              : "i"(x)
	              : "d0");
	return ret;
}
#define spl0() spl(0)
#define spl1() spl(1)
#define spl2() spl(2)
#define spl3() spl(3)
#define spl4() spl(4)
#define spl5() spl(5)
#define spl6() spl(6)
#define spl7() spl(7)
#define splclock() spl1()

#define STRING2(x) #x
#define STRING(x) STRING2(x)

#define TRACE2(f, l) do { *(long *)(0x4c00+0xf00-22) = (long)(f " (" STRING(l) ")"); } while (0)
#define TRACE() TRACE2(__FILE__, __LINE__)

#endif

#include "globals.h"
#include "heap.h"

void meminit()
{
	struct heapentry *hp = &G.heap.heaplist[0];
	/* insert two sentinal heap entries,
	 * one at the bottom and one at the top of the heap */
	hp->start = 0;
	hp->end = 0;
	hp->pid = 0;
	++hp;
	hp->start = ((void *)0x40000 - (void *)G.heap.heap[0]) / HEAPBLOCKSIZE;
	hp->end = hp->start;
	hp->pid = 0;
	G.heap.heapsize = 2;
	//printmemstats(NULL);
	
}

/* insert an entry before existing entry hp, moving hp and other entries up */
static void insertentry(struct heapentry *hp, int start, int size, pid_t pid)
{
//	assert(G.heap.heapsize < HEAPSIZE);
//	assert(hp < &hp[G.heap.heapsize]);

	memmove(hp + 1, hp,
	        (void *)&G.heap.heaplist[G.heap.heapsize] - (void *)hp);
	++G.heap.heapsize;
	
	hp->start = start;
	hp->end = start + size;
	hp->pid = pid;
}

static struct heapentry *allocentry(int size, pid_t pid)
{
	struct heapentry *hp;
	
	if (size <= 0) return NULL;
	if (G.heap.heapsize >= HEAPSIZE) return NULL;
	
#define SIZETHRESHOLD 8192L
	if (size >= SIZETHRESHOLD / HEAPBLOCKSIZE) {
		/* allocate larger requests toward the bottom of memory */
		int prevend = 0;
		for (hp = &G.heap.heaplist[0];
		     hp < &G.heap.heaplist[G.heap.heapsize];
		     ++hp) {
			if (size <= hp->start - prevend) {
				insertentry(hp, prevend, size, pid);
				return hp;
			}
			prevend = hp->end;
		}
	}

	/* allocate smaller requests toward the top of memory */
	int nextstart = G.heap.heaplist[G.heap.heapsize-1].start;
	for (hp = &G.heap.heaplist[G.heap.heapsize-2];
	     hp >= &G.heap.heaplist[0];
	     --hp) {
		if (size <= (nextstart - hp->end)) {
			++hp;
			insertentry(hp, nextstart - size, size, pid);
			return hp;
		}
		nextstart = hp->start;
	}
	/* we couldn't find a slot big enough, so let's write out the oldest
	 * buffer in the avbuflist and try again if we could free that buffer */
	/* FIXME: first check to see if freeing buffers would possibly free up
	 * enough memory for this allocation to succeed. If the largest
	 * unallocated chunk size plus the size of all the buffers in the
	 * avbuflist (leaving at least MINBUF buffers) is less than the
	 * requested chunk size, then it would not do any good to flush any of
	 * the buffers */
	/* FIXME: maybe move this into a function in bio.c */
#if 0
	if (pid != 0 && G.avbuflist.b_avnext != &G.avbuflist) {
		struct buf *bp = G.avbuflist.b_avnext;
		if (buffree(bp)) goto loop;
	}
#endif
	return NULL;
}

/*
 * sizep is a pointer to the desired size. The actual size of the allocated
 * chunk is returned in the destination of sizep.
 * 
 * pid is the process id of the process that owns this memory chunk.
 */
void *memalloc(size_t *sizep, pid_t pid)
{
	size_t size;
	struct heapentry *hp;
	
	if (!sizep) {
//		P.p_error = EINVAL;
		return NULL;
	}
	
	/*
	 * If *sizep is either 0 or is large enough that
	 * (*sizep + HEAPBLOCKSIZE - 1) wraps around, size will be 0,
	 * and the following test will catch this.
	 */
	size = (*sizep + HEAPBLOCKSIZE - 1) / HEAPBLOCKSIZE;
	if (size == 0)
		return NULL;
	//kprintf("memalloc(%5u, %d)\n", (int)size, pid);
	hp = allocentry(size, pid);
	if (!hp) {
//		P.p_error = ENOMEM;
		return NULL;
	}
	*sizep = (size_t)HEAPBLOCKSIZE * size;
	return &G.heap.heap[hp->start];
}

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

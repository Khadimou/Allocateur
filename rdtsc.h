//
static inline unsigned long long rdtsc (void)
{
  //64 bit variables
  unsigned long long a, d;

  __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
  
  return (d << 32) | a;
}

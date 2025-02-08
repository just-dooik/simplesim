/* */

#include "mshr.h"   


/* create mshr */
struct mshr_t *
mshr_create(int size)
{
  struct mshr_t *mshr;
  
  /* TODO: Check parameters (but do we actually need it?)*/ 

  mshr = (struct mshr_t *)
    calloc(1, sizeof(struct mshr_t));
  if(!mshr)
    fatal("out of virtual memory");

  


}
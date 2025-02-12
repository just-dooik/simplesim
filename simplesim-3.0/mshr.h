/*
 *  This module contains code to implement MSHR(Miss Status Holding Register). 
 */

#ifndef MSHR_H
#define MSHR_H

#include <stdio.h>
#include "machine.h"
#include "memory.h"


/* forward declaration of RUU station */
struct RUU_station;

/* MSHR Block Status */
#define MSHR_BLOCK_VALID    0x00000001  /* 사용 중 */
#define MSHR_BLOCK_READY    0x00000002  /* 데이터 도착, 처리 대기 */

/* MSHR Entry Status */
#define MSHR_ENTRY_VALID    0x00000001  /* 사용 중 */
#define MSHR_ENTRY_FULL     0x00000002  /* 블록 가득 참 */
#define MSHR_ENTRY_PENDING  0x00000004  /* 메모리 접근 중 */
#define MSHR_ENTRY_PREFETCH 0x00000008  /* 프리페치 요청 */

/* MSHR block structure */
struct mshr_blk_t {
  unsigned int status; /* MSHR block status(valid, dirty) */
  md_addr_t offset; /* offset */  
  struct RUU_station *dest; /* destination, only pointer is stored so forward declaration is used */ 
  enum md_opcode op; /* opcode */ 
};

/* MSHR entry structure */
struct mshr_entry_t {
  struct mshr_blk_t *blk; /* mshr block pointer */
  unsigned int status; /* mshr entry status(valid, dirty)  */
  md_addr_t block_addr; /* block address */
  int nvalid; /* number of valid blocks */
};

/* memory access function type, like cache's blk_access_fn */
typedef unsigned int
(*mshr_mem_access_fn)(enum mem_cmd cmd,     /* Read or Write */
                      md_addr_t addr,        /* block address */
                      int bsize,             /* block size */
                      struct mshr_entry_t *entry,
                      tick_t now);           /* current time */

/* MSHR structure */
struct mshr_t {
  struct mshr_entry_t *entries; /* MSHR entries */
  int nentries; /* number of entries */ 
  int nblks; /* number of blocks for each entry */
  int nvalid; /* number of valid entries */
  int bsize; /* block size */ 
  
  /* derived data, for fast decoding */
  md_addr_t blk_mask; /* block mask */
  int blk_shift; /* block shift */

  /* memory access function */
  mshr_mem_access_fn mem_access_fn;  /* L2/memory access function */
  unsigned int mem_lat;              /* memory access latency */
};

/* create mshr*/
struct mshr_t *
mshr_create(
  int bsize, /* block size */
  int nentries, /* number of entries*/
  int nblks, /* number of blocks for each entry*/
  mshr_mem_access_fn mem_access_fn
);

/* lookup mshr */
struct mshr_entry_t *
mshr_lookup(
  struct mshr_t *mshr, 
  md_addr_t addr
);

/* mshr insert */
struct mshr_entry_t *
mshr_insert(  
  struct mshr_t *mshr,
  md_addr_t addr,
  struct RUU_station *dest,
  tick_t now
);

/* memory request send function */
unsigned int 
mshr_send_request(
  struct mshr_t *mshr, 
  struct mshr_entry_t *entry, 
  tick_t now
);

/* memory request complete function */
void
mshr_complete_request(
  struct mshr_t *mshr, 
  struct mshr_entry_t *entry, 
  byte_t *data, 
  tick_t now
);

void 
mshr_free_entry( 
  struct mshr_t *mshr, 
  struct mshr_entry_t *entry
);

#endif // !MSHR_H

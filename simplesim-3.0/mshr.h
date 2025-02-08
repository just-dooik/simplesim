/*
 *  This module contains code to implement MSHR(Miss Status Holding Register). 
 */

#ifndef MSHR_H
#define MSHR_H

#include <stdio.h>

#include "machine.h"

#define MSHR_BLOCK_VALID   0x00000001 /* MSHR block is valid, in use*/
#define MSHR_BLOCK_DIRTY   0x00000002 /* MSHR block is dirty */

#define MSHR_ENTRY_VALID   0x00000001 /* MSHR entry is valid, in use*/
#define MSHR_ENTRY_DIRTY   0x00000002 /* MSHR entry is dirty */
#define MSHR_ENTRY_TYPE_INST 0x00000004 /* MSHR entry is instruction */ 
#define MSHR_ENTRY_TYPE_DATA 0x00000008 /* MSHR entry is data */  


struct mshr_blk_t {
  struct mshr_blk_t *next; /* next MSHR block */
  struct mshr_blk_t *prev; /* previous MSHR block */
  struct RUU_STATION *dest; /* destination */

  unsigned int status; /* MSHR block status(valid, dirty) */
  md_addr_t offset; /* offset */  
};

struct mshr_entry_t {
  struct mshr_blk_t *blk; /* mshr block pointer */
  unsigned int status; /* mshr entry status(valid, dirty)  */
  md_addr_t block_addr; /* block address */
};

struct mshr_t {
  struct mshr_entry_t *entries; /* MSHR entries */

  md_addr_t blk_mask; /* block mask */
  int blk_shift; /* block shift */
};

#endif // !MSHR_H

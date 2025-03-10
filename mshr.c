/* MSHR functions implementation */

#include <math.h>  // for log2
#include "mshr.h"   
#include "memory.h" /* for enum mem_cmd */
#include "machine.h" /* for enum md_opcode */
#include "cache.h"  // for cache_access
#include "sim-outorder.h" // for RUU_station definition

extern struct cache_t *cache_dl1;  // 추가

/*
* MSHR macros
* simplescalar simulator goals to be faster, so we define some macros to speed up the code
* using bitwise operations
*/
/* get the offset of the block */
#define MSHR_BLK_OFFSET(mshr, addr) \
  ((addr) & (mshr)->blk_mask)
/* get the address of the block */
#define MSHR_BLK_ADDR(mshr, addr) \
  ((addr) & ~(mshr)->blk_mask)
/* check if the mshr is full */
#define MSHR_IS_FULL(mshr) \
  ((mshr)->nvalid == (mshr)->nentries)  
/* check if the entry is full */
#define MSHR_ENTRY_IS_FULL(mshr, entry) \
  ((mshr)->nvalid_entries == (mshr)->nentries)
/* check if the entry is valid */
#define MSHR_ENTRY_IS_VALID(mshr, entry) \
  ((entry)->status & MSHR_ENTRY_VALID)


/* create mshr */
struct mshr_t *
mshr_create(
  int bsize, /* block size */
  int nentries, /* number of entries*/
  int nblks, /* number of blocks for each entry*/
  mshr_mem_access_fn mem_access_fn
)
{
  struct mshr_t *mshr;

  mshr = (struct mshr_t *)
    calloc(1, sizeof(struct mshr_t) + (nentries-1) * sizeof(struct mshr_entry_t));
  if(!mshr)
    fatal("out of virtual memory");

  mshr->nentries = nentries;
  mshr->nblks = nblks;
  mshr->bsize = bsize;
  mshr->nvalid_entries = 0;

  /* set the block mask and shift same as cache for efficient decoding */
  mshr->blk_mask = (1 << bsize) - 1;
  mshr->blk_shift = (int)log2(mshr->blk_mask);


  /* allocate entries */
  mshr->entries = (struct mshr_entry_t *)
    calloc(nentries, sizeof(struct mshr_entry_t));
  if(!mshr->entries)
    fatal("out of virtual memory");

  /* allocate blocks */
  for(int i = 0; i < nentries; i++) {
    mshr->entries[i].blk = (struct mshr_blk_t *)
      calloc(nblks, sizeof(struct mshr_blk_t));
    if(!mshr->entries[i].blk)
      fatal("out of virtual memory");
  }

  /* 메모리 접근 함수 설정 */
  mshr->mem_access_fn = mem_access_fn;

  /* initialize entries */
  for(int i = 0; i < nentries; i++) {
    mshr->entries[i].status = 0; 
    mshr->entries[i].block_addr = 0;
    mshr->entries[i].nvalid = 0;
  } 

  /* initialize blocks */
  for(int i = 0; i < nentries; i++) {
    for(int j = 0; j < nblks; j++) {
      mshr->entries[i].blk[j].status = 0;
      mshr->entries[i].blk[j].offset = 0;
      mshr->entries[i].blk[j].dest = NULL;
    }
  } 

  return mshr;
}

/* lookup mshr
 * returns valid entry if found, otherwise returns NULL
 */
/* 시간 체크가 필요함
   now ? mshr->now + mem_latency
   여부로 pending인지 ready인지 판단 가능
*/
struct mshr_entry_t *
mshr_lookup(
  struct mshr_t *mshr, 
  md_addr_t addr /* address */
)
{ 
  /* if mshr is full, return NULL */
  if(MSHR_IS_FULL(mshr)) {
    return NULL;
  }
  
  /* get the block address */
  md_addr_t block_addr = MSHR_BLK_ADDR(mshr, addr);
  
  struct mshr_entry_t *entry; // using for loop
  struct mshr_entry_t *entry_dirty; // last dirty entry

  /* check if the entry is valid */
  for(entry = mshr->entries; entry != mshr->entries + mshr->nentries; entry++) {
    if(entry->status & MSHR_ENTRY_VALID && entry->block_addr == block_addr) 
      return entry;
    }
    if(entry->status & ~MSHR_ENTRY_VALID) {
      entry_dirty = entry;
  } 
  /* if not found, return the last dirty entry */
  if(entry_dirty) {
    entry_dirty->status |= MSHR_ENTRY_VALID;
    entry_dirty->block_addr = block_addr;
    return entry_dirty;
  }
  /* if not found, return NULL */
  return NULL;
}

/* mshr insert */
struct mshr_entry_t *
mshr_insert(
  struct mshr_t *mshr,
  md_addr_t addr,
  struct RUU_station *rs,
  tick_t now
)
{
  struct mshr_entry_t *entry;
  struct mshr_blk_t *blk;

  entry = mshr_lookup(mshr, addr);
  if (!entry) {
    /* 새 entry 할당 */
    entry = &mshr->entries[mshr->nvalid++];
    if (!entry) 
      return NULL;
    
    /* 메모리 요청 전송 */
    if (entry->status & ~MSHR_ENTRY_PENDING) {
      unsigned int lat = mshr_send_request(mshr, entry, now);
    }
  }

  /* 블록 추가 */
  if (entry && entry->nvalid < mshr->nblks) { // if entry is valid and not full
    blk = &entry->blk[entry->nvalid++];
    blk->status &= ~MSHR_BLOCK_VALID;
    blk->offset = MSHR_BLK_OFFSET(mshr, addr);
    blk->dest = rs;
    blk->request_time = now;
    blk->status |= MSHR_BLOCK_VALID;
    if (MSHR_ENTRY_IS_FULL(mshr, entry)) { // entry is full
    }
  }

  return entry; // return valid entry, if not valid, return NULL
}

/* free entry 
 * flushing entry due to misspeculation
*/
void
mshr_free_entry(
  struct mshr_t *mshr, 
  struct mshr_entry_t *entry
)
{
  for(int i = 0; i < entry->nvalid; i++) {
    struct mshr_blk_t *blk = &entry->blk[i];
    blk->status &= ~MSHR_BLOCK_VALID; // clear the valid status
  }
  entry->nvalid = 0; // clear the number of valid blocks  
  entry->status &= ~MSHR_ENTRY_VALID; // clear the valid status
}


/* memory request send function */
unsigned int 
mshr_send_request(
  struct mshr_t *mshr, 
  struct mshr_entry_t *entry, 
  tick_t now
)
{
  unsigned int lat;

  lat = mshr->mem_access_fn(Read, entry->block_addr, mshr->bsize, entry, now);

  entry->status |= MSHR_ENTRY_PENDING;
  return lat;
}

/* memory request complete function 
* complete the memory request and update the entry status 
* send the data to the destined LSQ station
*/
void 
mshr_complete_request(
  struct mshr_t *mshr, 
  struct mshr_entry_t *entry, 
  byte_t *data, 
  tick_t now
)
{
  cache_access(
    cache_dl1,  // mshr->cache 대신 cache_dl1 사용
    Write,
    entry->block_addr,
    data,
    mshr->bsize,
    now,
    NULL, NULL
  );

  /* set the completed status of the destination station */
  for(int i = 0; i < entry->nvalid; i++) {
    struct mshr_blk_t *blk = &entry->blk[i];
    if(blk->status & MSHR_BLOCK_VALID) {
      struct RUU_station *dest = blk->dest;
      dest->completed = 1; // TRUE -> 1
    }
  }

  mshr_free_entry(mshr, entry); // free the entry
}

void
mshr_dump(struct mshr_t *mshr, FILE *stream)
{
  if (!stream)
    stream = stderr;

  fprintf(stream, "\n=== MSHR State ===\n");
  fprintf(stream, "Total entries: %d, Valid entries: %d\n", 
          mshr->nentries, mshr->nvalid);

  /* dump each entry */
  for (int i = 0; i < mshr->nentries; i++) {
    struct mshr_entry_t *entry = &mshr->entries[i];
    
    fprintf(stream, "\nEntry %d:\n", i);
    fprintf(stream, "  status: 0x%x ", entry->status);
    if (entry->status & MSHR_ENTRY_VALID)   fprintf(stream, "(valid) ");
    if (entry->status & MSHR_ENTRY_FULL)    fprintf(stream, "(full) ");
    if (entry->status & MSHR_ENTRY_PENDING) fprintf(stream, "(pending) ");
    fprintf(stream, "\n");
    
    fprintf(stream, "  block_addr: 0x%08llx\n", entry->block_addr);
    fprintf(stream, "  valid blocks: %d\n", entry->nvalid);

    /* dump each block */
    for (int j = 0; j < entry->nvalid; j++) {
      struct mshr_blk_t *blk = &entry->blk[j];
      fprintf(stream, "    block %d:\n", j);
      fprintf(stream, "      status: 0x%x %s\n", 
              blk->status,
              (blk->status & MSHR_BLOCK_VALID) ? "(valid)" : "");
      fprintf(stream, "      offset: 0x%08llx\n", blk->offset);
      fprintf(stream, "      dest: %p\n", (void*)blk->dest);
    }
  }
  fprintf(stream, "\n");
}
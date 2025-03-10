#ifndef SIM_OUTORDER_H
#define SIM_OUTORDER_H

#include "cache.h"  // cache_t 타입을 위해 추가
#include "machine.h"  // md_addr_t 등을 위해 추가
#include "bpred.h"   // bpred_update_t를 위해 추가
#include "mshr.h"  // 추가

/* RUU station 구조체 선언 */
struct RUU_station {
    md_inst_t IR;                    /* instruction bits */
    enum md_opcode op;               /* decoded instruction opcode */
    md_addr_t PC, next_PC, pred_PC;  /* inst PC, next PC, predicted PC */
    int in_LSQ;                      /* non-zero if op is in LSQ */
    int LSQ_index;                   /* LSQ index if op is in LSQ */
    int ea_comp;                     /* non-zero if op has completed addr comp */
    int recover_inst;                /* start of mis-speculation? */
    int stack_recover_idx;           /* non-speculative TOS for RSB pred */
    struct bpred_update_t dir_update;/* bpred direction update info */
    int spec_mode;                   /* non-zero if issued in spec_mode */
    md_addr_t addr;                  /* effective address for ld/st's */
    int ptrace_seq;                  /* print trace sequence id */
    int queued;                      /* operands ready and queued */
    int issued;                      /* operation is/was executing */
    int completed;                   /* operation has completed execution */
    int onames[2];                   /* output logical names (NA=unused) */
    struct RS_link *odep_list[2];    /* chains of dependent operations */
    int idep_ready[3];              /* input operand ready? */
};

/* L1 데이터 캐시 extern 선언 */
extern struct cache_t *cache_dl1;

/* RS_link 구조체 선언 (RUU_station에서 사용됨) */
struct RS_link {
    struct RS_link *next;            /* next entry in list */
    struct RUU_station *rs;          /* referenced RUU resv station */
    int tag;                         /* operation tag */
};

#endif 
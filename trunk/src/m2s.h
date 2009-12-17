/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <time.h>
#include <options.h>
#include <signal.h>
#include <list.h>
#include <lnlist.h>
#include <repos.h>
#include <cachesystem.h>
#include <m2skernel.h>



/* Current simulation cycle */
extern uint64_t sim_cycle;


/* Environment variables */
extern char **environ;


/* Error debug */
#define error_debug(...) debug(error_debug_category, __VA_ARGS__)
extern int error_debug_category;



/* Processor parameters */

extern int p_stage_time_stats;
extern uint32_t p_cores;
extern uint32_t p_threads;
extern uint32_t p_quantum;
extern uint32_t p_switch_penalty;
extern int p_occupancy_stats;

/* recover_kind */
extern enum p_recover_kind_enum {
	p_recover_kind_writeback = 0,
	p_recover_kind_commit
} p_recover_kind;
extern uint32_t p_recover_penalty;

/* Fetch stage */
extern enum p_fetch_kind_enum {
	p_fetch_kind_shared = 0,
	p_fetch_kind_timeslice,
	p_fetch_kind_switchonevent
} p_fetch_kind;

/* Decode stage */
extern uint32_t p_decode_width;

/* Dispatch stage */
extern enum p_dispatch_kind_enum {
	p_dispatch_kind_shared = 0,
	p_dispatch_kind_timeslice,
} p_dispatch_kind;
extern uint32_t p_dispatch_width;

/* Issue stage */
extern enum p_issue_kind_enum {
	p_issue_kind_shared = 0,
	p_issue_kind_timeslice,
} p_issue_kind;
extern uint32_t p_issue_width;

/* Retire stage */
extern enum p_commit_kind_enum {
	p_commit_kind_shared = 0,
	p_commit_kind_timeslice
} p_commit_kind;
extern uint32_t p_commit_width;




/* Memory Management Unit */

struct mm_t;

struct mm_t *mm_create();
void mm_free(struct mm_t *mm);
void mm_translate(struct mm_t *mm, int ctx, uint32_t vtl_addr, uint32_t *phaddr);
int mm_rtranslate(struct mm_t *mm, uint32_t phaddr, int *ctx, uint32_t *vtladdr);




/* Micro Operations */

enum dep_enum {
	DNONE	= 0,
	DFIRST	= 1,

	DEAX	= 0x01,
	DECX	= 0x02,
	DEDX	= 0x03,
	DEBX	= 0x04,
	DESP	= 0x05,
	DEBP	= 0x06,
	DESI	= 0x07,
	DEDI	= 0x08,

	DES	= 0x09,
	DCS	= 0x0a,
	DSS	= 0x0b,
	DDS	= 0x0c,
	DFS	= 0x0d,
	DGS	= 0x0e,

	DZPS	= 0x0f,
	DOF	= 0x10,
	DCF	= 0x11,
	DDF	= 0x12,

	DAUX	= 0x13,  /* Intermediate results for uops */
	DAUX2	= 0x14,

	DEA	= 0x15,  /* Internal - Effective address */
	DDATA   = 0x16,  /* Internal - Data for load/store */

	DFP	= 0x17,  /* FP instructions */

	DLAST	= 0x17,
	DCOUNT	= 0x17,

	DRM8	= 0x100,
	DRM16	= 0x101,
	DRM32	= 0x102,
	DIR8	= 0x200,
	DIR16	= 0x201,
	DIR32	= 0x202,
	DR8	= 0x300,
	DR16	= 0x301,
	DR32	= 0x302,
	DSREG	= 0x400,
	DMEM	= 0x500,  /* m8, m16, m32 or m64 */
	DEASEG	= 0x501,  /* Effective address - segment */
	DEABAS	= 0x502,  /* Effective address - base */
	DEAIDX	= 0x503   /* Effective address - index */
};

#define DVALID(dep) ((dep) >= DFIRST && (dep) <= DLAST)
#define DFLAG(dep) ((dep) >= DZPS && (dep) <= DDF)

enum uop_enum {
#define UOP(_uop, _fu, _flags) uop_##_uop,
#include "uop1.dat"
#undef UOP
	uop_count
};

enum fu_class_enum {
	fu_none = 0,
	fu_intadd,
	fu_intsub,
	fu_intmult,
	fu_intdiv,
	fu_effaddr,
	fu_logical,

	fu_fpsimple,
	fu_fpadd,
	fu_fpcomp,
	fu_fpmult,
	fu_fpdiv,
	fu_fpcomplex,
	fu_fpload,
	fu_fpstore,

	fu_count
};

enum uop_flags_enum {
	FICOMP        = 0x001,  /* Integer computation */
	FLCOMP        = 0x002,  /* Logical computation */
	FFCOMP        = 0x004,  /* Floating-point computation */
	FMEM          = 0x008,  /* Memory access */
	FLOAD         = 0x010,  /* Load */
	FSTORE        = 0x020,  /* Store */
	FCTRL         = 0x040,  /* Control */
	FCALL         = 0x080,  /* Call to procedure */
	FRET          = 0x100,  /* Return from procedure */
	FCOND         = 0x200   /* Conditional branch */
};

#define IDEP_COUNT 3
#define ODEP_COUNT 5

struct uop_t {
	
	/* Main uop fields */
	enum uop_enum uop;  /* opcode */
	struct ctx_t *ctx;
	int core, thread;
	uint64_t seq;  /* sequence number - unique uop identifier */
	uint32_t eip;  /* address of macroinst */
	uint32_t neip;  /* address of next non-speculative macroinst */
	uint32_t pred_neip; /* address of next predicted macroinst (for branches) */
	uint32_t target_neip;  /* address of target macroinst assuming branch taken (for branches) */
	int specmode;
	uint64_t fetch_access;  /* Access identifier to the instruction cache */

	/* Fields associated with macroinstruction */
	int mop_index;  /* Index of uop within macroinstruction */
	int mop_count;  /* Number of uops within macroinstruction */
	int mop_size;  /* Corresponding macroinstruction size */
	uint64_t mop_seq;  /* Sequence number of macroinstruction */

	/* Logical dependencies */
	int idep[IDEP_COUNT];
	int odep[ODEP_COUNT];

	/* Physical mappings */
	int ph_idep[IDEP_COUNT];
	int ph_odep[ODEP_COUNT];
	int ph_oodep[ODEP_COUNT];

	/* Fetch */
	int fetch_tcache;  /* True if uop comes from trace cache */

	/* Execution */
	int fu_class;
	int flags;

	/* Queues where instruction is */
	int in_fetchq : 1;
	int in_uopq : 1;
	int in_iq : 1;
	int in_lq : 1;
	int in_sq : 1;
	int in_eventq : 1;
	int in_rob : 1;

	/* Instruction status */
	int ready;
	int issued;
	int completed;

	/* For memory uops */
	uint32_t mem_vtladdr;  /* virtual address */
	uint32_t mem_phaddr;  /* physical address */
	int data_witness;  /* for cache system access */

	/* Cycles */
	uint64_t when;  /* cycle when ready */
	uint64_t issue_when;  /* cycle when issued */

	/* Branch prediction */
	int pred;  /* Global prediction (0=not taken, 1=taken) */
	int bimod_index, bimod_pred;
	int twolevel_bht_index, twolevel_pht_row, twolevel_pht_col, twolevel_pred;
	int choice_index, choice_pred;
};

void uop_init(void);
void uop_done(void);

void uop_list_dump(struct list_t *uop_list, FILE *f);
void uop_lnlist_dump(struct lnlist_t *uop_list, FILE *f);
struct uop_t *uop_decode(struct list_t *list);

void uop_free_if_not_queued(struct uop_t *uop);
void uop_dump(struct uop_t *uop, FILE *f);
int uop_exists(struct uop_t *uop);




/* Functional Units */
 
struct fu_t;

void fu_reg_options(void);
void fu_init(void);
void fu_done(void);

int fu_reserve(struct fu_t *fu, int class);
void fu_release(struct fu_t *fu);




/* Fetch Queue */

extern uint32_t fetchq_size;

void fetchq_reg_options(void);
void fetchq_init(void);
void fetchq_done(void);

void fetchq_recover(int core, int thread);
struct uop_t *fetchq_remove(int core, int thread, int index);




/* Uop Queue */

extern uint32_t uopq_size;

void uopq_reg_options(void);
void uopq_init(void);
void uopq_done(void);

void uopq_recover(int core, int thread);




/* Reorder Buffer */

extern uint32_t rob_size;
extern enum rob_kind_enum {
	rob_kind_private = 0,
	rob_kind_shared
} rob_kind;

void rob_reg_options(void);
void rob_init(void);
void rob_done(void);
void rob_dump(int core, FILE *f);

int rob_can_enqueue(struct uop_t *uop);
void rob_enqueue(struct uop_t *uop);
int rob_can_dequeue(int core, int thread);
struct uop_t *rob_head(int core, int thread);
void rob_remove_head(int core, int thread);
struct uop_t *rob_tail(int core, int thread);
void rob_remove_tail(int core, int thread);
struct uop_t *rob_get(int core, int thread, int index);




/* Instruction Queue */

extern uint32_t iq_size;
extern enum iq_kind_enum {
	iq_kind_shared = 0,
	iq_kind_private
} iq_kind;


void iq_reg_options(void);
void iq_init(void);
void iq_done(void);

int iq_can_insert(struct uop_t *uop);
void iq_insert(struct uop_t *uop);
void iq_remove(int core, int thread);
void iq_recover(int core, int thread);




/* Load Queue */

extern uint32_t lq_size;

extern enum lq_kind_enum {
	lq_kind_shared = 0,
	lq_kind_private
} lq_kind;

void lq_reg_options(void);
void lq_init(void);
void lq_done(void);

int lq_can_insert(struct uop_t *uop);
void lq_insert(struct uop_t *uop);
void lq_remove(int core, int thread);
void lq_recover(int core, int thread);




/* Store Queue - Assumed private. */

extern uint32_t sq_size;

void sq_reg_options(void);
void sq_init(void);
void sq_done(void);

int sq_can_insert(struct uop_t *uop);
void sq_insert(struct uop_t *uop);
void sq_remove(int core, int thread);
void sq_recover(int core, int thread);




/* Event Queue */

void eventq_init(void);
void eventq_done(void);

int eventq_longlat(int core, int thread);
int eventq_cachemiss(int core, int thread);
void eventq_insert(struct lnlist_t *eventq, struct uop_t *uop);
struct uop_t *eventq_extract(struct lnlist_t *eventq);
void eventq_recover(int core, int thread);



/* Physical Register File */

extern uint32_t phregs_size;
extern enum phregs_kind_enum {
	phregs_kind_shared = 0,
	phregs_kind_private
} phregs_kind;

struct phreg_t {
	int pending;  /* not completed (bit) */
	int busy;  /* number of mapped logical registers */
};

struct phregs_t {
	int size;
	struct phreg_t *phreg;
	int rat[DCOUNT];

	/* List of free physical registers */
	int *free_phreg;
	int free_phreg_count;
};

void phregs_reg_options(void);
void phregs_init(void);
void phregs_done(void);

struct phregs_t *phregs_create(int size);
void phregs_free(struct phregs_t *phregs);

void phregs_dump(int core, int thread, FILE *f);
int phregs_can_rename(struct uop_t *uop);
void phregs_rename(struct uop_t *uop);
int phregs_ready(struct uop_t *uop);
void phregs_write(struct uop_t *uop);
void phregs_undo(struct uop_t *uop);
void phregs_commit(struct uop_t *uop);
void phregs_check(int core, int thread);




/* Branch Predictor */

struct bpred_t;

void bpred_reg_options(void);
void bpred_init(void);
void bpred_done(void);

struct bpred_t *bpred_create(void);
void bpred_free(struct bpred_t *bpred);
int bpred_lookup(struct bpred_t *bpred, struct uop_t *uop);
int bpred_lookup_multiple(struct bpred_t *bpred, uint32_t eip, int count);
void bpred_update(struct bpred_t *bpred, struct uop_t *uop);

uint32_t bpred_btb_lookup(struct bpred_t *bpred, struct uop_t *uop);
void bpred_btb_update(struct bpred_t *bpred, struct uop_t *uop);
uint32_t bpred_btb_next_branch(struct bpred_t *bpred, uint32_t eip, uint32_t bsize);




/* Trace cache */

#define TCACHE_ENTRY_SIZE  (sizeof(struct tcache_entry_t) + sizeof(uint32_t) * tcache_uop_max)
#define TCACHE_ENTRY(SET, WAY)  ((struct tcache_entry_t *) (((unsigned char *) tcache->entry) + TCACHE_ENTRY_SIZE * ((SET) * tcache_assoc + (WAY))))

struct tcache_entry_t {
	int counter;  /* lru counter */
	uint32_t tag;
	int uop_count, mop_count;
	int branch_mask, branch_flags, branch_count;
	uint32_t fall_through;
	uint32_t target;

	/* Last field. This is a list of 'tcache_uop_max' elements containing
	 * the addresses of the microinst located in the trace. Only in the case that
	 * all macroinst are decoded into just one uop can this array be filled up. */
	uint32_t mop_array[0];
};

struct tcache_t {
	
	/* Entries (sets * assoc) */
	struct tcache_entry_t *entry;
	struct tcache_entry_t *temp;  /* temporary trace */

	/* Stats */
	char name[20];
	uint64_t accesses;
	uint64_t hits;
	uint64_t committed;
	uint64_t squashed;
};


extern int tcache_present;
extern uint32_t tcache_uop_max;
extern uint32_t tcache_branch_max;
extern uint32_t tcache_queue_size;

struct tcache_t;

void tcache_reg_options(void);
void tcache_init(void);
void tcache_done(void);

struct tcache_t *tcache_create(void);
void tcache_free(struct tcache_t *tcache);
void tcache_new_uop(struct tcache_t *tcache, struct uop_t *uop);
int tcache_lookup(struct tcache_t *tcache, uint32_t eip, int pred,
	int *ptr_mop_count, uint32_t **ptr_mop_array, uint32_t *ptr_neip);



/* Pipeline Trace */

enum ptrace_stage_enum {
	ptrace_fetch = 0,
	ptrace_dispatch,
	ptrace_issue,
	ptrace_execution,
	ptrace_memory,
	ptrace_writeback,
	ptrace_commit
};

void ptrace_init(void);
void ptrace_done(void);
void ptrace_reg_options(void);

void ptrace_new_uop(struct uop_t *uop);
void ptrace_end_uop(struct uop_t *uop);
void ptrace_new_stage(struct uop_t *uop, enum ptrace_stage_enum stage);
void ptrace_new_cycle(void);





/* Multi-Core Multi-Thread Processor */

/* Time for stages */
extern uint64_t stage_time_fetch;
extern uint64_t stage_time_dispatch;
extern uint64_t stage_time_issue;
extern uint64_t stage_time_writeback;
extern uint64_t stage_time_commit;
extern uint64_t stage_time_rest;

/* Fast access macros */
#define CORE			(p->core[core])
#define THREAD			(p->core[core].thread[thread])
#define ICORE(I)		(p->core[(I)])
#define ITHREAD(I)		(p->core[core].thread[(I)])
#define FOREACH_CORE		for (core = 0; core < p_cores; core++)
#define FOREACH_THREAD		for (thread = 0; thread < p_threads; thread++)


/* Dispatch stall reasons */
enum di_stall_enum {
	di_stall_used = 0,  /* Dispatch slot was used with a finally committed inst. */
	di_stall_spec,  /* Used with a speculative inst. */
	di_stall_uopq,  /* No instruction in the uop queue */
	di_stall_rob,  /* No space in the rob */
	di_stall_iq,  /* No space in the iq */
	di_stall_lq,  /* No space in the lq */
	di_stall_sq,  /* No space in the sq */
	di_stall_rename,  /* No free physical register */
	di_stall_ctx,  /* No running ctx */
	di_stall_max
};


/* Thread */
struct processor_thread_t {

	struct ctx_t *ctx;  /* mapped kernel context */

	/* Reorder buffer */
	int rob_count;
	int rob_left_bound;
	int rob_right_bound;
	int rob_head;
	int rob_tail;

	/* Number of uops in private structures */
	int iq_count;
	int lq_count;
	int sq_count;

	/* Private structures */
	struct list_t *fetchq;
	struct list_t *uopq;
	struct lnlist_t *iq;
	struct lnlist_t *lq;
	struct lnlist_t *sq;
	struct bpred_t *bpred;  /* branch predictor */
	struct tcache_t *tcache;  /* trace cache */
	struct phregs_t *phregs;  /* physical register file */

	/* Fetch */ /*FIXME*/
	uint32_t fetch_eip, fetch_neip;  /* eip and next eip */
	int fetchq_occ;  /* Number of bytes occupied in the fetch queue */
	int tcacheq_occ;  /* Number of uops occupied in the trace cache queue */
	int fetch_stall;
	int fetch_bsize;  /* Block size of instruction cache for this thread */
	uint32_t fetch_block;  /* Virtual address of last fetched block */
	uint64_t fetch_access;  /* Cache access id of last instruction cache access */

	/* Stats */
	uint64_t fetched;
	uint64_t dispatched;
	uint64_t issued;
	uint64_t committed;
	uint64_t fpcommitted;
	uint64_t last_commit_cycle;
};


/* Cores */
struct processor_core_t {

	/* Array of threads */
	struct processor_thread_t *thread;

	/* Shared structures */
	struct lnlist_t *eventq;
	struct fu_t *fu;

	/* Per core counters */
	int context_map_count;
	int iq_count;
	int lq_count;
	int sq_count;

	/* Reorder Buffer */
	struct list_t *rob;
	int rob_count;
	int rob_head;
	int rob_tail;

	/* Stages */
	int fetch_current;		/* for thread switch policy */
	int64_t fetch_switch;		/* for switchonevent */
	int decode_current;
	int dispatch_current;
	int issue_current;
	int commit_current;
};


/* Processor */
struct processor_t {
	
	/* Array of cores */
	struct processor_core_t *core;

	/* Some fields */
	uint64_t seq;  /* seq num assigned to last instr (with pre-incr) */
	char *stage;  /* Name of currently simulated stage */
	int context_map_count;  /* Number of contexts mapped to threads */
	
	/* Structures */
	struct mm_t *mm;		/* memory management unit */
	
	/* Statistics */
	uint64_t fetched;
	uint64_t dispatched;
	uint64_t issued;
	uint64_t committed;
	uint64_t squashed;
	uint64_t branches;
	uint64_t mispred;
	uint64_t di_stall[di_stall_max];
	double time;

	uint64_t occupancy_count;
	uint64_t occupancy_iq_acc;
	uint64_t occupancy_lq_acc;
	uint64_t occupancy_sq_acc;
	uint64_t occupancy_rf_acc;
	uint64_t occupancy_rob_acc;

	/* For dumping */
	uint64_t last_committed;
	uint64_t last_dump;
	
};


/* Processor external variable */
extern struct processor_t *p;


/* Procedures and functions */
void p_reg_options(void);
void p_init(void);
void p_done(void);
void p_load_progs(int argc, char **argv, char *ctxfile);
void p_dump(FILE *f);
uint32_t p_tlb_address(int ctx, uint32_t vaddr);
void p_fast_forward(uint64_t cycles);

void p_update_occupancy_stats(int core);

void p_context_map(struct ctx_t *ctx, int *pcore, int *pthread);
void p_context_unmap(int core, int thread);
void p_context_map_update(void);

void p_stages(void);
void p_fetch(void);
void p_decode(void);
void p_dispatch(void);
void p_issue(void);
void p_writeback(void);
void p_commit(void);
void p_recover(int core, int thread);


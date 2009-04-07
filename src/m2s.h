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


/* processor parameters */
extern enum p_arch_enum {
	p_arch_rob = 0,
	p_arch_vb
} p_arch;

extern int p_stage_time_stats;
extern int p_effaddr;
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

/* fetch_kind */
extern enum p_fetch_kind_enum {
	p_fetch_kind_timeslice = 0,
	p_fetch_kind_switchonevent,
	p_fetch_kind_multiple
} p_fetch_kind;

/* Fetch stage */
extern enum p_fetch_policy_enum {
	p_fetch_policy_equal = 0,
	p_fetch_policy_icount,
	p_fetch_policy_pdg,
	p_fetch_policy_dcra
} p_fetch_policy;
extern uint32_t p_fetch_width;

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

	enum uop_enum uop;
	x86_inst_t *inst;
	struct ctx_t *ctx;
	int core, thread;
	int specmode;
	int mispred;  /* Mispredicted & non-speculative executed branch  */
	uint32_t eip, neip, pred_neip;
	uint64_t fetch_access;  /* Access identifier to the instruction cache */

	/* Logical dependencies */
	int idep[IDEP_COUNT];
	int odep[ODEP_COUNT];

	/* Physical mappings */
	int ph_idep[IDEP_COUNT];
	int ph_odep[ODEP_COUNT];
	int ph_oodep[ODEP_COUNT];

	int fu_class;
	int flags;

	/* PDG fetch policy */
	int lmpred_idx;
	int lmpred_miss;
	int lmpred_actual_miss;

	/* Queues where instruction is */
	int in_fetchq;
	int in_iq;
	int in_lq;
	int in_sq;
	int in_eventq;
	int in_rob;

	/* Instruction status */
	int ready;
	int issued;
	int completed;

	/* For memory uops */
	uint32_t mem_vtladdr;  /* virtual address */
	uint32_t mem_phaddr;  /* physical address */
	int data_witness;  /* for cache system access */

	/* Cycles */
	uint64_t seq;
	uint64_t when;
	uint64_t issue_when;

	/* Branch prediction */
	int bimod_taken, bimod_idx;
	int gshare_taken, gshare_idx;
	int choice_value, choice_idx;
};

void uop_init(void);
void uop_done(void);

void uop_list_dump(struct list_t *uop_list, FILE *f);
void uop_lnlist_dump(struct lnlist_t *uop_list, FILE *f);
void uop_decode(x86_inst_t *inst, struct list_t *uop_list);

void uop_free_if_not_queued(struct uop_t *uop);
void uop_dump(struct uop_t *uop, FILE *f);
int uop_exists(struct uop_t *uop);
void uop_pdg_recover(struct uop_t *uop);




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

int fetchq_can_insert(int core, int thread);
void fetchq_recover(int core, int thread);




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
	int thread;
	int pending_readers;
	int valid_remapping;
	int completed;
	int busy;
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
void phregs_read(struct uop_t *uop);
int phregs_ready(struct uop_t *uop);
void phregs_write(struct uop_t *uop);
void phregs_undo(struct uop_t *uop);
void phregs_commit(struct uop_t *uop);
void phregs_check(int core, int thread); /* FIXME */




/* Branch Predictor */

/* BTB Entry */
struct btb_entry_t {
	uint32_t source;  /* eip */
	uint32_t dest;  /* neip */
	int counter;  /* LRU counter */
};


/* Branch Predictor Structure */
struct bpred_t {
	
	/* RAS */
	uint32_t *ras;
	int ras_idx;
	
	/* BTB - bidimensional array representing btb sets
	 * Each set has 'bpred_btb_assoc' entries of type btb_entry_t. */
	struct btb_entry_t **btb;
	
	/* choice - array of bimodal counters indexed by PC
	 *   0,1 - Use bimodal predictor.
	 *   2,3 - Use gshare predictor */
	char *choice;
	
	/* bimod - array of bimodal counters indexed by PC
	 *   0,1 - Branch not taken.
	 *   2,3 - Branch taken. */
	char *bimod;
	
	/* gshare - array of bimodal counters indexed by BHR^PC
	 *   0,1 - Branch not taken.
	 *   2,3 - Branch taken. */
	char *gshare;
	uint32_t gshare_bhr;

	/* Stats */
	char name[20];
	uint64_t accesses;
	uint64_t hits;
};



void bpred_reg_options(void);
void bpred_init(void);
void bpred_done(void);
struct bpred_t *bpred_get(int core, int thread);

struct bpred_t *bpred_create(void);
void bpred_free(struct bpred_t *bpred);
uint32_t bpred_lookup(struct bpred_t *bpred, struct uop_t *uop);
void bpred_update(struct bpred_t *bpred, struct uop_t *uop);




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
	di_stall_fetchq,  /* No instruction in the fetch queue */
	di_stall_rob,  /* No space in the rob */
	di_stall_iq,  /* No space in the iq */
	di_stall_lq,  /* No space in the lq */
	di_stall_sq,  /* No space in the sq */
	di_stall_rename,  /* No free physical register */
	di_stall_ctx,  /* No running ctx */
	di_stall_max
};


/* DCRA resources */
#define DCRA_ACTIVE_MAX 256
enum dcra_resource_enum {
	dcra_resource_fetchq = 0,
	dcra_resource_iq,
	dcra_resource_lq,
	dcra_resource_sq,
	dcra_resource_phregs,
	dcra_resource_count
};


/* PDG */
#define PDG_THRESHOLD 0
#define PDG_LMPRED_SIZE 4096


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
	struct lnlist_t *iq;
	struct lnlist_t *lq;
	struct lnlist_t *sq;
	struct bpred_t *bpred;		/* branch predictor */
	struct phregs_t *phregs;	/* physical register file */

	/* Fetch */
	uint32_t fetch_eip, fetch_neip;  /* eip and next eip */
	int fetch_stall;
	uint32_t fetch_block;  /* Virtual address of last fetched block */
	uint64_t fetch_access;  /* Access ID of last instruction cache access */

	/* DCRA fetch policy */
	int dcra_fast;
  	int dcra_active[dcra_resource_count];
	int dcra_limit[dcra_resource_count];	/* space of resource limited for this thread */
	int dcra_count[dcra_resource_count];	/* number of resource entries occupied by this thread */

	/* PDG fetch policy */
	int lmpred_misses;
	char *lmpred;

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

	/* DCRA fetch policy */
	int dcra_fast_count;
	int dcra_slow_count;
	int dcra_fast_active_count[dcra_resource_count];
	int dcra_slow_active_count[dcra_resource_count];
	int dcra_active_count[dcra_resource_count];
	int dcra_inactive_count[dcra_resource_count];

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
	int dcra_resource_size[dcra_resource_count];
	
	/* Structures */
	struct mm_t *mm;		/* memory management unit */
	
	/* Statistics */
	uint64_t fetched;
	uint64_t dispatched;
	uint64_t issued;
	uint64_t committed;
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

void p_update_dcra(int core);
void p_update_occupancy_stats(int core);

void p_context_map(struct ctx_t *ctx, int *pcore, int *pthread);
void p_context_unmap(int core, int thread);
void p_context_map_update(void);

void p_stages(void);
void p_fetch();
void p_dispatch();
void p_issue();
void p_writeback();
void p_commit();
void p_recover(int core, int thread);


/* paging.h */

#ifndef _PAGING_H_
#define _PAGING_H_

typedef unsigned int	 bsd_t;

/* Structure for a page directory entry */

typedef struct {

  unsigned int pd_pres	: 1;		/* page table present?		*/
  unsigned int pd_write : 1;		/* page is writable?		*/
  unsigned int pd_user	: 1;		/* is use level protection?	*/
  unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
  unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
  unsigned int pd_acc	: 1;		/* page table was accessed?	*/
  unsigned int pd_mbz	: 1;		/* must be zero			*/
  unsigned int pd_fmb	: 1;		/* four MB pages?		*/
  unsigned int pd_global: 1;		/* global (ignored)		*/
  unsigned int pd_avail : 3;		/* for programmer's use		*/
  unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres	: 1;		/* page is present?		*/
  unsigned int pt_write : 1;		/* page is writable?		*/
  unsigned int pt_user	: 1;		/* is use level protection?	*/
  unsigned int pt_pwt	: 1;		/* write through for this page? */
  unsigned int pt_pcd	: 1;		/* cache disable for this page? */
  unsigned int pt_acc	: 1;		/* page was accessed?		*/
  unsigned int pt_dirty : 1;		/* page was written?		*/
  unsigned int pt_mbz	: 1;		/* must be zero			*/
  unsigned int pt_global: 1;		/* should be zero in 586	*/
  unsigned int pt_avail : 3;		/* for programmer's use		*/
  unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

typedef struct{
  unsigned int pg_offset : 12;		/* page offset			*/
  unsigned int pt_offset : 10;		/* page table offset		*/
  unsigned int pd_offset : 10;		/* page directory offset	*/
} virt_addr_t;

typedef struct{
  int bs_status;			/* MAPPED or UNMAPPED		*/
  int bs_pid;				/* process id using this slot   */
  int bs_vpno;				/* starting virtual page number */
  int bs_npages;			/* number of pages in the store */
  int bs_sem;				/* semaphore mechanism ?	*/
  int bs_bsid;
  unsigned int start_addr;
  unsigned int end_addr;
  int is_vheap;
} bs_map_t;

typedef struct {
  int sbs_bsid;
  int sbs_vpno;
  int sbs_npages;
  int sbs_pid;
  int sbs_status;
} sbs_map_t;


typedef struct{
  int fr_status;			/* MAPPED or UNMAPPED		*/
  int fr_pid;				/* process id using this frame  */
  int fr_vpno;				/* corresponding virtual page no*/
  int fr_refcnt;			/* reference count		*/
  int fr_type;				/* FR_DIR, FR_TBL, FR_PAGE	*/
  int fr_dirty;
  unsigned int  *cookie;		/* private data structure	*/
  unsigned long int fr_loadtime;	/* when the page is loaded 	*/
  int fr_frid;
  int fr_class;
}fr_map_t;

extern  bs_map_t bsm_tab[];
extern  fr_map_t frm_tab[];
extern  sbs_map_t sbsm_tab[];

/* calls for differet fucntions */

//bsm.c

SYSCALL init_bsm();
SYSCALL get_bsm(unsigned int **avail);
SYSCALL free_bsm(int i);
SYSCALL bsm_lookup(int pid, long vaddr, unsigned int **store, unsigned int **pageth);
SYSCALL bsm_map(int pid, int vpno, int source, int npages);
SYSCALL bsm_unmap(int pid, int vpno, int flag);

//printutils.c

extern void printbsmtab();
extern void printsecbsmtab();
extern void printfrminfo();

//secbsm.c

SYSCALL init_secbsm();
SYSCALL sbsm_map(int pid, int vpno, int source, int npages);
SYSCALL sbsm_unmap(int pid,int vpno);
SYSCALL get_sbsm_to_bsm(int i);

//frame.c

SYSCALL init_frm();
SYSCALL get_frm(unsigned int **avail);
SYSCALL free_frm(int i);

// pages.c

pd_t *pd_create(int pid);
int init_pt();
pt_t *pt_create();

// utils.c

int delete_sequence(int pid);
int get_page(int vpno,int pid);
int get_bsid(int vpno,int pid);
int table_free(pt_t *pt);
int dir_free(pd_t *pd);
int delete_mappings(int pid);
fr_map_t *find_frame(int bsid,int pageoffset);
void set_PDBR(unsigned long n);
int masks_test(unsigned long addr, int option);
int unmap_all(int pid);

//xudelseq.c

int xmunmap_del_seq(int virtpage);

//scprp.c

int init_ptr();
int increment_ptr();
int reset_ptr();
int get_flag(int bsid,int frid);
int invalidate_entries(int bsid, int frid);
int evict_frame();

//nru.c
int set_class();
int evict_frame_nru();

/* Prototypes for required API calls */
SYSCALL xmmap(int, bsd_t, int);
SYSCALL xunmap(int);
int count_sec_map;
unsigned int ptr; // for SC page replacement
extern unsigned int ctr1000;
int global_count;
int page_replace_policy;
/* given calls for dealing with backing store */

int get_bs(bsd_t, unsigned int);
SYSCALL release_bs(bsd_t);
SYSCALL read_bs(char *, bsd_t, int);
SYSCALL write_bs(char *, bsd_t, int);

#define NBPG		4096	/* number of bytes per page	*/
#define FRAME0		1024	/* zero-th frame		*/


#define NFRAMES 	1024	/* number of frames		*/

#define BSM_UNMAPPED	0
#define BSM_MAPPED	1
#define BSM_FREE        2
#define BSM_LOCKED      3

#define FRM_UNMAPPED	0
#define FRM_MAPPED	1

#define FR_PAGE		0
#define FR_TBL		1
#define FR_DIR		2

#define SC		3
#define NRU		4

#define MAX_ID          15              /* You get 15 mappings, 0 - 15 */

#define BACKING_STORE_BASE	    0x00800000
#define BACKING_STORE_UNIT_SIZE 0x00080000

#define FRAME_BASE 0x00400000
#define SIZE_OF_FRAME 0x00001000

#endif

/* pages.c -- handling page tables and directories */
#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

// have a variable fpgt[4] containing the address of the first four page tables
//only for convienience purposes.. to populate 4 entries of PD

pt_t *fpgt[4]={0,0,0,0};  // they are initialised to NULL 


/* pd_create -- to create a new directory everytime a process is created.. the function returns a pointer of type pd_t and hence we define the function to be a pointer to the type pd_t */

pd_t *pd_create(int pid) {
   //kprintf("\n pd_create: for pid %d",pid);
   pd_t *pd_loc =NULL;
   fr_map_t *frame = NULL;
   int i;
   unsigned int var;
   // first, find a free frame

   unsigned int *avail =NULL;
   int rc = get_frm(&avail);
   if (rc == SYSERR) {
      kprintf("\n could not get free frame");
      return(SYSERR);
   }
   frame = (fr_map_t *)avail;   
   
   if(frame->fr_frid > 1535) {
         kprintf("\n ERROR: TABLE/DIRECTORY LOCATION OUTSIDE DEFINED RANGE");
         kill(pid);
         return(SYSERR);
      }

   frame->fr_type = FR_DIR;
   frame->fr_pid = pid;
   frame->fr_status = FRM_MAPPED;
   frame->cookie = (unsigned int *)(FRAME_BASE + ((frame->fr_frid -1024)* SIZE_OF_FRAME));
   pd_loc = (pd_t *)(FRAME_BASE + ((frame->fr_frid -1024)* SIZE_OF_FRAME));
   for(i=0;i<1024;i++) {
      pd_loc[i].pd_pres =0;
      pd_loc[i].pd_write =0;
      pd_loc[i].pd_user =0;
      pd_loc[i].pd_pwt =0;
      pd_loc[i].pd_pcd =0;
      pd_loc[i].pd_acc =0;
      pd_loc[i].pd_mbz =0;
      pd_loc[i].pd_fmb =0;
      pd_loc[i].pd_global = 0;
      pd_loc[i].pd_avail =0;
      pd_loc[i].pd_base =0;
   }
   // now initialize the first four entries alone with the location of page tables

   for(i=0;i<4;i++) {
      pd_loc[i].pd_pres =1;
      pd_loc[i].pd_write = 1;
      var = (unsigned int) fpgt[i];
      pd_loc[i].pd_base = var/NBPG; 
   }
   //kprintf("\n pd_create:returning the base address of %08x",frame->cookie);
   //kprintf("\n pd_create: pid = %d",currpid);
   return pd_loc;
}


/* init_pt -- initialize the 1st four page tables */

int init_pt() {

   //kprintf("\n entered init_pt");
   // get a free frame
   int i,rc,j;
   unsigned int *avail =NULL;
   fr_map_t *frame =NULL;
   pt_t *pt = NULL;

   for(i=0;i<4;i++) {
      //kprintf("\n i =%d",i);
      rc = get_frm(&avail);
      if(rc == SYSERR) {
         kprintf("\nerror: could not get a free frame");
         return(SYSERR);
      }
      frame =(fr_map_t *)avail;  
      frame->fr_type = FR_TBL;
      frame->fr_status = FRM_MAPPED;
      //kprintf("\n frame got is %d",frame->fr_frid); 
      fpgt[i] = (pt_t *)(FRAME_BASE + ((frame->fr_frid -1024)* SIZE_OF_FRAME));
      pt = (pt_t *)(FRAME_BASE + ((frame->fr_frid -1024)* SIZE_OF_FRAME));
      //kprintf("\n pt is pointing to %08x",frame->cookie);
      for(j=0;j<1024;j++) {
         pt[j].pt_pres =1;
         pt[j].pt_write =1;
         pt[j].pt_user =0;
         pt[j].pt_pwt =0;
         pt[j].pt_pcd =0;
         pt[j].pt_acc =0;
         pt[j].pt_dirty =0;
         pt[j].pt_mbz =0;
         pt[j].pt_global = 0;
         pt[j].pt_avail =0;
         pt[j].pt_base = (i*1024) + j;
      }
   }
   //kprintf("\n debug: frame base address are:");
   //kprintf("\n %08x \n %08x \n %08x \n %08x",frm_tab[0].cookie,frm_tab[1].cookie,frm_tab[2].cookie,frm_tab[3].cookie);
   
   return(OK);
}


/* pg_create -- creating a page table.. this is done on demand and has a similiar procedure to that of pd_create */

pt_t *pt_create() {
   //kprintf("\n pt_create: entered page table create");
   pt_t *pt =NULL;
   fr_map_t *frame = NULL;

   // first, find a free frame
   int j;
   unsigned int *avail =NULL;
   int rc = get_frm(&avail);
   if (rc == SYSERR) {
      kprintf("\n could not get free frame");
      return(SYSERR);
   }
   frame = (fr_map_t *)avail;

   if(frame->fr_frid > 1535) {
         kprintf("\n ERROR: TABLE/DIRECTORY LOCATION OUTSIDE DEFINED RANGE");
         return(SYSERR);
      }

   //kprintf("\n pt_create: frame id is %d",frame->fr_frid);
   frame->fr_type = FR_TBL;
   frame->fr_pid = currpid;
   frame->fr_status = FRM_MAPPED;
   frame->cookie = (fr_map_t *) (FRAME_BASE + ((frame->fr_frid -1024)* SIZE_OF_FRAME));
   pt =(pt_t *)(FRAME_BASE + ((frame->fr_frid -1024)* SIZE_OF_FRAME));
   for(j=0;j<1024;j++) {
         pt[j].pt_pres =0;
         pt[j].pt_write =0;
         pt[j].pt_user =0;
         pt[j].pt_pwt =0;
         pt[j].pt_pcd =0;
         pt[j].pt_acc =0;
         pt[j].pt_dirty =0;
         pt[j].pt_mbz =0;
         pt[j].pt_global = 0;
         pt[j].pt_avail =0;
         pt[j].pt_base =0;
   }
   //kprintf("\n pt_create: returning base address of %08x",frame->cookie);
   return(pt);
}


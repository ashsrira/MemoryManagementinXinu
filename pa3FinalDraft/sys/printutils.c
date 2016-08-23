/* printutils.c -- used to printing values for debugging */
#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

void printbsmtab() {

int i;
for(i=0;i<16;i++) {

   kprintf("\n %d %d %d %d %d %d",bsm_tab[i].bs_bsid,bsm_tab[i].bs_vpno,bsm_tab[i].bs_npages,bsm_tab[i].bs_pid,bsm_tab[i].bs_status,bsm_tab[i].is_vheap);

//   kprintf("\n vheap is %d",bsm_tab[i].is_vheap);
}
}

void printsecbsmtab() {

int i;
for(i=0;i<count_sec_map;i++) {
   kprintf("\n %d %d %d %d ",sbsm_tab[i].sbs_bsid,sbsm_tab[i].sbs_vpno,sbsm_tab[i].sbs_npages,sbsm_tab[i].sbs_status);
}

}


void printfrminfo() {

// prints the status of all frames to see if any of the frames are in mapped state

int i;
for(i=0;i<1024;i++) {

   if (frm_tab[i].fr_status == FRM_MAPPED) {
      kprintf("\n %d is active",frm_tab[i].fr_frid);
   }
}
}

printfrminfo2() {

// checks the dirty and reference bits and prints the class accordingly for all frames

int i1,i,j,k,pid;
kprintf("\n DIRTY AND ACCESS BITS");

pd_t *pd =NULL;
pt_t *pt =NULL;
struct pentry *pptr = NULL;
for(i1 =0;i1 < 16;i1++) {

   if(bsm_tab[i1].bs_status == BSM_MAPPED) {
   
      pid = bsm_tab[i1].bs_pid;
      pptr = &proctab[pid];
      pd = (pd_t *)(pptr->pdbr);
      for(i=4;i<1024;i++) {
         if(pd[i].pd_pres ==1) {
            pt = (pt_t *)(pd[i].pd_base * 4096);
               for(j=0;j<1024;j++) {
                  if (pt[j].pt_pres == 1) {
                     kprintf("\n the dirty bit= %d access bit = %d, frame: %d",pt[j].pt_dirty,pt[j].pt_acc,pt[j].pt_base);
                  }
               }
          }
      }
   }
}

}


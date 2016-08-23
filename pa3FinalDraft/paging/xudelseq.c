#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>


int xmunmap_del_seq(int virtpage) {

   //kprintf("\nxudelseq:  entered xmunmap_del_seq with pid =%d",currpid);
   //first get the range of addresses to be unmapped in the variables start and end
   unsigned int start = virtpage;
   int bsid= get_bsid(virtpage,currpid);
   //kprintf("\n bsid got is %d",bsid);
   int i,j;
   unsigned int end;
   pt_t *pt = NULL;
   fr_map_t *frame = NULL;
   end =-1;
   end = bsm_tab[bsid].bs_vpno+ bsm_tab[bsid].bs_npages;
   /* for(i=0;i<16;i++) {
      if ((bsm_tab[i].bs_pid == currpid) && (bsm_tab[i].is_vheap ==0)) {
         end = bsm_tab[i].bs_vpno+ bsm_tab[i].bs_npages;
         //kprintf("\n end extreme value is %d",end);
         if(virtpage != bsm_tab[i].bs_vpno) {
            kprintf("\n incorrect virtpage");
            return(SYSERR);
         }
      }
   } */
   // if not there then search the secondary tab
/*   if (end == -1) {
      for(j=0;j<count_sec_map;j++) {
         if ((sbsm_tab[j].sbs_pid == currpid) && (sbsm_tab[j].sbs_status == BSM_MAPPED)) {
            end = sbsm_tab[j].sbs_vpno+ sbsm_tab[j].sbs_npages;
            if(virtpage != bsm_tab[j].bs_vpno) {
              kprintf("\n incorrect virtpage");
              return(SYSERR);
            }
         }
      }
   } */
// now that start and end have been got now traverse through the page directory to
// the page table and free and invalidate those entries

   struct pentry *pptr = &proctab[currpid];
   pd_t *pd = (pd_t *) (pptr->pdbr);
   //kprintf("\nxudelseq:  the page directory address is %08x",pd);

   for(i=4;i<1024;i++) {
      if (pd[i].pd_pres ==1) {
         pt = (pt_t *) ( pd[i].pd_base * 4096);
         //kprintf("\n the page table has address %08x with i = %d",pt,i);
         for(j=0;j<1024;j++) {
            if (pt[j].pt_pres ==1) {
               //kprintf("\n page entry is present for the page table with base entry %d and dirty bit is %d",pt[j].pt_base,pt[j].pt_dirty);
               frame = &frm_tab[pt[j].pt_base -1024]; 
               //kprintf("\n the frame base address is %d",frame->fr_frid);
               if ((frame->fr_vpno >= start) && (frame->fr_vpno <= end)) {
                  frame->fr_refcnt--;
                  //kprintf("\n the refcnt is %d and ref id is %d",frame->fr_refcnt,frame->fr_frid);
                  if(frame->fr_refcnt == -1) {
                     // this means that this is the last process to use the frame
                     //kprintf("\nxudelseq:  freeing frame %d",frame->fr_frid);
                     free_frm(frame->fr_frid);
                     //kprintf("\n xudelseq: status of the frame is %d",frame->fr_status);
                     pt[j].pt_pres =0;
                  }
               }
            }
         }
       pt[j].pt_pres =0; //confirm with anyone whether the whole page table must be invalidated
     }
  }
  return(OK);
}


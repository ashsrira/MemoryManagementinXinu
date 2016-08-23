#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

   /* Basic idea: when a process requests this function, if there is no mapping , it will return npages. if there is already a mapping but not vheap then the size of the request   is given. If it is vheap .. SYSERR is returned */

   int get_bs(bsd_t bs_id, unsigned int npages) {
   STATWORD ps;
   disable(ps);

   if ((bsm_tab[bs_id].bs_status == BSM_MAPPED) && (bsm_tab[bs_id].is_vheap == 1)) {
       kprintf("\n error:bs already allocated to virtual heap of another process");
       restore(ps);
       return(SYSERR);
   }
   if ((npages <=0) || (npages > 128)) {
      kprintf("\n error:invalid pages requested");
      restore(ps);
      return(SYSERR);
   }
   if (bsm_tab[bs_id].bs_status == BSM_LOCKED) {
      kprintf("\nbs is locked");
      restore(ps);
      return(bsm_tab[bs_id].bs_npages);
   }
   if ((bsm_tab[bs_id].bs_status == BSM_MAPPED) && (bsm_tab[bs_id].is_vheap ==0)) {
      restore(ps);
      return(bsm_tab[bs_id].bs_npages);
   }
   if (bsm_tab[bs_id].bs_status == BSM_FREE) {
      bsm_tab[bs_id].bs_status = BSM_LOCKED;
      bsm_tab[bs_id].bs_npages = npages;
      bsm_tab[bs_id].bs_pid = currpid;
      //kprintf("\n %d has been set to %d",bsm_tab[bs_id].bs_bsid,bsm_tab[bs_id].bs_status);
      restore(ps);
      return npages;
   }
   if (bsm_tab[bs_id].bs_status == BSM_UNMAPPED) {
      return(bsm_tab[bs_id].bs_npages);
   }
   kprintf("\n error:could not get bs");
   restore(ps);
   return(SYSERR);
  
}



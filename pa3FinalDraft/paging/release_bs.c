#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

   if ((bs_id > 16) || (bs_id < 0)) {
      kprintf("\n error: bad bsid");
      return(SYSERR);
   }
   if (bsm_tab[bs_id].bs_status != BSM_UNMAPPED) 
       return(SYSERR);

   int rc =free_bsm(bs_id);
   if (rc == SYSERR) {
      kprintf("\n error: could not free bs");
      return(SYSERR);
   } 
   return OK;
}


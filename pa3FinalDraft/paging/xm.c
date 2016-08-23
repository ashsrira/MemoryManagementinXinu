/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  /* sanity check ! */

  if ( (virtpage < 4096) || ( source < 0 ) || ( source > MAX_ID) ||(npages < 1) || ( npages >128)){
	kprintf("xmmap call error: parameter error! \n");
	return SYSERR;
  }
    STATWORD ps;
    disable(ps);
    int rc;
    //kprintf("\n entered source with %d as the source's status",bsm_tab[source].bs_status);
    if (bsm_tab[source].bs_status == BSM_FREE) {
       kprintf("\n error: cannot map a free bs");
       restore(ps);
       return(SYSERR);
    }
    if (bsm_tab[source].is_vheap ==1) {
       kprintf("\n error: cannot map to a vheap bs");
       restore(ps);
       return(SYSERR);
    }
  // process has to ask for only that amount that it asked for in get_bs.. else syserr it
    if (npages > bsm_tab[source].bs_npages) {
       kprintf("\n error: invalid npages arguement");
       restore(ps);
       return(SYSERR);
    }
  // now add the mapping
    if ((bsm_tab[source].bs_status == BSM_LOCKED) && (bsm_tab[source].bs_pid == currpid)) {
       bsm_tab[source].bs_status = BSM_MAPPED;
       bsm_tab[source].bs_pid = currpid;
       bsm_tab[source].bs_vpno = virtpage;
       bsm_tab[source].bs_npages = npages;
       return(OK);
    }
    if ((bsm_tab[source].bs_status == BSM_LOCKED) && (bsm_tab[source].bs_pid != currpid)) {
       rc = sbsm_map(currpid,virtpage,source,npages);
       if (rc == OK) {
          restore(ps);
          return(OK);
       }
    }
    // do the mapping in the secondary if the primary is already in mapped or unmapped state
    if ((bsm_tab[source].bs_status == BSM_MAPPED) || (bsm_tab[source].bs_status == BSM_UNMAPPED)) {
       rc = sbsm_map(currpid,virtpage,source,npages);
       if (rc == OK) {
          restore(ps);
          return(OK);
       }
    }
    kprintf("\n error:cannot map");
    restore(ps);
    return SYSERR;
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage )
{
  /* sanity check ! */
  if ( (virtpage < 4096) ){ 
	kprintf("xmummap call error: virtpage (%d) invalid! \n", virtpage);
	return SYSERR;
  }

   STATWORD ps;
   disable(ps);

   // go through the frame table and whichever frame matches the vpno range and pid, then release that frame -- TO BE IMPLEMENTED !!
   
   // delete sequence for xmunmap  
   xmunmap_del_seq(virtpage);

   struct pentry *pptr =&proctab[currpid];
   // use the bsm_lookup for frame clean up if needed --- MAY/MAY NOT BE IMPLEMENTED
   int rc = bsm_unmap(currpid,virtpage,0);
   
   if (rc == OK) {     
     restore(ps);
     return OK;
   }
   return SYSERR;
}


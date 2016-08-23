/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
bs_map_t bsm_tab[16];

SYSCALL init_bsm()
{
int i;
   for(i=0;i<16;i++) {
      bsm_tab[i].bs_status = BSM_FREE;
      bsm_tab[i].bs_pid = -1;
      bsm_tab[i].bs_vpno = 0;
      bsm_tab[i].bs_npages =128;
      bsm_tab[i].bs_sem =1;
      bsm_tab[i].bs_bsid = i;
      bsm_tab[i].is_vheap =0;
      if (i ==0) {
         bsm_tab[i].start_addr = 0x00800000;
         bsm_tab[i].end_addr = 0x0087ffff;
      }
      if (i ==1) {
         bsm_tab[i].start_addr = 0x00880000;
         bsm_tab[i].end_addr = 0x008fffff;
      }
      if (i ==2) {
         bsm_tab[i].start_addr = 0x00900000;
         bsm_tab[i].end_addr = 0x0097ffff;
      }
      if (i ==3) {
         bsm_tab[i].start_addr = 0x00980000;
         bsm_tab[i].end_addr = 0x009fffff;
      }
      if (i ==4) {
         bsm_tab[i].start_addr = 0x00a00000;
         bsm_tab[i].end_addr = 0x00a7ffff;
      }
      if (i ==5) {
         bsm_tab[i].start_addr = 0x00a80000;
         bsm_tab[i].end_addr = 0x00afffff;
      }
      if (i ==6) {
         bsm_tab[i].start_addr = 0x00b00000;
         bsm_tab[i].end_addr = 0x00b7ffff;
      }
      if (i ==7) {
         bsm_tab[i].start_addr = 0x00b80000;
         bsm_tab[i].end_addr = 0x00bfffff;
      }
      if (i ==8) {
         bsm_tab[i].start_addr = 0x00c00000;
         bsm_tab[i].end_addr = 0x00c7ffff;
      }
      if (i ==9) {
         bsm_tab[i].start_addr = 0x00c80000;
         bsm_tab[i].end_addr = 0x00cfffff;
      }
      if (i ==10) {
         bsm_tab[i].start_addr = 0x00d00000;
         bsm_tab[i].end_addr = 0x00d7ffff;
      }
      if (i ==11) {
         bsm_tab[i].start_addr = 0x00d80000;
         bsm_tab[i].end_addr = 0x00dfffff;
      }
      if (i ==12) {
         bsm_tab[i].start_addr = 0x00e00000;
         bsm_tab[i].end_addr = 0x00e7ffff;
      }
      if (i ==13) {
         bsm_tab[i].start_addr = 0x00e80000;
         bsm_tab[i].end_addr = 0x00efffff;
      }
      if (i ==14) {
         bsm_tab[i].start_addr = 0x00f00000;
         bsm_tab[i].end_addr = 0x00f7ffff;
      }
      if (i ==15) {
         bsm_tab[i].start_addr = 0x00f80000;
         bsm_tab[i].end_addr = 0x00ffffff;
      } 
   }
   return(OK);
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(unsigned int **avail)
{
//kprintf("\n entered get_bsm");
int i;
   for(i=0;i<16;i++) {
      if (bsm_tab[i].bs_status == BSM_FREE) {
         *avail = &(bsm_tab[i].bs_bsid);
         //kprintf("\n the bsm id is %d",**avail);
         return OK;
      }
   }
   kprintf("\n error:could not get free bsm");
   return SYSERR ;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
   if ((i < 0) ||(i > 15)) {
      kprintf("\n error:invalid i");
      return(SYSERR);
   }
   // if the free_bsm is called by the process in sbm-tab do the following

   if (bsm_tab[i].bs_pid == currpid) {
   // release_bs calls this function
   // what this does is make an entry in the bsm tab as BSM_FREE
   // so this will make have any secondary mapping that it has to the bsm_tab from the sbs_tab
      bsm_tab[i].bs_status = BSM_FREE;
      bsm_tab[i].bs_pid = -1;
      bsm_tab[i].bs_vpno =0;
      bsm_tab[i].bs_npages =128;
      bsm_tab[i].bs_sem =1;
      bsm_tab[i].is_vheap =0;

      /* look through the secondary mapping and get that into the primary bsm */
      get_sbsm_to_bsm(i);
   }
   else {
      int j;
      for(j=0;j<count_sec_map;j++) {
         if ((sbsm_tab[j].sbs_pid == currpid) &&(sbsm_tab[j].sbs_bsid == i)) {
            sbsm_tab[j].sbs_vpno =0;
            sbsm_tab[j].sbs_npages =0;
            sbsm_tab[j].sbs_pid =-1;
            sbsm_tab[j].sbs_status = BSM_FREE;
         }
      }
   }
   return(OK);
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, unsigned int ** store, unsigned int **pageth)
{
/* Idea: search through the first mapping and if not search through the second mapping */
   int i,diff;
   int vpno = vaddr >>12;
   long a = 0x00001000;
   long addr;
   int procid=-1;
   for (i=0;i<16;i++) {
      if ((bsm_tab[i].bs_pid == pid) && (bsm_tab[i].bs_status == BSM_MAPPED)) {
         if ((vpno > bsm_tab[i].bs_vpno) && (vpno < bsm_tab[i].bs_vpno + bsm_tab[i].bs_npages)) {
            diff = vpno - bsm_tab[i].bs_vpno;
            addr = bsm_tab[i].start_addr + (diff * a);
            *store = &(bsm_tab[i].bs_bsid);
            *pageth = addr;
            return(OK);
         }
      }
   }
   // this is not found in the primary bsm_tab..so we have to search the secondary tabulation
   for (i=0;i<16;i++) {
      if ((sbsm_tab[i].sbs_pid == pid) && (sbsm_tab[i].sbs_status == BSM_MAPPED)) {
         if ((vpno > sbsm_tab[i].sbs_vpno) && (vpno < sbsm_tab[i].sbs_vpno + sbsm_tab[i].sbs_npages)) {
            procid = sbsm_tab[i].sbs_bsid;
            diff = vpno - sbsm_tab[i].sbs_vpno;
            addr = bsm_tab[procid].start_addr + (diff * a);
            store = &(bsm_tab[i].bs_bsid);
            pageth = addr;
            return(OK);
         }
      }
   }

   kprintf("\nerror: search did not get result");
   return(SYSERR);
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
// xmmap calls this function..
// first check if this has already been mapped to a process
// if that process is using this as a vheap then return SYSERR
// else if the there is already a mapping then store it in the secondary mapping table.

   if ((source < 0) || (source > 15) || (npages <= 0) || (npages >128)) {
      kprintf("\n error: parameter problem");
      return(OK);
   }
   if (bsm_tab[source].is_vheap ==1) {
      kprintf("\n error:another process using this as vheap");
      return(SYSERR);
   }
   if ((bsm_tab[source].is_vheap ==0) && (bsm_tab[source].bs_status == BSM_MAPPED)) {
      if (sbsm_map(pid,vpno,source,npages) == SYSERR) {
         kprintf("\n error: could not map");
         return(SYSERR);
      }
   }
   if ((bsm_tab[source].bs_status == BSM_UNMAPPED) && (bsm_tab[source].is_vheap == 0)) {
      if (sbsm_map(pid,vpno,source,npages) == SYSERR) {
         kprintf("\n error: could not map");
         return(SYSERR);
      }
   }
   if (bsm_tab[source].bs_status == BSM_FREE) {
      bsm_tab[source].bs_status = BSM_MAPPED;
      bsm_tab[source].bs_pid = pid;
      bsm_tab[source].bs_vpno = vpno;
      bsm_tab[source].bs_npages = npages;
      return(OK);
   }
   kprintf("\n error: could not map");
   return(SYSERR);
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
//xmunmap calls this function
// this will just change the status of the bsm_tab
   int i,bs;
      for(i=0;i<16;i++) {
         if (bsm_tab[i].bs_pid == pid) {
            if ((vpno >= bsm_tab[i].bs_vpno ) && (vpno <= (bsm_tab[i].bs_vpno + bsm_tab[i].bs_npages) ) ) {
                   bs =i;
                   bsm_tab[bs].bs_status = BSM_UNMAPPED;
                   return(OK);
            }
         }
      }
   //kprintf("\n searching secondary mapping");
   int rc = sbsm_unmap(pid,vpno);
   if (rc == SYSERR) {
      kprintf("\n error: unable to unmap");
      return(SYSERR);
   }
}



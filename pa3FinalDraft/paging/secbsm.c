/* secbsm.c -- managing secondary bs mapping tabulation */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

sbs_map_t sbsm_tab[50];

SYSCALL init_secbsm() {
   int i;
   for (i=0;i<16;i++) {
      sbsm_tab[i].sbs_bsid = -1;
      sbsm_tab[i].sbs_vpno =0;
      sbsm_tab[i].sbs_npages =0;
      sbsm_tab[i].sbs_pid = -1;
      sbsm_tab[i].sbs_status = BSM_FREE;
   }
   count_sec_map=0;
   return(OK);
}

SYSCALL sbsm_map(int pid, int vpno, int source, int npages) {
/* this will add a secondary mapping in the sbsm-tab */

   sbsm_tab[count_sec_map].sbs_bsid = source;
   sbsm_tab[count_sec_map].sbs_vpno = vpno;
   sbsm_tab[count_sec_map].sbs_npages = npages;
   sbsm_tab[count_sec_map].sbs_pid = pid;
   sbsm_tab[count_sec_map].sbs_status = BSM_MAPPED;
   count_sec_map++;
   return(OK);

}

SYSCALL sbsm_unmap(int pid,int vpno) {
   int i,bs;
      for(i=0;i<count_sec_map;i++) {
         if (sbsm_tab[i].sbs_pid == pid) {
            if ((vpno >= sbsm_tab[i].sbs_vpno ) && (vpno <= (sbsm_tab[i].sbs_vpno + sbsm_tab[i].sbs_npages) ) ) {
                   bs =i;
                   sbsm_tab[bs].sbs_status = BSM_UNMAPPED;
                   return(OK);
            }
         }
      }
   //kprintf("\n error: unable to unmap");
   return(SYSERR);
}

SYSCALL get_sbsm_to_bsm(int i) {
/* search through the table and find the same bsid as i and map the first occurence to bsm_tab */

   int j;
   if (count_sec_map !=0) {
      for(j=0;j<count_sec_map;j++) {
         if ((sbsm_tab[j].sbs_bsid == i) && (sbsm_tab[j].sbs_status == BSM_MAPPED)) {
            bsm_tab[i].bs_status = BSM_MAPPED;
            bsm_tab[i].bs_pid = sbsm_tab[j].sbs_pid;
            bsm_tab[i].bs_vpno = sbsm_tab[j].sbs_vpno;
            bsm_tab[i].bs_npages = sbsm_tab[j].sbs_npages;

      /* now clear the old contents from the secondary backing store */
            sbsm_tab[j].sbs_status = BSM_FREE;
            return(OK);
         }
      }
      //kprintf("\n error: could not get from sbsm");
      
   }
   return(OK);
}


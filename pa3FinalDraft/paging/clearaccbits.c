/*clearaccbits.c -- clears pt_acc bits for all processes .. called every 2 seconds by the interrupt handler clkint.S */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

int clearaccbits() {

   global_count++;

   if (((global_count % 2) == 0) || (page_replace_policy != NRU))
      return(OK); 
   
   //kprintf("\n entered clearaccbits");
   int i1,i,j,cl;
   struct pentry *pptr =NULL;
   pd_t *pd =NULL;
   pt_t *pt =NULL;
   //first go through the bsm tab and set all the parameters
   for(i1=0;i1<16;i1++) {
      if (bsm_tab[i1].bs_status == BSM_MAPPED) {
         pptr = &proctab[bsm_tab[i1].bs_pid];
         pd =(pd_t *) (pptr->pdbr);
         //kprintf("\n pd points to %08x",pd);
         for(i=4;i<1024;i++) {
            if ( pd[i].pd_pres ==1) {
               //kprintf("\n pd_base points to %08x",pd[i].pd_base);
               pt = (pt_t *)(pd[i].pd_base * 4096);
               //kprintf("\n pt points to %08x",pt);
               for(j=0;j<1024;j++) {
                  if(pt[j].pt_pres ==1) {
                     //clear the access bits 
                     pt[j].pt_acc =0;
                  }
               }
            }
         }
      }
   }
   //kprintf("\n the same is done with the secbsmtab");
   for(i1=0;i1<count_sec_map;i1++) {
      if (sbsm_tab[i1].sbs_status == BSM_MAPPED) {
         pptr = &proctab[sbsm_tab[i1].sbs_pid];
         pd =(pd_t *) (pptr->pdbr);
         //kprintf("\n pd points to %08x",pd);
         for(i=4;i<1024;i++) {
            if(pd[i].pd_pres ==1) {
               //kprintf("\n pd_base points to %08x",pd[i].pd_base);
               pt = (pt_t *)(pd[i].pd_base *4096);
	       //kprintf("\n pt points to %08x",pt);
               for(j=0;j<1024;j++) {
                  if(pt[j].pt_pres ==1) {
                     //clear the access bits 
                     pt[j].pt_acc =0;
                  }
               }
            }
         }
      }
   }
   //kprintf("\n returning clearaccbits");
   return(OK);
}





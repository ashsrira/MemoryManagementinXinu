/*scprp.c -- second chance page replacement policy */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>


/*--- init_ptr -- initializes the the global pointer from frame 1031--- */

int init_ptr() {

    //kprintf("\n initialize pointer\n");
    ptr =5;  
    //kprintf("\n ptr is set to %d",ptr);
    return(OK);
}


/*---increment_ptr -- makes the global pointer jump to next page frame -----*/

int increment_ptr() {

   //kprintf("\n increment pointer");
   ptr = ptr+1;
   //kprintf("\n ptr now points to %d",ptr);
   return(OK);

}

/* --- reset_ptr -- resets the pointer to the frame 1031 again --------- */
int reset_ptr() {

   //kprintf("\n entered reset_ptr");
   //kprintf("\n ptr is at %d",ptr);
   if(ptr == 1024) {
      //kprintf("\n ptr has gone to the end .. resettting it");
      ptr = 5;
   }
}


/* get_flag --- gets the to know if pt_acc is set if so, it returns flag =1 and sets the
   pt_acc for all process that refer to that frame as 0 */

int get_flag(int bsid,int frid) {

/* Idea: given the bsid traverse through the bsmtab and secbsmtab and find the processes that map to the frame frid .. in that page table entry see of pt_acc is set.. if so set flag to 1 and and make those pt-acc zero.. go through all of bsmtab and secbsm tab */
   //kprintf("\n entered get_flag with bsid = %d and frid = %d",bsid,frid);
   int flag =0;
   int procid,i,j,k;
   procid =bsm_tab[bsid].bs_pid;
   struct pentry *pptr = &proctab[procid];
   pd_t *pd = (pd_t *) (pptr->pdbr);
   pt_t *pt =NULL;
  // kprintf("\n the page directory for %d is at %08x",procid,pd);
   for(i=4;i<1024;i++) {
      if (pd[i].pd_pres ==1) {
         pt = (pt_t *) ( pd[i].pd_base * 4096);
         //kprintf("\n the page table has address %08x with i = %d",pt,i);
         for(j=0;j<1024;j++) {
            if((pt[j].pt_pres == 1) && (pt[j].pt_base == frid)) {
               //kprintf("\n j =%d and access bit is %d",j,pt[j].pt_acc);
               if (pt[j].pt_acc ==1) {
                  //kprintf("\n access bit is one");
                  flag =1;
                  pt[j].pt_acc =0;
               }
            }
         }
      }
   }
   
   //now check the secbsmtab
   for(k=0;k<count_sec_map;k++) {
      
      if(sbsm_tab[k].sbs_bsid == bsid) {
          
         procid = sbsm_tab[k].sbs_pid; 
         pptr = &proctab[procid];
         pd = (pd_t *) (pptr->pdbr);
         //kprintf("\n the page directory for %d is at %08x",procid,pd);
         for(i=4;i<1024;i++) {
            if (pd[i].pd_pres ==1) {
               pt = (pt_t *) ( pd[i].pd_base * 4096);
               //kprintf("\n the page table has address %08x with i = %d",pt,i);
               for(j=0;j<1024;j++) {
                  if((pt[j].pt_pres == 1) && (pt[j].pt_base == frid)) {
                     if (pt[j].pt_acc ==1) {
                        //kprintf("\n access bit is one");
                        flag =1;
                        pt[j].pt_acc =0;
                     }
                  }
               }
            }
         }
      }
   }
   //kprintf("\n flag has the value %d",flag);
   return(flag);

}

/* invalidate_entries -- will invalidate the entries of all processes' page tables which point to the frame no to be evicted */

int invalidate_entries(int bsid, int frid) {

   //kprintf("\n entered invalidated_entries");
   int flag =0;
   int procid,i,j,k;
   procid =bsm_tab[bsid].bs_pid;
   struct pentry *pptr = &proctab[procid];
   pd_t *pd = (pd_t *) (pptr->pdbr);
   pt_t *pt =NULL;
   //kprintf("\n the page directory for %d is at %08x",procid,pd);
   for(i=4;i<1024;i++) {
      if (pd[i].pd_pres ==1) {
         pt = (pt_t *) ( pd[i].pd_base * 4096);
         //kprintf("\n the page table has address %08x with i = %d",pt,i);
         for(j=0;j<1024;j++) {
            if((pt[j].pt_pres == 1) && (pt[j].pt_base == frid)) {
               //kprintf("\n the access bit is %d",pt[j].pt_acc);
               pt[j].pt_base =0;
               pt[j].pt_pres =0;
            }
         }
      }
   }
  
   //now check the secbsmtab
   for(k=0;k<count_sec_map;k++) {

      if(sbsm_tab[k].sbs_bsid == bsid) {

         procid = sbsm_tab[k].sbs_pid;
         pptr =&proctab[procid];
         pd = (pd_t *) (pptr->pdbr);
         //kprintf("\n the page directory for %d is at %08x",procid,pd);
         for(i=4;i<1024;i++) {
            if (pd[i].pd_pres ==1) {
               pt = (pt_t *) ( pd[i].pd_base * 4096);
               //kprintf("\n the page table has address %08x with i = %d",pt,i);
               for(j=0;j<1024;j++) {
                  if((pt[j].pt_pres == 1) && (pt[j].pt_base == frid)) {
                     //kprintf("\n the access bit iss %d",pt[j].pt_acc);
                     pt[j].pt_base =0;
                     pt[j].pt_pres =0;
                  }
               }
            }
         }
      }
   }
   return(OK);
}


/* evict_frame --- Manin function which will carry out the replacement policy ---*/

int evict_frame() {
   //kprintf("\n entered evict frame algo with ptr at %d",ptr);
   int flag,bsid,pageoff,frameno; 
   int no=ptr;
   int loop =0;
   char *src =NULL;
   while (loop == 0) {
  
      if (frm_tab[no].fr_type == FR_PAGE) {

              bsid =get_bsid(frm_tab[no].fr_vpno,frm_tab[no].fr_pid);
              if (bsid == SYSERR) {
                  kprintf("\nerror: no bsid");
                  return(SYSERR);
               }
              pageoff = get_page(frm_tab[no].fr_vpno,frm_tab[no].fr_pid); // function to get the page number
               if (pageoff == SYSERR) {
                  kprintf("\n error: no page");
                  return(SYSERR);
               }
              //kprintf("\n bsid =%d pageoffset = %d",bsid,pageoff); 

              flag = get_flag(bsid,frm_tab[no].fr_frid);

              if(flag ==1) {
                 increment_ptr();
                 reset_ptr();
              }
              else {
              
                 src = (char *)(FRAME_BASE + ((frm_tab[no].fr_frid -1024) * SIZE_OF_FRAME));
                 //kprintf("\n src points to %08x with vpno %d",src,frm_tab[no].fr_pid);
                 //kprintf("\n writing to the back store src with bsid %d and page offset %d",bsid,pageoff);
                 write_bs(src,bsid,pageoff);
                 frameno=frm_tab[no].fr_frid;
                 invalidate_entries(bsid,frameno);
                 increment_ptr();
                 reset_ptr();
                 loop =1; // this will stop the loop
                 frm_tab[no].fr_status = FRM_UNMAPPED;
                 kprintf("\n SC PAGE REPLACEMENT .. %d IS REPLACED",frameno);
                 return(frameno);
              }
      }
      no++;
      if(no == 1024) 
         no=5;
   }
   //kprintf("\n could not evict frame");
   return(SYSERR);
}
                  
   

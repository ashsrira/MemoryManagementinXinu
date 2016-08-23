#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>


/* programs to see through a process termination */

int delete_sequence(int pid) {
  //kprintf("\ndelete_sequence:  entered delete sequence with pid %d",currpid);
  char *src =NULL;
  pt_t *pt =NULL;
  fr_map_t *frame =NULL;
  struct pentry *pptr =&proctab[pid];
  pd_t *pd =(pd_t *)(pptr->pdbr);
  //kprintf("\n the page directory address is %08x",pd);
  int bsid,page;
  int i,j;
  for(i=4;i<1024;i++) {
      if (pd[i].pd_pres ==1) {
         pt = (pt_t *) ( pd[i].pd_base * 4096);
         //kprintf("\n the page table has address %08x with i = %d",pt,i);
         for(j=0;j<1024;j++) {
            if (pt[j].pt_pres ==1) {
               //kprintf("\n page entry is present for the page table with base entry %d and the dirty bit is %d",pt[j].pt_base,pt[j].pt_dirty);
               frame = &frm_tab[pt[j].pt_base -1024];
               //kprintf("\n delete_sequence: the frame base address is %d",frame->fr_frid);
               src = (char *)(FRAME_BASE + ((frame->fr_frid -1024) * SIZE_OF_FRAME));
               //kprintf("\n src points to %08x with vpno %d",src,frame->fr_pid);
               bsid = get_bsid(frame->fr_vpno,frame->fr_pid); //function to get bsid
               if (bsid == SYSERR) {
                  kprintf("\nerror: no bsid");
                  return(SYSERR);
               }
               page = get_page(frame->fr_vpno,frame->fr_pid); // function to get the page number
               if (page == SYSERR) {
                  kprintf("\n error: no page");
                  return(SYSERR);
               }
               //kprintf("\n bsid = %d page offset = %d",bsid,page);
               //decrement the ref cnt
               frame->fr_refcnt--;
               if((frame->fr_refcnt ==-1) && (pt[j].pt_dirty ==1)) {
                  //kprintf("\n writing to the backing store");
                  write_bs(src,bsid,page);
                  //kprintf("\n delete_sequence: freeing frame no %d",frame->fr_frid);
                  free_frm(frame->fr_frid);
               }
               //kprintf("\n delete_sequence: status of the frame is %d",frame->fr_status);
            }
         }
         //kprintf("\n delete_sequence: freeing the page table");
         //table_free(pt);
         //kprintf("\n  delete_sequence: frame to be freed is %d",pd[i].pd_base);
         free_frm(pd[i].pd_base); 
         //kprintf("\n delete_sequence: status of frame is %d",frm_tab[pd[i].pd_base-1024].fr_status);
      }
  }
  //kprintf("\n delete_sequence: freeing the directory");
  //dir_free(pd);
  page = pptr->pdbr/4096;
  //kprintf("\n delete_sequence: direcotry frame is %d",page);
  free_frm(page);
  unmap_all(currpid);  // function to unmap all the table entries 
  //kprintf("\n delete_sequence: status of frame is %d",frm_tab[page-1024].fr_status);         
  return(OK);
}

/* get_page-- finds the bsid and the offset page and returns it to be wrtten to the backing store */

int get_page(int vpno, int pid) {

   int i,page;
   for(i=0;i<15;i++) {
      //search through the primary bsm-tab and see if we can get it
      if (bsm_tab[i].bs_pid == pid) {
         if ((vpno >= bsm_tab[i].bs_vpno) && (vpno <= (bsm_tab[i].bs_vpno + bsm_tab[i].bs_npages))) {
            page = vpno - bsm_tab[i].bs_vpno;
            return(page);
         }
      }
   }
   //kprintf("\n get_page: search did not yield results in primary tab");
   for(i=0;i<count_sec_map;i++) {
      //search through the sencondary bsm_tab and see if we can find it
      if (sbsm_tab[i].sbs_pid == currpid) {
         if ((vpno >= sbsm_tab[i].sbs_vpno) && (vpno <= (sbsm_tab[i].sbs_vpno + sbsm_tab[i].sbs_npages))) {
            page = vpno - sbsm_tab[i].sbs_vpno;
            return(page);
         }
      }
   }
   //kprintf("\nget_page: error: could not return page");
   return(SYSERR);
}



int get_bsid(int vpno,int pid) {
   int i,bsid;
   //printbsmtab();
   //kprintf("\n vpno = %d and pid = %d",vpno,pid);
   for(i=0;i<15;i++) {
      //search through the primary bsm-tab and see if we can get it
      if (bsm_tab[i].bs_pid == pid) {
         if ((vpno >= bsm_tab[i].bs_vpno) && (vpno <= (bsm_tab[i].bs_vpno + bsm_tab[i].bs_npages))) {
           bsid = i;
           return(bsid);
         }
      }
   }
   //kprintf("\n get_bsid: search did not yield results in primary tab");
   for(i=0;i<count_sec_map;i++) {
      //search through the sencondary bsm_tab and see if we can find it
      if (sbsm_tab[i].sbs_pid == currpid) {
         if ((vpno >= sbsm_tab[i].sbs_vpno) && (vpno <= (sbsm_tab[i].sbs_vpno + sbsm_tab[i].sbs_npages))) {
             bsid =sbsm_tab[i].sbs_bsid;
             return(bsid);
         }
      }
   }
   //kprintf("\nget_bsid : error: could not return bsid");
   return(SYSERR);
}

int table_free(pt_t *pt) {
   //kprintf("\n freeing table located at %08x",pt);
   int i;
   for(i=0;i<1024;i++) {
      pt[i].pt_pres =0;
      pt[i].pt_write =0;
      pt[i].pt_user =0;
      pt[i].pt_pwt =0;
      pt[i].pt_pcd =0;
      pt[i].pt_acc =0;
      pt[i].pt_dirty =0;
      pt[i].pt_mbz =0;
      pt[i].pt_global =0;
      pt[i].pt_avail =0;
      pt[i].pt_base =0;
  }
  return(OK);
}

int dir_free(pd_t *pd) {
   //kprintf("\n freeing directory located at %08x",pd);
   int i;
   for(i=0;i<1024;i++) {
      pd[i].pd_pres =0;
      pd[i].pd_write =0;
      pd[i].pd_user =0;
      pd[i].pd_pwt =0;
      pd[i].pd_pcd =0;
      pd[i].pd_acc =0;
      pd[i].pd_mbz =0;
      pd[i].pd_fmb =0;
      pd[i].pd_global =0;
      pd[i].pd_avail =0;
      pd[i].pd_base =0;
   }
   return(OK);
}

int delete_mappings(int pid) {

   // search through bsm tab and release bs those which are not vheap and then in the secondary tab
   int i;
   for(i=0;i<16;i++) {
      if ((bsm_tab[i].bs_pid == pid) && (bsm_tab[i].is_vheap ==0) && (bsm_tab[i].bs_status == BSM_MAPPED)) {
         kprintf("\n unmapping %d",bsm_tab[i].bs_bsid);
         bsm_tab[i].bs_status = BSM_UNMAPPED;
      }
   }
   // search through the secondary tab now
   for(i=0;i<count_sec_map;i++) {
      if ((sbsm_tab[i].sbs_pid == pid) && (sbsm_tab[i].sbs_status == BSM_MAPPED)) {
         kprintf("\n unmapping from sec tab %d",sbsm_tab[i].sbs_bsid);
         sbsm_tab[i].sbs_status = BSM_UNMAPPED;
      }
   }
   return(OK);
}


/* find_frame(int bsid, int pageoffset) -- will find a frame with same bs from another process and return address */


fr_map_t *find_frame(int bsid,int pageoffset) {

   //kprintf("\n entered find_frame");
   fr_map_t *frame = NULL;
   unsigned int vpno = bsm_tab[bsid].bs_vpno + pageoffset;
   //kprintf("\n vpno is %d",vpno);
   int i,j;
   for(i=4;i<1024;i++) {
      //kprintf("\n vpno of the frame is %d",frm_tab[i].fr_vpno);
      if ((frm_tab[i].fr_vpno == vpno) && (frm_tab[i].fr_status == FRM_MAPPED)) {
         frame = (fr_map_t *)((FRAME_BASE + ((frm_tab[i].fr_frid -1024)*SIZE_OF_FRAME)));
         return &(frm_tab[i]);
      }
   }
   //kprintf("\n error: could not located a page");
   //when 3 or more process share and primary is in unmapped state then search the sectab
   for(j=0;j<count_sec_map;j++) {
   
      if(sbsm_tab[j].sbs_bsid == bsid) {
  
         if(sbsm_tab[j].sbs_status == BSM_MAPPED) {
         
            vpno = sbsm_tab[j].sbs_vpno + pageoffset;
            for(i=4;i<1024;i++) {
            //kprintf("\n vpno of the frame is %d",frm_tab[i].fr_vpno);
               if ((frm_tab[i].fr_vpno == vpno) && (frm_tab[i].fr_status == FRM_MAPPED)) {
                  frame = (fr_map_t *)((FRAME_BASE + ((frm_tab[i].fr_frid -1024)*SIZE_OF_FRAME)));
                  return &(frm_tab[i]);
               }
            }
         }
      }
   }
   //kprintf("\n returning NULL");
   return NULL;
}


void set_PDBR(unsigned long n) {
  
  write_cr3(n);
}


/* function to calculatethe masks --- just for testing now */

int masks_test(unsigned long addr, int option) {
   
   //kprintf("\n address = %08x",addr); 
   int m1 = addr & 0xffc00000;
   int m2 = addr & 0x003ff000;
   int m3 = addr & 0x00000fff;
   m1 = m1 >> 22;
   m2 = m2 >> 12;
   //kprintf("\n %08x %08x %08x",m1,m2,m3);
   if (option ==1) 
      return m1;
   else if (option ==2) 
      return m2;
   else if (option ==3)
      return m3;
}


int unmap_all(int pid) {

   int i;
   for(i=0;i<16;i++) {
      if ((bsm_tab[i].bs_pid == pid) && (bsm_tab[i].is_vheap ==0)) {
         bsm_tab[i].bs_status = BSM_UNMAPPED;
      }
      if ((bsm_tab[i].bs_pid == pid) && (bsm_tab[i].is_vheap ==1)) {
         release_bs(i);
      }
   }
   for(i=0;i<count_sec_map;i++) {
      if(sbsm_tab[i].sbs_pid == pid) {
         sbsm_tab[i].sbs_status = BSM_UNMAPPED;
      }
   }
   return(OK);
}


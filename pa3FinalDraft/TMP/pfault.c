#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>


/* Page fault handler routine */

SYSCALL pfault() {

   STATWORD ps;
   
   pd_t *pd = NULL;
   pt_t *pt = NULL;
   int rc;
   int pdoff;
   int ptoff;
   int pgoff;
   fr_map_t *frame = NULL;
   unsigned long cr2;
   disable(ps);
   
   cr2 = read_cr2();
   
   
  //kprintf("\n pfault:  debug: vaddr points to %08x",cr2);

// anything to do with page replacement must be done here -- TO BE IMPLEMENTED

   pd = (pd_t *) (proctab[currpid].pdbr);  //------------- DOUBT
   //kprintf("\ndebug:the base address of page directory is %08x",pd);
   // now get the bsid and page offset
   
   int vpnocr2 = cr2/NBPG;
   int bsid = get_bsid(vpnocr2,currpid);
   if (bsid == SYSERR) {
      kprintf("\nerror: could not located bsid");
      restore(ps);
      kill(currpid);
      return(SYSERR);
   }
   int pageoff = get_page(vpnocr2,currpid);
   if (pageoff == SYSERR) {
      kprintf("\nerror: could not located bsid");
      restore(ps);
      kill(currpid);
      return(SYSERR);
   }
   //kprintf("\npfault:  debug: page offset = %d and bsid = %d",pageoff,bsid);
   //calculate all offsets in the virtual address
   
   
   //pdoff = vaddr->pd_offset;
   //ptoff = vaddr->pt_offset;
   //pgoff = vaddr->pg_offset;

   pdoff = masks_test(cr2,1);
   ptoff = masks_test(cr2,2);
   pgoff = masks_test(cr2,3);
   //kprintf("\n debug: pdoff = %d ptoff = %d pgoff = %d",pdoff,ptoff,pgoff);
   // flow: now we need to see if page table exists.. if not then create it
   // see if page exists , if not create it

      
   if (pd[pdoff].pd_pres == 0) {
      //kprintf("\n checking if the page directory entry is present");
      pt = pt_create();
      //kprintf("\n pt's address is %08x",pt);
      if (pt == NULL) {
         kprintf("\n error: page could not be created");
         restore(ps);
         kill(currpid);
         return(SYSERR);
      }
      pd[pdoff].pd_pres =1;
      pd[pdoff].pd_write =1;
      pd[pdoff].pd_user =0;
      pd[pdoff].pd_pwt =0;
      pd[pdoff].pd_pcd=0;
      pd[pdoff].pd_acc =0;
      pd[pdoff].pd_mbz =0;
      pd[pdoff].pd_fmb =0;
      pd[pdoff].pd_global =0;
      pd[pdoff].pd_avail =0;
      pd[pdoff].pd_base= ((unsigned int)(pt))/NBPG;
   }

   pt = (pt_t *) (pd[pdoff].pd_base * 4096); // -------------DOUBT
   //kprintf("\n pfault: debug: base address returned for pt is %08x",pt);
   // now to check if a bs frame has been allocated already.. if not create a new frame
   // else create an entry in the page table which points to the same frame and
   // increase the refcnt of that frame

   frame = find_frame(bsid,pageoff);
   //kprintf("\n pfault: the location frame points to is %08x",frame);
   
   if (frame == NULL) {
      //kprintf("\n frame not found");
      unsigned int *avail =NULL;
      rc = get_frm(&avail);
      if (rc == SYSERR) {
         kprintf("\n error: could not get free frame");
         restore(ps);
         kill(currpid);
         return(SYSERR);
      }
      frame = (fr_map_t *)avail;
      frame->fr_type = FR_PAGE;
      frame->fr_refcnt = 0;
      frame->fr_pid = currpid;
      frame->fr_vpno = vpnocr2;
      //kprintf("\npfault:  frame %d has vpno %d",frame->fr_frid,frame->fr_vpno);
      frame->fr_loadtime = ctr1000;
      frame->fr_status = FRM_MAPPED;

      //read from the backing store and add the value to the frame
      int physaddr = FRAME_BASE + ((frame->fr_frid - 1024) * SIZE_OF_FRAME);
      char *address = physaddr;

      //kprintf("\n read from the backing store");

      read_bs(address ,bsid, pageoff);
   }
   else {
      //kprintf("\n incrementing reference count for frame with id %d",frame->fr_frid);
      frame->fr_refcnt++;
   }

      // now we have to make the frame present in the page table
   pt[ptoff].pt_pres = 1;
   pt[ptoff].pt_write =1;
   pt[ptoff].pt_base = (FRAME_BASE + ((frame->fr_frid - 1024) * SIZE_OF_FRAME))/NBPG;
   //kprintf("\n page table entry points to %d",pt[ptoff].pt_base);
   
   //kprintf("\n ACCESS BIT IN THE TAPGE TABLE IS %d",pt[ptoff].pt_acc);
   // set the pdbr register
   unsigned long qwe = proctab[currpid].pdbr;
   
   write_cr3(qwe);
   restore(ps);
   return(OK);
}




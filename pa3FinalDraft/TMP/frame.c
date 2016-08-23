/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */

fr_map_t frm_tab[1024];

SYSCALL init_frm()
{  
  int i;
   for(i=0;i<1024;i++) {
      frm_tab[i].fr_status = FRM_UNMAPPED;
      frm_tab[i].fr_pid = -1;
      frm_tab[i].fr_vpno =0;
      frm_tab[i].fr_refcnt = 0;
      frm_tab[i].fr_type = FR_PAGE;
      frm_tab[i].fr_dirty =0;
      frm_tab[i].fr_frid = 1024+i;
      frm_tab[i].cookie = (unsigned long *)FRAME_BASE + (i*SIZE_OF_FRAME);
      frm_tab[i].fr_class =0;
   }
   return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(unsigned int **avail)
{
   int i,frameno;
   for(i=0;i < 1024 ;i++) {
      if(frm_tab[i].fr_status == FRM_UNMAPPED) {
         *avail = &(frm_tab[i]);
         //kprintf("\n get_frm: returning frame id %d",frm_tab[i].fr_frid);
         return(OK);
      }
   }
   //kprintf("\ncould not get a free frame");
   //kprintf("\n looking to evict a frame");


   if(page_replace_policy == SC)
      frameno =evict_frame();
   if(page_replace_policy == NRU)
      frameno =evict_frame_nru();
   *avail = &(frm_tab[frameno-1024]);
   //kprintf("\n returning frame id %d",frameno);

   return(OK);
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

  if ((i < 1024) || (i >2047)) {
     kprintf("\n error: invalid i");
     return(SYSERR);
  }
  frm_tab[i-1024].fr_status = FRM_UNMAPPED;

  return OK;
}




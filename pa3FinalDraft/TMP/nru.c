/*nru.c -- NRU  page replacement policy */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

//loadtime = -1; // for checking which frame to remove in case of a tie
//class =0;  // for checking purposes


/* set_class -- this function will set the highest class available for each of the frame */
int set_class () {
   //kprintf("\n setting class");
   int i1,i,j,cl,frno;
   struct pentry *pptr =NULL;
   pd_t *pd =NULL;
   pt_t *pt =NULL;
   //first go through the bsm tab and set all the parameters
   for(i1=0;i1<16;i1++) {
      if (bsm_tab[i1].bs_status == BSM_MAPPED) {
         //kprintf("\n the pid for that bsm_tab is %d",bsm_tab[i1].bs_pid);
         pptr = &proctab[bsm_tab[i1].bs_pid];
         pd =(pd_t *) (pptr->pdbr);
         for(i=4;i<1024;i++) {
            if(pd[i].pd_pres ==1) {
               pt = (pt_t *)(pd[i].pd_base *4096);
               for(j=0;j<1024;j++) {
                  if(pt[j].pt_pres ==1) {
                     //set the class comes over here
                     frno =pt[j].pt_base;
                     if ((pt[j].pt_acc ==0) && (pt[j].pt_dirty ==0)) {
                        cl =0;
                        if (frm_tab[frno-1024].fr_class <= cl) {
                           frm_tab[frno-1024].fr_class = cl;
                        }
                     }
                     if ((pt[j].pt_acc ==0) && (pt[j].pt_dirty ==1)) {
                        cl =1;
                        if (frm_tab[frno-1024].fr_class <= cl) {
                           frm_tab[frno-1024].fr_class = cl;
                        }
                     }
                     if ((pt[j].pt_acc ==1) && (pt[j].pt_dirty ==0)) {
                        cl =2;
                        if (frm_tab[frno-1024].fr_class <= cl) {
                           frm_tab[frno-1024].fr_class = cl;
                        }
                     }
                     if ((pt[j].pt_acc ==1) && (pt[j].pt_dirty ==1)) {
                        cl =3;
                        if (frm_tab[frno-1024].fr_class <= cl) {
                           frm_tab[frno-1024].fr_class = cl;
                        }
                     }
                  }
               }
            }
         }
      }
   }
   // the same is done with the secbsmtab
   for(i1=0;i1<count_sec_map;i1++) {
      if (sbsm_tab[i1].sbs_status == BSM_MAPPED) {
         pptr = &proctab[sbsm_tab[i1].sbs_pid];
         pd =(pd_t *) (pptr->pdbr);
         for(i=4;i<1024;i++) {
            if(pd[i].pd_pres ==1) {
               pt = (pt_t *)(pd[i].pd_base *4096);
               for(j=0;j<1024;j++) {
                  if(pt[j].pt_pres ==1) {
                     //set the class comes over here
                     frno =pt[j].pt_base;
                     if ((pt[j].pt_acc ==0) && (pt[j].pt_dirty ==0)) {
                        cl =0;
                        if (frm_tab[frno-1024].fr_class <= cl) {
                           frm_tab[frno-1024].fr_class = cl;
                        }
                     }
                     if ((pt[j].pt_acc ==0) && (pt[j].pt_dirty ==1)) {
                        cl =1;
                        if (frm_tab[frno-1024].fr_class <= cl) {
                           frm_tab[frno-1024].fr_class = cl;
                        }
                     }
                     if ((pt[j].pt_acc ==1) && (pt[j].pt_dirty ==0)) {
                        cl =2;
                        if (frm_tab[frno-1024].fr_class <= cl) {
                           frm_tab[frno-1024].fr_class = cl;
                        }
                     }
                     if ((pt[j].pt_acc ==1) && (pt[j].pt_dirty ==1)) {
                        cl =3;
                        if (frm_tab[frno-1024].fr_class <= cl) {
                           frm_tab[frno-1024].fr_class = cl;
                        }
                     }
                  }  
               }  
            }
         }
      }
   }
   //kprintf("\n setting class done");
   return(OK);
}   
      
/* evict_frame_nru -- overall function which will execute the nru routine */

int evict_frame_nru() {
   //kprintf("\n evict frame using nru started");
/* first set the class for all frames ... then search through the list of frames and find the lowerst class frame with the lowest time */

   int loadtime =-1;
   int class =3;
   int frameno;
   int bsid;
   int vpno;
   int pid;
   int pageoff;
   //set the class
   set_class();
   //kprintf("\n class has been set");
   // search through the frame tab to determine the lowest class that you would get

   int i;
   for(i=5;i<1024;i++) {
      if(frm_tab[i].fr_type == FR_PAGE) {
         if(frm_tab[i].fr_class < class) {
            frameno =frm_tab[i].fr_frid;
            class = frm_tab[i].fr_class;
            loadtime = frm_tab[i].fr_loadtime;
            bsid =get_bsid(frm_tab[i].fr_vpno,frm_tab[i].fr_pid);
            vpno = frm_tab[i].fr_vpno;
            pid = frm_tab[i].fr_pid;
         }
         else if (frm_tab[i].fr_class == class) {
            if ((loadtime > frm_tab[i].fr_loadtime) && (loadtime > 0)) {
               frameno =frm_tab[i].fr_frid;
               class = frm_tab[i].fr_class;
               loadtime = frm_tab[i].fr_loadtime;
               bsid =get_bsid(frm_tab[i].fr_vpno,frm_tab[i].fr_pid);
               vpno = frm_tab[i].fr_vpno;
               pid = frm_tab[i].fr_pid;
            }
            else if (loadtime ==-1) {
               frameno =frm_tab[i].fr_frid;
               class = frm_tab[i].fr_class;
               loadtime = frm_tab[i].fr_loadtime;
               bsid =get_bsid(frm_tab[i].fr_vpno,frm_tab[i].fr_pid);
               vpno = frm_tab[i].fr_vpno;
               pid = frm_tab[i].fr_pid;
           }
         }
      }
   }
   pageoff = get_page(vpno,pid);
   if (pageoff == SYSERR) {
                  kprintf("\n error: no page");
                  return(SYSERR);
   }
   //write to the bs
   char *src = (char *) (FRAME_BASE + ((frameno -1024) * SIZE_OF_FRAME));
   write_bs(src,bsid,pageoff);   
   // now invalidate all the entries in all the process that contain that frame no
   invalidate_entries(bsid,frameno);
   kprintf("\n NRU PAGE REPLACEMENT POLICY .. %d IS REPLACED ",frameno);
   return(frameno);
}
         

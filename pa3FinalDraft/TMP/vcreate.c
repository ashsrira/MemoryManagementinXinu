/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>
#include <memories.h>
/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
   STATWORD ps;
   disable(ps);

   int pid;
   // create is called to get the pid and setup the usual

   pid = create(procaddr,ssize,priority,name,nargs,args);

   struct pentry *pptr=&proctab[pid];
   struct mblock *mblk= NULL;
   int *avail =NULL;

   // a free bs is got
   //kprintf("\n debug: calling get_bsm");
   int rc = get_bsm(&avail);
   //kprintf("\n debug: the value of rc is %d",rc);
   if (rc == SYSERR) {
      kprintf("\n could not get bsm");
      restore(ps);
      return(SYSERR);
   }
   // PCB is added with details
   //kprintf("\n debug:vcreate: the bsm got is %d",*avail);
   int bsid =*avail;
   pptr->store = *avail;
   pptr->vhpno = 4096;
   pptr->vhpnpages = hsize;

   // Initializing virtual memory

   pptr->vstart =getmem(sizeof(struct vmemblock));
   (pptr->vstart)->mlen =hsize*4096;
   (pptr->vstart)->loc = 4096*4096;
   pptr->vend =getmem(sizeof(struct vmemblock));
   (pptr->vend)->mlen = 0;
   (pptr->vend)->loc =(hsize+4096)*4096;
   (pptr->vstart)->mnext = pptr->vend;
   (pptr->vend)->mnext=NULL;
   
   // mapping is added
   rc = bsm_map(pid,4096,pptr->store,pptr->vhpnpages);
   
   if (rc == SYSERR) {
      kprintf("\n could not map");
      restore(ps);
      return(SYSERR);
   }
   bsm_tab[bsid].is_vheap = 1; 
   //kprintf("\n debig: vheap is %d",bsm_tab[bsid].is_vheap);
   // call the function to allocate the page directory inside create !!!!!!!!!!!!!
   //printbsmtab();
   
   //for computation purposes..
   mblk = BACKING_STORE_BASE + (pptr->store *BACKING_STORE_UNIT_SIZE);
   mblk->mnext = NULL;
   mblk->mlen = hsize*NBPG;
  
   restore(ps);

   return (pid);
}

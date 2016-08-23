/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>
#include <memories.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{
   STATWORD ps;
   struct vmemblock *p =NULL;
   struct vmemblock *q = NULL;
   struct vmemblock *block =NULL;
   disable(ps);
   struct pentry *pptr =&proctab[currpid];
   q =pptr->vstart;
   p=q->mnext;
   nbytes = (unsigned int) roundmb(nbytes);
   while(p!= NULL) {
      //kprintf("\n q points to %08x",q->loc);
      //kprintf("\n q has length  %d",q->mlen);
      if(q->mlen >=nbytes) {
         block =getmem(sizeof(struct vmemblock));
         q->mnext = block;
         block->mnext =p;
         block->mlen = q->mlen- nbytes;
         block->loc = q->loc + nbytes;
         //kprintf("\n block location is %08x",block->loc);
         q->mlen =0; 
         restore(ps);
         return((WORD *)(q->loc));
      }
      q=p;
      p=p->mnext;
   }
   kprintf("\n could not allocate memory");
   restore(ps);
   return(SYSERR);
}



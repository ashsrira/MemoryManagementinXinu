/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>
#include <memories.h>
extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct vmemblock *block;
	unsigned size;
{
   STATWORD ps;
   disable(ps);
   struct vmemblock *p,*q,*b;
   //kprintf("\n entered vfreemem");
   struct pentry *pptr = &proctab[currpid];
   size = (unsigned)roundmb(size);
   //kprintf("\n block points to %08x and size = %d",block,size);
   
   q=pptr->vstart;
   
   //kprintf("\n b");
   
   p=q->mnext;
   
   //kprintf("\n c");   

   while(p->loc <= block) {
      q=p;
      p=p->mnext;
   }
  //kprintf("\n q points to the memory location %08x",q->loc);
  //kprintf("\n p points to the memory location %08x",p->loc);
   unsigned long r=p->loc;
   r = r - size; // check if that entire size that was allocated is freed
   //kprintf("\n r points to %08x and q has location %08x",r,q->loc);
   if(r == q->loc) {
      //kprintf ("\n q has length %d, p has length %d",q->mlen,p->mlen);
      q->mlen =p->mlen +size;
      q->mnext=p->mnext;
      //freemem(p,sizeof(p));
   }
   else {
      b=getmem(sizeof(struct vmemblock));
      b->loc = q->loc +size;
      q->mnext =b;
      b->mlen = 0;
      b->mnext =p;
      q->mlen =size;
   }
   restore(ps);
   return(OK);
}

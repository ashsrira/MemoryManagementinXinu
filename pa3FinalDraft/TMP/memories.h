#ifndef _MEMORIES_H_
#define MEMORIES_H_

struct vmemblock{
   unsigned int loc;
   struct memblock *mnext;
   unsigned int mlen;
};


#endif

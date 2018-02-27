#ifndef DEF_FMDMASYNC
#define DEF_FMDMASYNC

#include "stdint.h"
#include "dma.h"

class fmdmasync:public dma
{
	public:
	fmdmasync(int Channel,uint32_t FifoSize);
	~fmdmasync();
}; 

#endif

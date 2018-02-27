#include "stdio.h"
#include "fmdmasync.h"

fmdmasync::fmdmasync(int Channel,uint32_t FifoSize):dma(Channel,FifoSize*2,FifoSize)
{
	
}

fmdmasync::~fmdmasync()
{
}


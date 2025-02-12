#include "StaticBuffer.h"


unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

StaticBuffer::StaticBuffer()
{

    for (int bufferIndex=0;bufferIndex<32;bufferIndex++)
    {
        metainfo[bufferIndex].free = true;
    }
}

StaticBuffer::~StaticBuffer() {}

int StaticBuffer::getFreeBuffer(int blockNum)
{
    if (blockNum < 0 || blockNum > DISK_BLOCKS)
    {
        return E_OUTOFBOUND;
    }
    int allocatedBuffer;
    for(int bufferIndex=0;bufferIndex<32;bufferIndex++){
        if(metainfo[bufferIndex].free){
            allocatedBuffer=bufferIndex;
            break;
        }
    }
    metainfo[allocatedBuffer].free = false;
    metainfo[allocatedBuffer].blockNum = blockNum;

    return allocatedBuffer;
}

int StaticBuffer::getBufferNum(int blockNum)
{
  
    if(blockNum<0 || blockNum>=DISK_BLOCKS){
        return E_OUTOFBOUND;
    }

    for(int i=0;i<32;i++){
        if(!metainfo[i].free && metainfo[i].blockNum==blockNum){
            return i;
        }
    }
    return E_BLOCKNOTINBUFFER;
}

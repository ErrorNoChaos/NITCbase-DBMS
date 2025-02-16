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
StaticBuffer::StaticBuffer()
{

    for (int i=0;i<32;i++)
    {
        metainfo[i].blockNum=-1;
        metainfo[i].dirty=false;
        metainfo[i].free=true;
        metainfo[i].timeStamp=-1;
    }
}

StaticBuffer::~StaticBuffer()
{
    for(int i=0;i<32;i++){
        if(!metainfo[i].free && metainfo[i].dirty){
            
            Disk::writeBlock(blocks[i],metainfo[i].blockNum);
        }
    }
}
int StaticBuffer::getFreeBuffer(int blockNum)
{
   
    if(blockNum<0 ||blockNum>=BLOCK_SIZE){
        return E_OUTOFBOUND;
    }

    for(int i=0;i<32;i++){
        if(!metainfo[i].free){
            metainfo[i].timeStamp+=1;
        }
    }
    int bufferNum=-1;
    for(int i=0;i<32;i++){
        if(metainfo[i].free){
            bufferNum=i;
            break;
        }
    }
    if(bufferNum==-1){
        int largest=-2;
        int flag=-1;
        for(int i=0;i<32;i++){
            if(metainfo[i].timeStamp>largest){
                largest=metainfo[i].timeStamp;
                flag=i;
            }
        }
        bufferNum=flag;
        if(metainfo[flag].dirty){
            Disk::writeBlock(blocks[flag],metainfo[flag].blockNum);
        }
    }
    metainfo[bufferNum].free=false;
    metainfo[bufferNum].dirty=false;
    metainfo[bufferNum].blockNum=blockNum;
    metainfo[bufferNum].timeStamp=0;
    return bufferNum;

}
int StaticBuffer::setDirtyBit(int blockNum)
{
    int buffindex=getBufferNum(blockNum);
    if(buffindex==E_BLOCKNOTINBUFFER){
        return E_BLOCKNOTINBUFFER;
    }
    if(buffindex==E_OUTOFBOUND){
        return E_OUTOFBOUND;
    }
    else{
        metainfo[buffindex].dirty=true;
    }
    return SUCCESS;
}
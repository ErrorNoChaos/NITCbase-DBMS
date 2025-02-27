#include "BlockBuffer.h"
#include "StaticBuffer.h"
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <iostream>
RecBuffer::RecBuffer(int blockNum) : BlockBuffer(blockNum) {}
RecBuffer::RecBuffer() : BlockBuffer('R') {}
BlockBuffer::BlockBuffer(int blockNum)
{
    this->blockNum=blockNum;

}
int BlockBuffer::getBlockNum()
{
    return this->blockNum;
}

/////////stage-5/////////////////
// int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr)
// {
//     int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

//     if (bufferNum == E_BLOCKNOTINBUFFER)
//     {
//         bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

//         if (bufferNum == E_OUTOFBOUND)
//         {
//             return E_OUTOFBOUND;
//         }

//         Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
//     }

//     *buffPtr = StaticBuffer::blocks[bufferNum];

//     return SUCCESS;
// }

//////stage-5//////

int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr)
{

    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);
    if(bufferNum!=E_BLOCKNOTINBUFFER){
        for(int i=0;i<32;i++){
            StaticBuffer::metainfo[i].timeStamp++;
        }
        StaticBuffer::metainfo[bufferNum].timeStamp=0;

    }
    else{
        bufferNum =StaticBuffer::getFreeBuffer(this->blockNum);
        if(bufferNum==E_OUTOFBOUND){
            return E_OUTOFBOUND;
        }
        Disk::readBlock(StaticBuffer::blocks[bufferNum],this->blockNum);
       
    }
    *buffPtr = StaticBuffer::blocks[bufferNum];
    return SUCCESS;
}
int BlockBuffer::getHeader(struct HeadInfo *head)
{

    unsigned char *bufferPtr;
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS)
    {
        return ret;
    }


    memcpy(&head->numSlots, bufferPtr + 24, 4);
    memcpy(&head->numEntries, bufferPtr + 16, 4);
    memcpy(&head->numAttrs, bufferPtr + 20, 4);
    memcpy(&head->rblock,bufferPtr+12, 4);
    memcpy(&head->lblock, bufferPtr+8, 4);
    return SUCCESS;
}
int BlockBuffer::setHeader(struct HeadInfo *head)
{

    unsigned char *bufferPtr;
    int l=loadBlockAndGetBufferPtr(&bufferPtr);
    if(l!=SUCCESS){
        return l;
    }

    struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;
    bufferHeader->numSlots=head->numSlots;
    bufferHeader->blockType=head->blockType;
    bufferHeader->numAttrs=head->numAttrs;
    bufferHeader->lblock=head->lblock;
    bufferHeader->numEntries=head->numEntries;
    bufferHeader->pblock=head->pblock;
    bufferHeader->rblock=head->rblock;
    int num=StaticBuffer::setDirtyBit(this->blockNum);
    if(num!=SUCCESS){
        return num;
    }

    return SUCCESS;
}
int BlockBuffer::setBlockType(int blockType)
{

    unsigned char *bufferPtr;
    int l = loadBlockAndGetBufferPtr(&bufferPtr);
    if (l != SUCCESS)
    {
        return l;
    }

    *((int32_t *)bufferPtr) = blockType;
    StaticBuffer::blockAllocMap[this->blockNum]= blockType;

    int num = StaticBuffer::setDirtyBit(this->blockNum);
    if (num != SUCCESS)
    {
        return num;
    }

    return SUCCESS;
}
int BlockBuffer::getFreeBlock(int blockType)
{

    int flag=0,freeblock;
    for(int i=0;i<DISK_BLOCKS;i++){
        if(StaticBuffer::blockAllocMap[i]==UNUSED_BLK){
            freeblock=i;
            flag=1;
            break;
        }
    }
    if(flag==0){
        return E_DISKFULL;
    }
    this->blockNum=freeblock;

    StaticBuffer::getFreeBuffer(this->blockNum);
    HeadInfo head;
    head.pblock=-1;
    head.lblock=-1;
    head.rblock=-1;
    head.numEntries=0;
    head.numSlots=0;
    head.numAttrs=0;
    this->setHeader(&head);
    this->setBlockType(blockType);
    return freeblock;

}
BlockBuffer::BlockBuffer(char blockType)
{
     int block=getFreeBlock(blockType);
     if(block<0 || block>=DISK_BLOCKS){
        printf("Blk number is invalid\n");
        this->blockNum=blockNum;
        return;
     }
     this->blockNum=block;

}


int RecBuffer::getRecord(union Attribute *rec, int slotNum)
{
    // ...
    unsigned char *bufferPtr;
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS)
    {
        return ret;
    }
    HeadInfo head;
    this->getHeader(&head);
    int attrcount=head.numAttrs;
    int slotcount=head.numSlots;
    int recordsize=attrcount*ATTR_SIZE;
    int offset=HEADER_SIZE+slotcount+(recordsize*slotNum);
    unsigned char *slotpointer=bufferPtr+offset;
    memcpy(rec,slotpointer,recordsize);
    return SUCCESS;
}
int RecBuffer::setRecord(union Attribute *rec, int slotNum)
{
    unsigned char *bufferPtr;
    int y;
    y=loadBlockAndGetBufferPtr(&bufferPtr);
    if(y!=SUCCESS){
        return y;
    }
    HeadInfo head;
    this->getHeader(&head);
    int numattrs=head.numAttrs;
    int numslots=head.numSlots;
    if(slotNum<0 ||slotNum>numslots){
        return E_OUTOFBOUND;
    }
   int recordsize=ATTR_SIZE*numattrs;
   int offset=HEADER_SIZE+(slotNum*recordsize)+numslots;
   unsigned char *slotpointer=bufferPtr+offset;
   memcpy(slotpointer,rec,recordsize);

   int ret=StaticBuffer::setDirtyBit(this->blockNum);
   if(ret!=SUCCESS){
    std::cout<<"setdirty function not working";
   }

   return SUCCESS;
}

int RecBuffer::getSlotMap(unsigned char *slotMap)
{
    unsigned char *bufferPtr;

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS)
    {
        return ret;
    }

    struct HeadInfo head;
    this->getHeader(&head);
    int slotCount = head.numSlots;

    unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;
    memcpy(slotMap,slotMapInBuffer,slotCount);
    return SUCCESS;
}
int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType)
{

    double diff;
    if(attrType==STRING){
        diff=strcmp(attr1.sVal,attr2.sVal);
    }
    else{
        diff=attr1.nVal-attr2.nVal;
    }
    if(diff>0){
        return 1;
    }
    else if (diff <0)
    {
        return -1;
    }
    else
    {
        return 0;
    }

}
int RecBuffer::setSlotMap(unsigned char *slotMap)
{
    unsigned char *bufferPtr;

    int re=loadBlockAndGetBufferPtr(&bufferPtr);
    if(re!=SUCCESS){
        return re;
    }

    HeadInfo head;
    this->getHeader(&head);

    int numSlots = head.numSlots;
    slotMap[numSlots];
    unsigned char *slotpointer=HEADER_SIZE+bufferPtr;
    memcpy(slotpointer,slotMap,numSlots);
    int se=StaticBuffer::setDirtyBit(this->blockNum);
    if(se!=SUCCESS){
        return se;
    }
    return SUCCESS;
}

/*
Used to load a block to the buffer and get a pointer to it.
NOTE: this function expects the caller to allocate memory for the argument
*/

///////////////////Stage 2///////////
// BlockBuffer::BlockBuffer(int blockNum)
// {
//     this->blockNum=blockNum;
//     // initialise this.blockNum with the argument
// }

// // calls the parent class constructor

// // load the block header into the argument pointer
// int BlockBuffer::getHeader(struct HeadInfo *head)
// {
//     unsigned char buffer[BLOCK_SIZE];
//     Disk::readBlock(buffer,this->blockNum);
//     // read the block at this.blockNum into the buffer

//     // populate the numEntries, numAttrs and numSlots fields in *head
//     memcpy(&head->numSlots, buffer + 24, 4);
//     memcpy(&head->numEntries, buffer+16, 4);
//     memcpy(&head->numAttrs, buffer + 20, 4);
//     memcpy(&head->rblock,buffer+12, 4);
//     memcpy(&head->lblock, buffer+8, 4);

//     return SUCCESS;
// }

// // load the record at slotNum into the argument pointer
// int RecBuffer::getRecord(union Attribute *rec, int slotNum)
// {
//     struct HeadInfo head;

//     // get the header using this.getHeader() function
    
//     this->getHeader(&head);
//     int attrCount = head.numAttrs;
//     int slotCount = head.numSlots;
//     // read the block at this.blockNum into a buffer
//     unsigned char buffer[BLOCK_SIZE];
//     Disk::readBlock(buffer,this->blockNum);
//     /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
//        - each record will have size attrCount * ATTR_SIZE
//        - slotMap will be of size slotCount
//     */
//     int recordSize = attrCount * ATTR_SIZE;
//     int offset=HEADER_SIZE+slotCount+(recordSize*slotNum);
//     unsigned char *slotPointer =buffer+offset;
    
//     /* calculate buffer + offset */

//     // load the record into the rec data structure
//     memcpy(rec, slotPointer, recordSize);

//     return SUCCESS;
// }
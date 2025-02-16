#include "BlockBuffer.h"
#include <cstdlib>
#include <cstring>
BlockBuffer::BlockBuffer(int blockNum)
{
    this->blockNum=blockNum;

}
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}
/////////stage-5\\\\\\\


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

//////stage-5

/* NOTE: This function will NOT check if the block has been initialised as a
   record or an index block. It will copy whatever content is there in that
   disk block to the buffer.
   Also ensure that all the methods accessing and updating the block's data
   should call the loadBlockAndGetBufferPtr() function before the access or
   update is done. This is because the block might not be present in the
   buffer due to LRU buffer replacement. So, it will need to be bought back
   to the buffer before any operations can be done.
 */
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr)
{

    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);
    if(bufferNum!=E_BLOCKNOTINBUFFER){
        for(int i=0;i<32;i++){
            if(i==bufferNum){
                StaticBuffer::metainfo[bufferNum].timeStamp=0;
            }
            StaticBuffer::metainfo[i].timeStamp+=1;
        }

    }
    else{
        bufferNum =StaticBuffer::getFreeBuffer(this->blockNum);
        if(bufferNum==E_OUTOFBOUND){
            return E_OUTOFBOUND;
        }
        Disk::readBlock(StaticBuffer::blocks[bufferNum],this->blockNum);
        *buffPtr=StaticBuffer::blocks[bufferNum];
    }
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
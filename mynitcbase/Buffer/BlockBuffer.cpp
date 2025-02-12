#include "BlockBuffer.h"
#include <cstdlib>
#include <cstring>

*/
BlockBuffer::BlockBuffer(int blockNum)
{
    this->blockNum=blockNum;
}
 RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr)
{

    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

    if (bufferNum == E_BLOCKNOTINBUFFER)
    {
        bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

        if (bufferNum == E_OUTOFBOUND)
        {
            return E_OUTOFBOUND;
        }

        Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
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

int RecBuffer::getRecord(union Attribute *rec, int slotNum)
{

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


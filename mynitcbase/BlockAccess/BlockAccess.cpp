#include "BlockAccess.h"
#include <stdlib.h>
#include <cstring>
RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op)
{

    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId,&prevRecId);
    int block=-1;
    int slot=-1;
    if (prevRecId.block == -1 && prevRecId.slot == -1)
    {
        RelCatEntry relcatentry;
        RelCacheTable::getRelCatEntry(relId,&relcatentry);
        block=relcatentry.firstBlk;
        slot=0;

    }
    else
    {
        block=prevRecId.block;
        slot=prevRecId.slot+1;
    }

    while (block != -1)
    {
        RecBuffer recbuff(block);
        RelCatEntry relcatentry;
        HeadInfo head;
        recbuff.getHeader(&head);
        int numattr=head.numAttrs;
        Attribute blockrec[numattr];
        recbuff.getRecord(blockrec,slot);
        unsigned char slotmap[head.numSlots];
        recbuff.getSlotMap(slotmap);
        if(slot>=relcatentry.numSlotsPerBlk)
        {

            block=head.rblock;
            slot=0;
            continue;
        }

        else if(slotmap[slot]==SLOT_UNOCCUPIED)
        {
            slot++;
            continue;
        }
        AttrCatEntry attrcatbuff;
        AttrCacheTable::getAttrCatEntry(relId,attrName,&attrcatbuff);
        int attrofset=attrcatbuff.offset;
        Attribute record[numattr];
        recbuff.getRecord(record,slot);

        int cmpVal = compareAttrs(record[attrofset],attrVal, attrcatbuff.attrType);

        if (
            (op == NE && cmpVal != 0) || // if op is "not equal to"
            (op == LT && cmpVal < 0) ||  // if op is "less than"
            (op == LE && cmpVal <= 0) || // if op is "less than or equal to"
            (op == EQ && cmpVal == 0) || // if op is "equal to"
            (op == GT && cmpVal > 0) ||  // if op is "greater than"
            (op == GE && cmpVal >= 0)    // if op is "greater than or equal to"
        )
        {
        
           RecId search;
           search.block=block;
           search.slot=slot;

           RelCacheTable::setSearchIndex(relId,&search);

            return RecId{block, slot};
        }

        slot++;
    }

    return RecId{-1, -1};
}
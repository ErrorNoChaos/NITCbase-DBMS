#include "BlockAccess.h"
#include <stdlib.h>
#include <cstring>
#include "BlockAccess.h"
#include <cstring>
#include <iostream>

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
   

         block = prevRecId.block;
        slot = prevRecId.slot+ 1;
    }


    while (block != -1)
    {
       
        RecBuffer blocker(block);
        HeadInfo head;
        blocker.getHeader(&head);
        int numattrs=head.numAttrs;
        int slotnum=head.numSlots;
        unsigned char slotmap[slotnum];
        blocker.getSlotMap(slotmap);
        if(slot>=slotnum)
        {
          
            block=head.rblock;
            slot=0;
            continue; 
        }
        if(slotmap[slot]==SLOT_UNOCCUPIED)
        {
            slot++;
            continue;
          
        }

        AttrCatEntry attrcatentry;
        AttrCacheTable::getAttrCatEntry(relId,attrName,&attrcatentry);
        Attribute record[numattrs];
        blocker.getRecord(record,slot);
        int offset=attrcatentry.offset;
        int cmpVal=compareAttrs(record[offset],attrVal,attrcatentry.attrType);
      
        if (
            (op == NE && cmpVal != 0) || // if op is "not equal to"
            (op == LT && cmpVal < 0) ||  // if op is "less than"
            (op == LE && cmpVal <= 0) || // if op is "less than or equal to"
            (op == EQ && cmpVal == 0) || // if op is "equal to"
            (op == GT && cmpVal > 0) ||  // if op is "greater than"
            (op == GE && cmpVal >= 0)    // if op is "greater than or equal to"
        )
        {
       
           RecId searchindex;
           searchindex.block=block;
           searchindex.slot=slot;
           RelCacheTable::setSearchIndex(relId,&searchindex);
            return RecId{block, slot};
        }

        slot++;
    }

    return RecId{-1, -1};
}

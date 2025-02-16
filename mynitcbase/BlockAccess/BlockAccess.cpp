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


        RelCatEntry relcatentry;
        RelCacheTable::getRelCatEntry(relId,&relcatentry);
        if(slot>=relcatentry.numSlotsPerBlk)
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
int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE])
{
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */

    Attribute newRelationName; // set newRelationName with newName

    // search the relation catalog for an entry with "RelName" = newRelationName

    // If relation with name newName already exists (result of linearSearch
    //                                               is not {-1, -1})
    //    return E_RELEXIST;

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */

    Attribute oldRelationName; // set oldRelationName with oldName

    // search the relation catalog for an entry with "RelName" = oldRelationName

    // If relation with name oldName does not exist (result of linearSearch is {-1, -1})
    //    return E_RELNOTEXIST;

    /* get the relation catalog record of the relation to rename using a RecBuffer
       on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
    */
    /* update the relation name attribute in the record with newName.
       (use RELCAT_REL_NAME_INDEX) */
    // set back the record value using RecBuffer.setRecord

    /*
    update all the attribute catalog entries in the attribute catalog corresponding
    to the relation with relation name oldName to the relation name newName
    */

    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */

    // for i = 0 to numberOfAttributes :
    //     linearSearch on the attribute catalog for relName = oldRelationName
    //     get the record using RecBuffer.getRecord
    //
    //     update the relName field in the record to newName
    //     set back the record using RecBuffer.setRecord

    return SUCCESS;
}
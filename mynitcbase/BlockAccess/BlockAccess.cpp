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
  
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute newRelationName; 
    strcpy(newRelationName.sVal,newName);

    char attrv[ATTR_SIZE];
    strcpy(attrv,RELCAT_ATTR_RELNAME);
    RecId rec;
    rec=linearSearch(RELCAT_RELID,attrv,newRelationName,EQ);
    if(rec.block!=-1 && rec.slot!=-1){
        return E_RELEXIST;
    }

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute oldRelationName;
    strcpy(oldRelationName.sVal, oldName);

    char attrc[ATTR_SIZE];
    strcpy(attrc,RELCAT_ATTR_RELNAME);
    RecId reco;
    reco=linearSearch(RELCAT_RELID,attrc,oldRelationName,EQ);
    if(reco.block==-1 && reco.slot==-1){
        return E_RELNOTEXIST;
    }


   RecBuffer relcatblock(reco.block);
   Attribute record[RELCAT_NO_ATTRS];
   relcatblock.getRecord(record,reco.slot);
   strcpy(record[RELCAT_REL_NAME_INDEX].sVal,newName);
   relcatblock.setRecord(record,reco.slot);

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

   char attrname[ATTR_SIZE];
   strcpy(attrname,ATTRCAT_ATTR_RELNAME);
   for(int i=0;i<record[RELCAT_NO_ATTRIBUTES_INDEX].nVal;i++){
    RecId rec;
    rec=linearSearch(ATTRCAT_RELID,attrname,oldRelationName,EQ);
    RecBuffer attrblock(rec.block);
    Attribute reco[ATTRCAT_NO_ATTRS];
    attrblock.getRecord(reco,rec.slot);
    strcpy(reco[ATTRCAT_REL_NAME_INDEX].sVal,newName);
    attrblock.setRecord(reco,rec.slot);
   }

    return SUCCESS;
}
int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE])
{

   
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    Attribute relNameAttr;
    strcpy(relNameAttr.sVal,relName);
    char relname[ATTR_SIZE];
    strcpy(relname,RELCAT_ATTR_RELNAME);
    RecId searchindex=linearSearch(RELCAT_RELID,relname,relNameAttr,EQ);
    if(searchindex.block==-1 && searchindex.slot==-1){
       return  E_RELNOTEXIST;
    }

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    RecId attrToRenameRecId{-1, -1};
   
    char attrname[ATTR_SIZE];
    strcpy(attrname, ATTRCAT_ATTR_RELNAME);
    Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];
    while (true)
    {
        
        searchindex=linearSearch(ATTRCAT_RELID,attrname,relNameAttr,EQ);

        if(searchindex.block==-1 && searchindex.slot==-1){
            break;
        }
        RecBuffer attrcat(searchindex.block);
        attrcat.getRecord(attrCatEntryRecord,searchindex.slot);

        if(strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,oldName)==0){
            attrToRenameRecId.block=searchindex.block;
            attrToRenameRecId.slot=searchindex.slot;
            break;
        }
        if(strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName)==0){
            return E_ATTREXIST;
        }

    }

    if(attrToRenameRecId.block==-1 && attrToRenameRecId.slot==-1){
        return E_ATTRNOTEXIST;
    }
    RecBuffer blocker(attrToRenameRecId.block);
    Attribute record[ATTRCAT_NO_ATTRS];
    blocker.getRecord(record,attrToRenameRecId.slot);
    strcpy(record[ATTRCAT_ATTR_NAME_INDEX].sVal,newName);
    blocker.setRecord(record,attrToRenameRecId.slot);

    return SUCCESS;
}
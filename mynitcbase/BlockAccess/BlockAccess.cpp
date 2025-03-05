#include "BlockAccess.h"
#include <stdlib.h>
#include <cstring>
#include "BlockAccess.h"
#include <cstring>
#include <iostream>

RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op)
{

    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId, &prevRecId);

    int block = -1;
    int slot = -1;

    if (prevRecId.block == -1 && prevRecId.slot == -1)
    {
        RelCatEntry relcatentry;

        RelCacheTable::getRelCatEntry(relId, &relcatentry);

        block = relcatentry.firstBlk;
        slot = 0;
    }
    else
    {

        block = prevRecId.block;
        slot = prevRecId.slot + 1;
    }

    while (block != -1)
    {

        RecBuffer blocker(block);
        HeadInfo head;
        blocker.getHeader(&head);
        int numattrs = head.numAttrs;
        int slotnum = head.numSlots;
        unsigned char slotmap[slotnum];
        blocker.getSlotMap(slotmap);

        RelCatEntry relcatentry;
        RelCacheTable::getRelCatEntry(relId, &relcatentry);
        if (slot >= relcatentry.numSlotsPerBlk)
        {

            block = head.rblock;
            slot = 0;
            continue;
        }

        if (slotmap[slot] == SLOT_UNOCCUPIED)
        {
            slot++;
            continue;
        }

        AttrCatEntry attrcatentry;
        AttrCacheTable::getAttrCatEntry(relId, attrName, &attrcatentry);
        Attribute record[numattrs];
        blocker.getRecord(record, slot);
        int offset = attrcatentry.offset;
        int cmpVal = compareAttrs(record[offset], attrVal, attrcatentry.attrType);

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
            searchindex.block = block;
            searchindex.slot = slot;
            RelCacheTable::setSearchIndex(relId, &searchindex);
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
    strcpy(newRelationName.sVal, newName);

    char attrv[ATTR_SIZE];
    strcpy(attrv, RELCAT_ATTR_RELNAME);
    RecId rec;
    rec = linearSearch(RELCAT_RELID, attrv, newRelationName, EQ);
    if (rec.block != -1 && rec.slot != -1)
    {
        return E_RELEXIST;
    }

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute oldRelationName;
    strcpy(oldRelationName.sVal, oldName);

    char attrc[ATTR_SIZE];
    strcpy(attrc, RELCAT_ATTR_RELNAME);
    RecId reco;
    reco = linearSearch(RELCAT_RELID, attrc, oldRelationName, EQ);
    if (reco.block == -1 && reco.slot == -1)
    {
        return E_RELNOTEXIST;
    }

    RecBuffer relcatblock(reco.block);
    Attribute record[RELCAT_NO_ATTRS];
    relcatblock.getRecord(record, reco.slot);
    strcpy(record[RELCAT_REL_NAME_INDEX].sVal, newName);
    relcatblock.setRecord(record, reco.slot);

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    char attrname[ATTR_SIZE];
    strcpy(attrname, ATTRCAT_ATTR_RELNAME);
    for (int i = 0; i < record[RELCAT_NO_ATTRIBUTES_INDEX].nVal; i++)
    {
        RecId rec;
        rec = linearSearch(ATTRCAT_RELID, attrname, oldRelationName, EQ);
        RecBuffer attrblock(rec.block);
        Attribute reco[ATTRCAT_NO_ATTRS];
        attrblock.getRecord(reco, rec.slot);
        strcpy(reco[ATTRCAT_REL_NAME_INDEX].sVal, newName);
        attrblock.setRecord(reco, rec.slot);
    }

    return SUCCESS;
}
int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE])
{

    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    Attribute relNameAttr;
    strcpy(relNameAttr.sVal, relName);
    char relname[ATTR_SIZE];
    strcpy(relname, RELCAT_ATTR_RELNAME);
    RecId searchindex = linearSearch(RELCAT_RELID, relname, relNameAttr, EQ);
    if (searchindex.block == -1 && searchindex.slot == -1)
    {
        return E_RELNOTEXIST;
    }

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    RecId attrToRenameRecId{-1, -1};

    char attrname[ATTR_SIZE];
    strcpy(attrname, ATTRCAT_ATTR_RELNAME);
    Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];
    while (true)
    {

        searchindex = linearSearch(ATTRCAT_RELID, attrname, relNameAttr, EQ);

        if (searchindex.block == -1 && searchindex.slot == -1)
        {
            break;
        }
        RecBuffer attrcat(searchindex.block);
        attrcat.getRecord(attrCatEntryRecord, searchindex.slot);

        if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, oldName) == 0)
        {
            attrToRenameRecId.block = searchindex.block;
            attrToRenameRecId.slot = searchindex.slot;
            break;
        }
        if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName) == 0)
        {
            return E_ATTREXIST;
        }
    }

    if (attrToRenameRecId.block == -1 && attrToRenameRecId.slot == -1)
    {
        return E_ATTRNOTEXIST;
    }
    RecBuffer blocker(attrToRenameRecId.block);
    Attribute record[ATTRCAT_NO_ATTRS];
    blocker.getRecord(record, attrToRenameRecId.slot);
    strcpy(record[ATTRCAT_ATTR_NAME_INDEX].sVal, newName);
    blocker.setRecord(record, attrToRenameRecId.slot);

    return SUCCESS;
}
int BlockAccess::insert(int relId, Attribute *record)
{

    RelCatEntry relcatentry;
    RelCacheTable::getRelCatEntry(relId, &relcatentry);

    int blockNum = relcatentry.firstBlk;

    RecId rec_id = {-1, -1};

    int numOfSlots = relcatentry.numSlotsPerBlk;
    int numOfAttributes = relcatentry.numAttrs;

    int prevBlockNum = -1;

    while (blockNum != -1)
    {

        RecBuffer blockbuff(blockNum);
        HeadInfo head;
        blockbuff.getHeader(&head);
        unsigned char slotmap[numOfSlots];
        blockbuff.getSlotMap(slotmap);
        for (int i = 0; i < numOfSlots; i++)
        {
            if (slotmap[i] == SLOT_UNOCCUPIED)
            {
                rec_id.block = blockNum;
                rec_id.slot = i;
                break;
            }
        }
        if (rec_id.block != -1 && rec_id.slot != -1)
        {
            break;
        }
        prevBlockNum = blockNum;
        blockNum = head.rblock;
    }

    if (rec_id.block == -1 && rec_id.slot == -1)
    {
        if (strcmp(relcatentry.relName, "RELATIONCAT") == 0)
        {
            return E_MAXRELATIONS;
        }

        RecBuffer blockbuffer;
        int ret = blockbuffer.getBlockNum();

        if (ret == E_DISKFULL)
        {
            return E_DISKFULL;
        }

        rec_id.block = ret;
        rec_id.slot = 0;
        HeadInfo head;
        head.blockType = REC;
        head.pblock = -1;
        head.lblock = prevBlockNum;
        head.rblock = -1;
        head.numEntries = 0;
        head.numSlots = numOfSlots;
        head.numAttrs = numOfAttributes;
        blockbuffer.setHeader(&head);
        unsigned char slotmaper[numOfSlots];
        for (int i = 0; i < numOfSlots; i++)
        {
            slotmaper[i] = SLOT_UNOCCUPIED;
        }
        blockbuffer.setSlotMap(slotmaper);

        if (prevBlockNum != -1)
        {
            RecBuffer prevblock(prevBlockNum);
            HeadInfo prevhead;
            prevblock.getHeader(&prevhead);
            prevhead.rblock = rec_id.block;
            prevblock.setHeader(&prevhead);
        }
        else
        {
            relcatentry.firstBlk = rec_id.block;
            RelCacheTable::setRelCatEntry(relId, &relcatentry);
        }
    }
    RecBuffer finalbuff(rec_id.block);
    finalbuff.setRecord(record, rec_id.slot);
    unsigned char slotmap[numOfSlots];
    finalbuff.getSlotMap(slotmap);
    slotmap[rec_id.slot] = SLOT_OCCUPIED;
    finalbuff.setSlotMap(slotmap);
    HeadInfo finalhead;
    finalbuff.getHeader(&finalhead);
    finalhead.numEntries++;
    finalbuff.setHeader(&finalhead);
    relcatentry.numRecs++;
    RelCacheTable::setRelCatEntry(relId, &relcatentry);

    return SUCCESS;
}

int BlockAccess::search(int relId, Attribute *record, char attrName[ATTR_SIZE], Attribute attrVal, int op)
{

    RecId recId;
    recId = linearSearch(relId, attrName, attrVal, EQ);

    if (recId.block == -1 && recId.slot == -1)
    {
        return E_NOTFOUND;
    }
    RecBuffer recblock(recId.block);
    recblock.getRecord(record, recId.slot);

    return SUCCESS;
}
int BlockAccess::deleteRelation(char relName[ATTR_SIZE])
{
    if (strcmp(relName, "RELATIONCAT") == 0 || strcmp(relName, "ATTRIBUTECAT") == 0)
    {
        return E_NOTPERMITTED;
    }

    RelCacheTable::resetSearchIndex(0);

    Attribute relNameAttr;

    strcpy(relNameAttr.sVal, relName);
    char relname[16];
    strcpy(relname, RELCAT_ATTR_RELNAME);
    RecId recid = linearSearch(0, relname, relNameAttr, EQ);
    if (recid.block == -1 && recid.slot == -1)
    {
        return E_RELNOTEXIST;
    }
    RecBuffer buffblock(recid.block);
    Attribute relCatEntryRecord[RELCAT_NO_ATTRS];
    buffblock.getRecord(relCatEntryRecord, recid.slot);
    int firstblock = relCatEntryRecord[RELCAT_FIRST_BLOCK_INDEX].nVal;
    int numAttrs = relCatEntryRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal;

    int currentblock = firstblock;
    while (currentblock != -1)
    {
        HeadInfo head;
        RecBuffer blockbuff(currentblock);
        blockbuff.getHeader(&head);
        int nextblocke = head.rblock;
        blockbuff.releaseBlock();
        currentblock = nextblocke;
    }

    RelCacheTable::resetSearchIndex(1);
    int numberOfAttributesDeleted = 0;

    while (true)
    {
        RecId attrCatRecId;
        char attrcat[16];
        strcpy(attrcat, RELCAT_ATTR_RELNAME);
        attrCatRecId = linearSearch(1, attrcat, relNameAttr, EQ);

        if (attrCatRecId.block == -1 && attrCatRecId.slot == -1)
        {
            break;
        }

        numberOfAttributesDeleted++;
        RecBuffer bufferblocker(attrCatRecId.block);
        HeadInfo head;
        bufferblocker.getHeader(&head);
        Attribute record[ATTRCAT_NO_ATTRS];
        bufferblocker.getRecord(record, attrCatRecId.slot);
        int rootBlock = record[ATTRCAT_ROOT_BLOCK_INDEX].nVal;
        int numslots = head.numSlots;
        unsigned char slotmap[numslots];
        bufferblocker.getSlotMap(slotmap);
        slotmap[attrCatRecId.slot] = SLOT_UNOCCUPIED;
        bufferblocker.setSlotMap(slotmap);
        head.numEntries--;
        bufferblocker.setHeader(&head);

        if (head.numEntries == 0)
        {
            int leftblock = head.lblock;
            RecBuffer leftblocke(leftblock);
            HeadInfo lefthead;
            leftblocke.getHeader(&lefthead);
            lefthead.rblock = head.rblock;
            leftblocke.setHeader(&lefthead);

            if (head.rblock != -1)
            {
                int rightblock = head.rblock;
                RecBuffer rightblocker(rightblock);
                HeadInfo righthead;
                rightblocker.getHeader(&righthead);
                righthead.lblock = head.lblock;
                rightblocker.setHeader(&righthead);
            }
            else
            {
                RelCatEntry relcatbuff;
                RelCacheTable::getRelCatEntry(1, &relcatbuff);
                relcatbuff.lastBlk = head.lblock;
            }
            bufferblocker.releaseBlock();
        }

        // if (rootBlock != -1)
        // {
        //     BPlusTree::bPlusDestroy(rootBlock);
        // }
    }

    RecBuffer relcatblock(RELCAT_BLOCK);
    HeadInfo relcathead;
    relcatblock.getHeader(&relcathead);
    relcathead.numEntries--;
    relcatblock.setHeader(&relcathead);

    int numslots = relcathead.numSlots;
    unsigned char slotmap[numslots];
    relcatblock.getSlotMap(slotmap);
    slotmap[recid.slot] = SLOT_UNOCCUPIED;
    relcatblock.setSlotMap(slotmap);

    RelCatEntry relcatentry;
    RelCacheTable::getRelCatEntry(0, &relcatentry);
    relcatentry.numRecs--;
    RelCacheTable::setRelCatEntry(0, &relcatentry);

    RelCacheTable::getRelCatEntry(1, &relcatentry);
    relcatentry.numRecs = relcatentry.numRecs - numberOfAttributesDeleted;
    RelCacheTable::setRelCatEntry(1, &relcatentry);

    return SUCCESS;
}
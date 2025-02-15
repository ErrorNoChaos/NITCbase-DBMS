
#include "OpenRelTable.h"
#include <stdlib.h>
#include <cstring>

AttrCacheEntry *CreateAttrCacheEntry(int size)
{

    AttrCacheEntry *head = nullptr, *curr = nullptr;
    head = curr = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    int i = 0;
    while (i < size - 1)
    {
        curr->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
        curr = curr->next;
        i++;
    }
    curr->next = nullptr;
    return head;
}

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];
int OpenRelTable::getFreeOpenRelTableEntry()
{
    for (int i = 0; i < MAX_OPEN; i++)
    {
        if (tableMetaInfo[i].free)
        {
            return i;
        }
    }
    return E_CACHEFULL;

}
int OpenRelTable::getRelId(char relName[ATTR_SIZE])
{
    for (int i = 0; i < MAX_OPEN; i++)
    {
        if (strcmp(tableMetaInfo[i].relName, relName) == 0 && tableMetaInfo[i].free == false)
        {
            return i;
        }
    }
    return E_RELNOTOPEN;

}
int OpenRelTable::closeRel(int relId)
{
    if (relId == 0 || relId == 1)
    {
        return E_NOTPERMITTED;
    }

    if (relId < 0 || relId >= MAX_OPEN)
    {
        return E_OUTOFBOUND;
    }

    if (tableMetaInfo[relId].free)
    {
        return E_RELNOTOPEN;
    }

    if (AttrCacheTable::attrCache[relId] == nullptr)
    {
        return E_RELNOTOPEN;
    }

    // Free RelCache entry
    if (RelCacheTable::relCache[relId] != nullptr)
    {
        free(RelCacheTable::relCache[relId]);
        RelCacheTable::relCache[relId] = nullptr;
    }

    // Free AttrCache entries
    AttrCacheEntry *current = AttrCacheTable::attrCache[relId];
    while (current != nullptr)
    {
        AttrCacheEntry *temp = current;
        current = current->next;
        free(temp);
    }
    AttrCacheTable::attrCache[relId] = nullptr;

    // Mark as free
    tableMetaInfo[relId].free = true;

    return SUCCESS;
}



int OpenRelTable::openRel(char relName[ATTR_SIZE])
{
    int relId = getRelId(relName);
    if (relId >= 0)
    {

        return relId;
    
    }
   
    relId = OpenRelTable::getFreeOpenRelTableEntry();
    if (relId == E_CACHEFULL)
    {
        return E_CACHEFULL;
    }
 
    Attribute attrv;
    strcpy(attrv.sVal, relName);
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    RecId relcatRecId;
    char attrname[ATTR_SIZE];
    strcpy(attrname, RELCAT_ATTR_RELNAME);
    relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, attrname, attrv, EQ);

    if (relcatRecId.block == -1 && relcatRecId.slot == -1)
    {
     
        return E_RELNOTEXIST;
    }

 
    RecBuffer relationbuff(relcatRecId.block);
    Attribute relationRecord[RELCAT_NO_ATTRS];
    relationbuff.getRecord(relationRecord, relcatRecId.slot);
    RelCacheEntry *relcacheentry = nullptr;
    relcacheentry = (RelCacheEntry *)malloc(sizeof(RelCacheEntry));
    RelCacheTable::recordToRelCatEntry(relationRecord, &relcacheentry->relCatEntry);
    relcacheentry->recId.block = relcatRecId.block;
    relcacheentry->recId.slot = relcatRecId.slot;
    RelCacheTable::relCache[relId] = relcacheentry;

    Attribute attrcatrecord[ATTRCAT_NO_ATTRS];
    AttrCacheEntry *attrCacheEntry = nullptr, *head = nullptr;
    int numofattributes = RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
    head = CreateAttrCacheEntry(numofattributes);
    attrCacheEntry = head;
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    for (int i = 0; i < numofattributes; i++)
    {
       
        RecId attrcatRecId;
        attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, attrname, attrv, EQ);
        RecBuffer attrcatblock(attrcatRecId.block);
        attrcatblock.getRecord(attrcatrecord, attrcatRecId.slot);
        AttrCacheTable::recordToAttrCatEntry(attrcatrecord, &attrCacheEntry->attrCatEntry);
        attrCacheEntry->recId.block = attrcatRecId.block;
        attrCacheEntry->recId.slot = attrcatRecId.slot;
        attrCacheEntry = attrCacheEntry->next;

    }
    AttrCacheTable::attrCache[relId] = head;
    tableMetaInfo[relId].free = false;
    strcpy(tableMetaInfo[relId].relName, relName);


    return relId;
}

OpenRelTable::OpenRelTable()
{
    for (int i = 0; i < MAX_OPEN; i++)
    {
        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
        tableMetaInfo[i].free = true;
    }
    RecBuffer relCatBlock(RELCAT_BLOCK);

    Attribute relCatRecord[RELCAT_NO_ATTRS];
    RelCacheEntry *relCacheEntry = nullptr;
    for (int relId = RELCAT_RELID; relId <= ATTRCAT_RELID; relId++)
    {

        relCatBlock.getRecord(relCatRecord, relId);
        relCacheEntry = (RelCacheEntry *)malloc(sizeof(RelCacheEntry));
        RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry->relCatEntry);
        relCacheEntry->recId.block = RELCAT_BLOCK;
        relCacheEntry->recId.slot = relId;
        relCacheEntry->searchIndex = {-1, -1};
        RelCacheTable::relCache[relId] = relCacheEntry;
    }

    RecBuffer attrCatBlock(ATTRCAT_BLOCK);
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    AttrCacheEntry *attrCacheEntry = nullptr, *head = nullptr;

    for (int relId = RELCAT_RELID, recordId = 0; relId <= ATTRCAT_RELID; relId++)
    {
        int numberOfAttributes = RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
        head = CreateAttrCacheEntry(numberOfAttributes);
        attrCacheEntry = head;
        while (numberOfAttributes--)
        {
            attrCatBlock.getRecord(attrCatRecord, recordId);

            AttrCacheTable::recordToAttrCatEntry(attrCatRecord,
                                                 &(attrCacheEntry->attrCatEntry));
            attrCacheEntry->recId.slot = recordId++;
            attrCacheEntry->recId.block = ATTRCAT_BLOCK;

            attrCacheEntry = attrCacheEntry->next;
        }

        AttrCacheTable::attrCache[relId] = head;
    }
    tableMetaInfo[0].free = false;
    tableMetaInfo[1].free = false;
    strcpy(tableMetaInfo[0].relName, "RELATIONCAT");
    strcpy(tableMetaInfo[1].relName, "ATTRIBUTECAT");
  
}

OpenRelTable::~OpenRelTable()
{


    for (int i = 2; i < MAX_OPEN; ++i)
    {
        if (!tableMetaInfo[i].free)
        {
            OpenRelTable::closeRel(i); 
        }
    }
    free(RelCacheTable::relCache[RELCAT_RELID]);
    free(RelCacheTable::relCache[ATTRCAT_RELID]);
    RelCacheTable::relCache[RELCAT_RELID] = nullptr;
    RelCacheTable::relCache[ATTRCAT_RELID] = nullptr;
}
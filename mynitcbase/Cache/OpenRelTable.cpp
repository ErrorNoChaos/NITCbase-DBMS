
#include "OpenRelTable.h"
#include <stdlib.h>
#include <cstring>
/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.
*/
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
    /* traverse through the tableMetaInfo array,
      find a free entry in the Open Relation Table.*/

    // if found return the relation id, else return E_CACHEFULL.
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
    /* traverse through the tableMetaInfo array,
      find the entry in the Open Relation Table corresponding to relName.*/

    // if found return the relation id, else indicate that the relation do not
    // have an entry in the Open Relation Table.
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

// free the memory allocated in the relation and attribute caches which was
// allocated in the OpenRelTable::openRel() function

// update `tableMetaInfo` to set `relId` as a free slot
// update `relCache` and `attrCache` to set the entry at `relId` to nullptr

int OpenRelTable::openRel(char relName[ATTR_SIZE])
{
    int relId = getRelId(relName);
    if (relId >= 0)
    {
        // (checked using OpenRelTable::getRelId())
        return relId;
        // return that relation id;
    }
    /* find a free slot in the Open Relation Table
       using OpenRelTable::getFreeOpenRelTableEntry(). */
    relId = OpenRelTable::getFreeOpenRelTableEntry();
    if (relId == E_CACHEFULL)
    {
        return E_CACHEFULL;
    }
    // let relId be used to store the free slot.
    /****** Setting up Relation Cache entry for the relation ******/

    /* search for the entry with relation name, relName, in the Relation Catalog using
        BlockAccess::linearSearch().
        Care should be taken to reset the searchIndex of the relation RELCAT_RELID
        before calling linearSearch().*/

    // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
    Attribute attrv;
    strcpy(attrv.sVal, relName);
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    RecId relcatRecId;
    char attrname[ATTR_SIZE];
    strcpy(attrname, RELCAT_ATTR_RELNAME);
    relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, attrname, attrv, EQ);

    if (relcatRecId.block == -1 && relcatRecId.slot == -1)
    {
        // (the relation is not found in the Relation Catalog.)
        return E_RELNOTEXIST;
    }

    /* read the record entry corresponding to relcatRecId and create a relCacheEntry
        on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
        update the recId field of this Relation Cache entry to relcatRecId.
        use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
      NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
    */
    RecBuffer relationbuff(relcatRecId.block);
    Attribute relationRecord[RELCAT_NO_ATTRS];
    relationbuff.getRecord(relationRecord, relcatRecId.slot);
    RelCacheEntry *relcacheentry = nullptr;
    relcacheentry = (RelCacheEntry *)malloc(sizeof(RelCacheEntry));
    RelCacheTable::recordToRelCatEntry(relationRecord, &relcacheentry->relCatEntry);
    relcacheentry->recId.block = relcatRecId.block;
    relcacheentry->recId.slot = relcatRecId.slot;
    RelCacheTable::relCache[relId] = relcacheentry;
    /****** Setting up Attribute Cache entry for the relation ******/

    // let listHead be used to hold the head of the linked list of attrCache entries.
    /*iterate over all the entries in the Attribute Catalog corresponding to each
    attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
    care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
    corresponding to Attribute Catalog before the first call to linearSearch().*/
    Attribute attrcatrecord[ATTRCAT_NO_ATTRS];
    AttrCacheEntry *attrCacheEntry = nullptr, *head = nullptr;
    int numofattributes = RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
    head = CreateAttrCacheEntry(numofattributes);
    attrCacheEntry = head;
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    for (int i = 0; i < numofattributes; i++)
    {
        /* let attrcatRecId store a valid record id an entry of the relation, relName,
        in the Attribute Catalog.*/
        RecId attrcatRecId;
        attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, attrname, attrv, EQ);
        RecBuffer attrcatblock(attrcatRecId.block);
        attrcatblock.getRecord(attrcatrecord, attrcatRecId.slot);
        AttrCacheTable::recordToAttrCatEntry(attrcatrecord, &attrCacheEntry->attrCatEntry);
        attrCacheEntry->recId.block = attrcatRecId.block;
        attrCacheEntry->recId.slot = attrcatRecId.slot;
        attrCacheEntry = attrCacheEntry->next;
        /* read the record entry corresponding to attrcatRecId and create an
        Attribute Cache entry on it using RecBuffer::getRecord() and
        AttrCacheTable::recordToAttrCatEntry().
        update the recId field of this Attribute Cache entry to attrcatRecId.
        add the Attribute Cache entry to the linked list of listHead .*/
        // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
    }
    AttrCacheTable::attrCache[relId] = head;
    tableMetaInfo[relId].free = false;
    strcpy(tableMetaInfo[relId].relName, relName);
    // set the relIdth entry of the AttrCacheTable to listHead.

    /****** Setting up metadata in the Open Relation Table for the relation******/

    // update the relIdth entry of the tableMetaInfo with free as false and
    // relName as the input.

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
    // caching happening

    // initialise all values in relCache and attrCache to be nullptr and all entries
    // in tableMetaInfo to be free

    // load the relation and attribute catalog into the relation cache (we did this already)

    // load the relation and attribute catalog into the attribute cache (we did this already)
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
    /************ Setting up tableMetaInfo entries ************/
    tableMetaInfo[0].free = false;
    tableMetaInfo[1].free = false;
    strcpy(tableMetaInfo[0].relName, "RELATIONCAT");
    strcpy(tableMetaInfo[1].relName, "ATTRIBUTECAT");
    // in the tableMetaInfo arra
    //   set free = false for RELCAT_RELID and ATTRCAT_RELID
    //   set relname for RELCAT_RELID and ATTRCAT_RELID
}

OpenRelTable::~OpenRelTable()
{

    // close all open relations (from rel-id = 2 onwards. Why?)
    for (int i = 2; i < MAX_OPEN; ++i)
    {
        if (!tableMetaInfo[i].free)
        {
            OpenRelTable::closeRel(i); // we will implement this function later
        }
    }
    free(RelCacheTable::relCache[RELCAT_RELID]);
    free(RelCacheTable::relCache[ATTRCAT_RELID]);
    RelCacheTable::relCache[RELCAT_RELID] = nullptr;
    RelCacheTable::relCache[ATTRCAT_RELID] = nullptr;
    // free the memory allocated for rel-id 0 and 1 in the caches
}
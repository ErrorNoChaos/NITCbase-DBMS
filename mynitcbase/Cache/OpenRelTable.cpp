
#include "OpenRelTable.h"
#include <cstring>
#include <stdlib.h>

OpenRelTable::OpenRelTable()
{

    for (int i = 0; i < MAX_OPEN; ++i)
    {
        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
    }
    //relcat caching in relcache
    RecBuffer relCatBlock(RELCAT_BLOCK);

    Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);
    
    struct RelCacheEntry relCacheEntry;
    RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

    RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;
    //attrcat caching in relcache
    relCatBlock.getRecord(relCatRecord,RELCAT_SLOTNUM_FOR_ATTRCAT);
    RelCacheTable::recordToRelCatEntry(relCatRecord,&relCacheEntry.relCatEntry);
    relCacheEntry.recId.block=ATTRCAT_BLOCK;
    relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;
    RelCacheTable::relCache[ATTRCAT_RELID] =(struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[ATTRCAT_RELID]) = relCacheEntry;
    //student caching in relcache
    relCatBlock.getRecord(relCatRecord, 2);
    RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = 2;
    RelCacheTable::relCache[2] = (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[2]) = relCacheEntry;





    RecBuffer attrCatBlock(ATTRCAT_BLOCK);
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    AttrCacheEntry *head = nullptr;
     AttrCacheEntry *curr=nullptr;

    for(int i=0;i<6;i++){

        AttrCacheEntry *newEntry = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
                attrCatBlock.getRecord(attrCatRecord,i);
                AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&newEntry->attrCatEntry);
                newEntry->recId.block = ATTRCAT_BLOCK;
                newEntry->recId.slot = i;
                newEntry->next=nullptr;
                if(head==nullptr){
                    head=newEntry;
                    curr=newEntry;
                }
                else{
                    curr->next=newEntry;
                    curr=newEntry;
                }
    }
        AttrCacheTable::attrCache[RELCAT_RELID] = head;
        head=nullptr;
        curr=nullptr;
        for(int i=6;i<12;i++){

            AttrCacheEntry *newentry=(AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
            attrCatBlock.getRecord(attrCatRecord,i);
            AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&newentry->attrCatEntry);
            newentry->recId.block=ATTRCAT_BLOCK;
            newentry->recId.slot=i;
            newentry->next=nullptr;
            if(head==nullptr){
                head=newentry;
                curr=newentry;
            }else{
                curr->next=newentry;
                curr=newentry;
            }
            
        }
        AttrCacheTable::attrCache[ATTRCAT_RELID] = head;
        head=nullptr;
        curr=nullptr;
        for(int i=12;i<16;i++){
            AttrCacheEntry *newentry = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
            attrCatBlock.getRecord(attrCatRecord, i);
            AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &newentry->attrCatEntry);
            newentry->recId.block = ATTRCAT_BLOCK;
            newentry->recId.slot = i;
            newentry->next = nullptr;
            if (head == nullptr)
            {
                head = newentry;
                curr = newentry;
            }
            else
            {
                curr->next = newentry;
                curr = newentry;
            }
        }
        AttrCacheTable::attrCache[2]=head;
}

OpenRelTable::~OpenRelTable()
{

    for(int i=0;i<12;i++){
        if(RelCacheTable::relCache[i]!=nullptr){
            free(RelCacheTable::relCache[i]);
            RelCacheTable::relCache[i]=nullptr;
        }
        AttrCacheEntry *curr=AttrCacheTable::attrCache[i];
        while(curr!=nullptr){
            AttrCacheEntry *temp=curr->next;
            free(curr);
            curr=temp;
        }
        AttrCacheTable::attrCache[i]=nullptr;
    
    }

}
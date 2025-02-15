#include "Algebra.h"
#include <stdlib.h>
#include <cstring>
#include <stdio.h>

/* used to select all the records that satisfy a condition.
the arguments of the function are
- srcRel - the source relation we want to select from
- targetRel - the relation we want to select into. (ignore for now)
- attr - the attribute that the condition is checking
- op - the operator of the condition
- strVal - the value that we want to compare against (represented as a string)
*/


bool isNumber(char *str)
{
    int len;
    float ignore;

    int ret = sscanf(str, "%f %n", &ignore, &len);
    return ret == 1 && len == strlen(str);
}
int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE])
{
    int srcRelId = OpenRelTable::getRelId(srcRel);
    if (srcRelId == E_RELNOTOPEN)
    {
        return E_RELNOTOPEN;
    }

    // Get attribute catalog entry for the attribute we're searching on
    AttrCatEntry attrCatEntry;
    int ret = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);
    if (ret == E_ATTRNOTEXIST)
    {
        return E_ATTRNOTEXIST;
    }

    // Convert search value to appropriate type
    Attribute attrVal;
    if (attrCatEntry.attrType == NUMBER)
    {
        if (!isNumber(strVal))
        {
            return E_ATTRTYPEMISMATCH;
        }
        attrVal.nVal = atof(strVal);
    }
    else
    {
        strcpy(attrVal.sVal, strVal);
    }

    // Reset search index for this relation
    RelCacheTable::resetSearchIndex(srcRelId);

    // Get relation catalog entry for header information
    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);

    // Print header
    printf("|");
    for (int i = 0; i < relCatEntry.numAttrs; ++i)
    {
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);
        printf(" %s |", attrCatEntry.attrName);
    }
    printf("\n");

    // Search and print matching records
    while (true)
    {
        RecId searchRes = BlockAccess::linearSearch(srcRelId, attr, attrVal, op);
        if (searchRes.block == -1 || searchRes.slot == -1)
        {
            break;
        }

        RecBuffer recBuf(searchRes.block);
        HeadInfo head;
        recBuf.getHeader(&head);

        // Get and print the record
        printf("|");
        Attribute record[head.numAttrs];
        recBuf.getRecord(record, searchRes.slot);

        for (int i = 0; i < relCatEntry.numAttrs; i++)
        {
            AttrCatEntry attrCatEntry;
            AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);

            if (attrCatEntry.attrType == NUMBER)
            {
                printf(" %d |", (int)record[i].nVal);
            }
            else
            {
                printf(" %s |", record[i].sVal);
            }
        }
        printf("\n");
    }

    return SUCCESS;
}

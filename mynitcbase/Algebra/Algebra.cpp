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
    int srcRelId = OpenRelTable::getRelId(srcRel); // we'll implement this later
    if (srcRelId == E_RELNOTOPEN)
    {
        return E_RELNOTOPEN;
    }

    AttrCatEntry attrCatEntry;
    if (AttrCacheTable::getAttrCatEntry(srcRelId,attr,&attrCatEntry) == E_ATTRNOTEXIST) {
        return E_ATTRNOTEXIST;
    }

    /*** Convert strVal (string) to an attribute of data type NUMBER or STRING ***/
    int type = attrCatEntry.attrType;
    Attribute attrVal;
    if (type == NUMBER)
    {
        if (isNumber(strVal))
        { // the isNumber() function is implemented below
            attrVal.nVal = atof(strVal);
        }
        else
        {
            return E_ATTRTYPEMISMATCH;
        }
    }
    else if (type == STRING)
    {
        strcpy(attrVal.sVal, strVal);
    }


    RelCacheTable::resetSearchIndex(srcRelId);
    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(srcRelId,&relCatEntry);

    printf("|");
    for (int i = 0; i < relCatEntry.numAttrs; ++i)
    {
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(srcRelId,i,&attrCatEntry);

        printf(" %s |", attrCatEntry.attrName);
    }
    printf("\n");

    while (true)
    {
        RecId searchRes = BlockAccess::linearSearch(srcRelId, attr, attrVal, op);

        if (searchRes.block != -1 && searchRes.slot != -1)
        {
            RecBuffer Blockbuffer(searchRes.block);
            HeadInfo head;
            Blockbuffer.getHeader(&head);
            int numattrs=head.numAttrs;
            Attribute record[numattrs];
            Blockbuffer.getRecord(record,searchRes.slot);
            AttrCatEntry tempAttrCatEntry;
            for (int i = 0; i < relCatEntry.numAttrs; i++)
            {
                
                AttrCacheTable::getAttrCatEntry(srcRelId, i, &tempAttrCatEntry);

                if (tempAttrCatEntry.attrType == NUMBER)
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
        else
        {

            break;
        }
    }

    return SUCCESS;
}


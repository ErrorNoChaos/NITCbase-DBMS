#include "Schema.h"

#include <cmath>
#include <cstring>
int Schema::openRel(char relName[ATTR_SIZE])
{
    int ret = OpenRelTable::openRel(relName);
    if (ret < 0 || ret >= MAX_OPEN)
    {
        return E_OUTOFBOUND;
    }
    if (ret >= 0)
    {
        return SUCCESS;
    }

    return ret;
}

int Schema::closeRel(char relName[ATTR_SIZE])
{
    if (strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0)
    {
        return E_NOTPERMITTED;
    }
    int relId = OpenRelTable::getRelId(relName);

    if (relId == E_RELNOTOPEN)
    {
        return E_RELNOTOPEN;
    }

    return OpenRelTable::closeRel(relId);
}
int Schema::renameRel(char oldRelName[ATTR_SIZE], char newRelName[ATTR_SIZE])
{
    if (strcmp(oldRelName, "RELATIONCAT") == 0 || strcmp(newRelName, "ATTRIBUTECAT") == 0 || strcmp(oldRelName, "ATTRIBUTECAT") == 0 || strcmp(newRelName, "RELATIONCAT") == 0)
    {
        return E_NOTPERMITTED;
    }
    int reli = OpenRelTable::getRelId(oldRelName);
    if (reli != E_RELNOTOPEN)
    {
        return E_RELOPEN;
    }
    int retVal = BlockAccess::renameRelation(oldRelName, newRelName);
    return retVal;
}
int Schema::renameAttr(char *relName, char *oldAttrName, char *newAttrName)

{

    if (strcmp(oldAttrName, "RELATIONCAT") == 0 || strcmp(oldAttrName, "ATTRIBUTECAT") == 0 || strcmp(newAttrName, "ATTRIBUTECAT") == 0 || strcmp(newAttrName, "RELATIONCAT") == 0)
    {
        return E_NOTPERMITTED;
    }
    if (OpenRelTable::getRelId(relName) != E_RELNOTOPEN)
    {
        return E_RELOPEN;
    }
    int retVal = BlockAccess::renameAttribute(relName, oldAttrName, newAttrName);
    return retVal;
}
int Schema::createRel(char relName[], int nAttrs, char attrs[][ATTR_SIZE], int attrtype[])
{

    Attribute relNameAsAttribute;
    strcpy((char *)relNameAsAttribute.sVal, (const char *)relName);
    RecId targetRelId;
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    char dotrelname[16];
    strcpy(dotrelname, "RelName");
    targetRelId = BlockAccess::linearSearch(RELCAT_RELID, dotrelname, relNameAsAttribute, EQ);
    if (targetRelId.block != -1 && targetRelId.slot != -1)
    {
        return E_RELEXIST;
    }
    for (int i = 0; i < nAttrs ; i++)
    {
        for (int j = i + 1; j < nAttrs; j++)
        {
            if (strcmp(attrs[i], attrs[j]) == 0)
            {
                return E_DUPLICATEATTR;
            }
        }
    }
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    strcpy(relCatRecord[RELCAT_REL_NAME_INDEX].sVal, relName);
    relCatRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal = nAttrs;
    relCatRecord[RELCAT_NO_RECORDS_INDEX].nVal = 0;
    relCatRecord[RELCAT_FIRST_BLOCK_INDEX].nVal = -1;
    relCatRecord[RELCAT_LAST_BLOCK_INDEX].nVal = -1;
    relCatRecord[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal = floor((2016 * 1.00) / (16 * nAttrs + 1));
    int retVal = BlockAccess::insert(RELCAT_RELID, relCatRecord);
    if (retVal != SUCCESS)
    {
        return retVal;
    }

    for (int i = 0; i < nAttrs; i++)
    {

        Attribute attrCatRecord[6];
        strcpy(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relName);
        strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrs[i]);
        attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal=attrtype[i];
        attrCatRecord[ATTRCAT_PRIMARY_FLAG_INDEX].nVal = -1;
        attrCatRecord[ATTRCAT_ROOT_BLOCK_INDEX].nVal = -1;
        attrCatRecord[ATTRCAT_OFFSET_INDEX].nVal = i;

        int retval = BlockAccess::insert(1, attrCatRecord);
        if (retval != SUCCESS)
        {
            Schema::deleteRel(relName);
            return E_DISKFULL;
        }
    }

    return SUCCESS;
}
int Schema::deleteRel(char *relName)
{

    if (strcmp(relName, "RELATIONCAT") == 0 || strcmp(relName, "ATTRIBUTECAT") == 0)
    {
        return E_NOTPERMITTED;
    }

    int relid = OpenRelTable::getRelId(relName);

    if (relid >= 0)
    {
        return E_RELOPEN;
    }

    int rtval = BlockAccess::deleteRelation(relName);
    return rtval;
}
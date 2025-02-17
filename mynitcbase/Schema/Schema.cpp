#include "Schema.h"

#include <cmath>
#include <cstring>
int Schema::openRel(char relName[ATTR_SIZE])
{
    int ret = OpenRelTable::openRel(relName);
    if (ret < 0 || ret>= MAX_OPEN)
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
    if (strcmp(relName,RELCAT_RELNAME)==0||strcmp(relName,ATTRCAT_RELNAME)==0)
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
    int reli=OpenRelTable::getRelId(oldRelName);
    if(reli!=E_RELNOTOPEN){
        return E_RELOPEN;
    }
    int retVal=BlockAccess::renameRelation(oldRelName,newRelName);
    return retVal;

}
int Schema::renameAttr(char *relName, char *oldAttrName, char *newAttrName)

{

    if (strcmp(oldAttrName, "RELATIONCAT") == 0 || strcmp(oldAttrName, "ATTRIBUTECAT") == 0 || strcmp(newAttrName, "ATTRIBUTECAT") == 0 || strcmp(newAttrName, "RELATIONCAT") == 0)
    {
        return E_NOTPERMITTED;
    }
    if(OpenRelTable::getRelId(relName)!=E_RELNOTOPEN){
        return E_RELOPEN;
    }
      return E_RELOPEN;

    int retVal=BlockAccess::renameAttribute(relName,oldAttrName,newAttrName);
    return retVal;
    
}
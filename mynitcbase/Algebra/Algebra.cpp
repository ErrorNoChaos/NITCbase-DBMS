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
// int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE])
// {
//     int srcRelId = OpenRelTable::getRelId(srcRel);
//     if (srcRelId == E_RELNOTOPEN)
//     {
//         return E_RELNOTOPEN;
//     }

//     // Get attribute catalog entry for the attribute we're searching on
//     AttrCatEntry attrCatEntry;
//     int ret = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);
//     if (ret == E_ATTRNOTEXIST)
//     {
//         return E_ATTRNOTEXIST;
//     }

//     // Convert search value to appropriate type
//     Attribute attrVal;
//     if (attrCatEntry.attrType == NUMBER)
//     {
//         if (!isNumber(strVal))
//         {
//             return E_ATTRTYPEMISMATCH;
//         }
//         attrVal.nVal = atof(strVal);
//     }
//     else
//     {
//         strcpy(attrVal.sVal, strVal);
//     }

//     // Reset search index for this relation
//     RelCacheTable::resetSearchIndex(srcRelId);

//     // Get relation catalog entry for header information
//     RelCatEntry relCatEntry;
//     RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);

//     // Print header
//     printf("|");
//     for (int i = 0; i < relCatEntry.numAttrs; ++i)
//     {
//         AttrCatEntry attrCatEntry;
//         AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);
//         printf(" %s |", attrCatEntry.attrName);
//     }
//     printf("\n");

//     // Search and print matching records
//     while (true)
//     {
//         RecId searchRes = BlockAccess::linearSearch(srcRelId, attr, attrVal, op);
//         if (searchRes.block == -1 || searchRes.slot == -1)
//         {
//             break;
//         }

//         RecBuffer recBuf(searchRes.block);
//         HeadInfo head;
//         recBuf.getHeader(&head);

//         // Get and print the record
//         printf("|");
//         Attribute record[head.numAttrs];
//         recBuf.getRecord(record, searchRes.slot);

//         for (int i = 0; i < relCatEntry.numAttrs; i++)
//         {
//             AttrCatEntry attrCatEntry;
//             AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);

//             if (attrCatEntry.attrType == NUMBER)
//             {
//                 printf(" %d |", (int)record[i].nVal);
//             }
//             else
//             {
//                 printf(" %s |", record[i].sVal);
//             }
//         }
//         printf("\n");
//     }

//     return SUCCESS;
// }
int Algebra::insert(char relName[ATTR_SIZE], int nAttrs, char record[][ATTR_SIZE])
{

    if(strcmp(relName,"RELATIONCAT")==0 || strcmp(relName,"ATTRIBUTECAT")==0){
        return E_NOTPERMITTED;
    }

    int relId = OpenRelTable::getRelId(relName);
    if(relId==E_RELNOTOPEN){
        return E_RELNOTOPEN;
    }

    RelCatEntry relcatentry;
    RelCacheTable::getRelCatEntry(relId,&relcatentry);
    
    if(relcatentry.numAttrs!=nAttrs){
        return E_NATTRMISMATCH;
    }

    Attribute recordValues[nAttrs];
    for(int i=0;i<nAttrs;i++)
    {
     
        AttrCatEntry attrcatentry;
        AttrCacheTable::getAttrCatEntry(relId,i,&attrcatentry);

       
        int type=attrcatentry.attrType;

        if (type == NUMBER)
        {
           
            if(isNumber(record[i]))
            {
               
                recordValues[i].nVal=atof(record[i]);

            }
            else
            {
                return E_ATTRTYPEMISMATCH;
            }
        }
        else if (type == STRING)
        {
        
            strcpy(recordValues[i].sVal,record[i]);
        }
    }

    int retVal= BlockAccess::insert(relId,recordValues);

    return retVal;
}
int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE])
{
   
    int srcRelid=OpenRelTable::getRelId(srcRel);
    if(srcRelid==E_RELNOTOPEN){
        return E_RELNOTOPEN;
    }

   
    AttrCatEntry attrCatEntry;
    int ret=AttrCacheTable::getAttrCatEntry(srcRelid,attr,&attrCatEntry);
    if(ret==E_ATTRNOTEXIST){
        return ret;
    }

    Attribute attrVal;
    int type = attrCatEntry.attrType;

    if (type == NUMBER)
    {
      

        if(isNumber(strVal))
        {
            
            attrVal.nVal=atof(strVal);
            
        }
        else
        {
            return E_ATTRTYPEMISMATCH;
        }
    }
    else if (type == STRING)
    {
        
        strcpy(attrVal.sVal,strVal);
    }

    
    RelCatEntry srcrelcatentry;
    RelCacheTable::getRelCatEntry(srcRelid,&srcrelcatentry);
    int src_nAttrs =srcrelcatentry.numAttrs;

    char attr_names[src_nAttrs][ATTR_SIZE];
    int attr_types[src_nAttrs];
   
    for(int i=0;i<src_nAttrs;i++){
        AttrCatEntry attrcatentry;
        AttrCacheTable::getAttrCatEntry(srcRelid,i,&attrcatentry);
        strcpy(attr_names[i],attrcatentry.attrName);
        attr_types[i]=attrcatentry.attrType;       
    }
    ret=Schema::createRel(targetRel,src_nAttrs,attr_names,attr_types);
    if(ret!=SUCCESS)
{
    return ret;
}

    int targetrelid=OpenRelTable::openRel(targetRel);
    if(targetrelid<0 || targetrelid>=MAX_OPEN){
        Schema::deleteRel(targetRel);
        return targetrelid;
    }
    Attribute record[src_nAttrs];

    RelCacheTable::resetSearchIndex(srcRelid);
    //AttrCacheTable::resetSearchIndex(srcRelid,attr);  //will implement in B++ 

    while (BlockAccess::search(srcRelid,record,attr,attrVal,op)==SUCCESS)
    {

        ret = BlockAccess::insert(targetrelid, record);
        if(ret!=SUCCESS){
            OpenRelTable::closeRel(targetrelid);
            Schema::deleteRel(targetRel);
            return ret;
        }
       
    }

    OpenRelTable::closeRel(targetrelid);

    return SUCCESS;
}
int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE])
{

    int srcRelId = OpenRelTable::getRelId(srcRel);
    if(srcRelId==E_RELNOTOPEN){
        return E_RELNOTOPEN;
    }
    RelCatEntry relcatentry;
    RelCacheTable::getRelCatEntry(srcRelId,&relcatentry);
    int numAttrs=relcatentry.numAttrs;

      
    char attrNames[numAttrs][ATTR_SIZE];
    int attrTypes[numAttrs];

  
   for(int i=0;i<numAttrs;i++){
    AttrCatEntry attrcatentry;
    AttrCacheTable::getAttrCatEntry(srcRelId,i,&attrcatentry);
    strcpy(attrNames[i],attrcatentry.attrName);
    attrTypes[i]=attrcatentry.attrType;
   }

   
    int ret=Schema::createRel(targetRel,numAttrs,attrNames,attrTypes);
    if(ret!=SUCCESS){
        return ret;
    }
   
    int targetrelid=OpenRelTable::openRel(targetRel);
    if(targetrelid<0 ||targetrelid>=12){
        Schema::deleteRel(targetRel);
        return targetrelid;
    }

   
    RelCacheTable::resetSearchIndex(srcRelId);

    Attribute record[numAttrs];

    while (BlockAccess::project(srcRelId, record)==SUCCESS)
    {

        ret = BlockAccess::insert(targetrelid, record);

        if (ret!=SUCCESS)
        {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
           
        }
    }

    //Schema::closeRel(targetRel);

    return SUCCESS;
}
int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], int tar_nAttrs, char tar_Attrs[][ATTR_SIZE])
{

    int srcRelId = OpenRelTable::getRelId(srcRel);
    if(srcRelId==E_RELNOTOPEN){
        return E_RELNOTOPEN;
    }

    RelCatEntry relcatentry;
    RelCacheTable::getRelCatEntry(srcRelId,&relcatentry);
    int numAttrs=relcatentry.numAttrs;
    int attr_offset[tar_nAttrs];
    int attr_types[tar_nAttrs];

        

       
        for(int i=0;i<tar_nAttrs;i++){
            AttrCatEntry attrcatentry;
            int ret=AttrCacheTable::getAttrCatEntry(srcRelId,tar_Attrs[i],&attrcatentry);


            if(ret!=SUCCESS){
                return ret;
            }
            attr_offset[i]=attrcatentry.offset;
            attr_types[i]=attrcatentry.attrType;

        }


       int ret=Schema::createRel(targetRel,tar_nAttrs,tar_Attrs,attr_types);
       if(ret!=SUCCESS){
        return ret;
       }


       
       int targetrelid=OpenRelTable::openRel(targetRel);
       if(targetrelid<0 ||targetrelid>=12){
        Schema::deleteRel(targetRel);
        return targetrelid;
       }
       
       RelCacheTable::resetSearchIndex(srcRelId);
        Attribute record[numAttrs];

    while (BlockAccess::project(srcRelId, record)==SUCCESS)
    {
     

        Attribute proj_record[tar_nAttrs];

       
        for(int i=0;i<tar_nAttrs;i++){
            proj_record[i] = record[attr_offset[i]];
        }

        int ret = BlockAccess::insert(targetrelid, proj_record);

        if (ret!=SUCCESS)
        {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }

    Schema::closeRel(targetRel);

    return SUCCESS;
}
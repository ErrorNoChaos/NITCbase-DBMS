#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
using namespace std;





///////////////Prac Queastion 2/////////
int main(int argc, char *argv[])
{
  Disk disk_run;
  RecBuffer relCatBlock(RELCAT_BLOCK);
  HeadInfo relCatHead;
  HeadInfo attrCatHead;
  relCatBlock.getHeader(&relCatHead);
  for (int i = 0; i < relCatHead.numEntries; i++)
  {
    int currentblock = ATTRCAT_BLOCK;
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBlock.getRecord(relCatRecord, i);
    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);
    while (currentblock != -1)
    {
      RecBuffer attrCatBlock(currentblock);
      attrCatBlock.getHeader(&attrCatHead);
      for (int j = 0; j < attrCatHead.numEntries; j++)
      {
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatBlock.getRecord(attrCatRecord, j);
        if (strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal, attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal) == 0)
        {
          if(strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,"Students")==0){
            if(strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,"Class")==0){
              strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,"Batch");
            }
          }
          const char *attrtype = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
          printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrtype);
        }
      }
      currentblock = attrCatHead.rblock;
    }
  }
}


/////////////practise //////////////////////////////////////////////////////////////////////
// int main(int argc, char *argv[])
// {
//   Disk disk_run;
//   RecBuffer relCatBlock(RELCAT_BLOCK);
//   HeadInfo relCatHead;
//   HeadInfo attrCatHead;
//   relCatBlock.getHeader(&relCatHead);
//   for(int i=0;i<relCatHead.numEntries;i++){
//     int currentblock = ATTRCAT_BLOCK;
//     Attribute relCatRecord[RELCAT_NO_ATTRS];
//     relCatBlock.getRecord(relCatRecord,i);
//     printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);
//     while(currentblock!=-1){
//       RecBuffer attrCatBlock(currentblock);
//       attrCatBlock.getHeader(&attrCatHead);
//       for(int j=0;j<attrCatHead.numEntries;j++){
//         Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
//         attrCatBlock.getRecord(attrCatRecord,j);
//         if(strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal)==0){
//           const char *attrtype=attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal==NUMBER ? "NUM":"STR";
//           printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrtype);
//         }
//       }
//       currentblock = attrCatHead.rblock;
      
//     }
//     }

// }

/////////////////////////Stage 2 example question///////////////////////////

// int main(int argc, char *argv[])
// {
//   Disk disk_run;

//   // create objects for the relation catalog and attribute catalog
//   RecBuffer relCatBuffer(RELCAT_BLOCK);
//   RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

//   HeadInfo relCatHeader;
//   HeadInfo attrCatHeader;

//   // load the headers of both the blocks into relCatHeader and attrCatHeader.
//   // (we will implement these functions later)
//   relCatBuffer.getHeader(&relCatHeader);
//   attrCatBuffer.getHeader(&attrCatHeader);

//   for (int i=0;i<relCatHeader.numEntries;i++)
//   {

//     Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

//     relCatBuffer.getRecord(relCatRecord, i);

//     printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

//     for (int j=0;j<attrCatHeader.numEntries;j++)
//     {

//       Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
//       attrCatBuffer.getRecord(attrCatRecord,j);
//       // declare attrCatRecord and load the attribute catalog entry into it

//       if (strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal)==0)
//       {
//         const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";

//         printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
//       }
//     }
//     printf("\n");
//   }

//   return 0;
// }


#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
int main(int argc, char *argv[])
{
    Disk disk_run;
    StaticBuffer buffer;
    OpenRelTable cache;
    RelCacheTable relcachetable;
    AttrCacheTable attrcachetable;
    RelCatEntry relCatentry;
    AttrCatEntry attrcatentry;
    for (int i = 0; i <=2; i++)
    {

      if (relcachetable.getRelCatEntry(i, &relCatentry) != SUCCESS)
      {
        printf("Failed\n");
        continue;
      }
      printf("Relation: %s\n", relCatentry.relName);
      for (int j = 0; j < relCatentry.numAttrs; j++)
      {
        if(attrcachetable.getAttrCatEntry(i,j,&attrcatentry)!=SUCCESS){
          printf("Failed\n");
          continue;
        }

        const char *attrtype = (attrcatentry.attrType == 1) ? "STRING" : "NUM";
        printf(" %s: %s\n", attrcatentry.attrName, attrtype);
      }
  }
  return 0;
}




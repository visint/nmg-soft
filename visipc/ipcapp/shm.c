#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <typedefs.h>
#include <infoset.h>
#include <unit.h>
//gcc -o shm shm.c  -L../../lib -l vispace -I../../include

static  void optProc(int argc, char *const *argv, int opt)
{
    switch (opt) {
    case 'C':
        while (*optarg) {
            switch (*optarg++) {
            case 'q':
                break;

            default:
                fprintf(stderr, "Unknown flag passed to -C: %c\n",
                        optarg[-1]);
                exit(1);
            }
        }
    }
}


int main(int argc, char **argv) 
{
  int n,slot,count;
  char flag;
  if (argc<2)
     return -1;
  if (!strcmp(argv[1],"c"))
     clearShm(0);
  else if (!strcmp(argv[1],"i"))
  {
    status_infoset_t infoset[5];
    flag='y';
    n=0;
    printf("Input slot:");
    setbuf(stdin, NULL);
    scanf("%d",&slot);
    while (flag=='y' && n<5)
    {
     memset(&infoset[n],0,sizeof(status_infoset_t));
     setbuf(stdin, NULL);
     printf("Input infoset_id:");
     scanf("%d",&infoset[n].infoset_id);
     printf("Input  index:");
     setbuf(stdin, NULL);
     scanf("%d",&infoset[n].index);
     printf("Input infoset_size:");
     setbuf(stdin, NULL);
     scanf("%d",&infoset[n].infoset_size);
     printf("Input item_count:");
     setbuf(stdin, NULL);
     scanf("%d",&infoset[n].item_count);
    //infoset[n].infoset_id=3;
    /* infoset[n].infoset_type=1;
     infoset[n].index=0;
     infoset[n].infoset_size=16;
     infoset[n].item_count=11;
     */
     printf("infoset[n].infoset_id=%d Continue?(Y/n):",infoset[n].infoset_id);
     setbuf(stdin, NULL);
     scanf("%c",&flag);
     n++;
    }
    
    insertInfoSet(0,slot,infoset,n);
    /*
    infoset[n].infoset_id=11;
    infoset[n].infoset_type=1;
    infoset[n].index=1;
    infoset[n].infoset_size=16;
    infoset[n].item_count=3;
    */
    //insertInfoSet(0,1,infoset,1);
  }
  else if (!strcmp(argv[1],"s"))
  {
    status_infoset_t *infoset=NULL;
    printf("Input slot:");
    setbuf(stdin, NULL);
    scanf("%d",&slot);
    
    count=getInfoSets(0,slot,&infoset);
    printf("uclass=%d\n",getUnitClass(0,slot));
    for (n=0;n<count;n++)
    {
      if (infoset!=NULL)
      {
        printf("\ncount=%d infoset_id=%d index=%d item_count=%d",count,infoset->infoset_id,infoset->index,infoset->item_count);
      }
      else
        printf("fail\n");
      infoset++;
    }
    printf("\n");
  }
  else if (!strcmp(argv[1],"d"))
  {

  }
}

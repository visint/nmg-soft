#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <unit.h>
#include <infoset.h>
#include <visipc.h>
//gcc -o  /usr/local/bin/uset -lvisipc -lvisdb ../vislib/unit.o ../vislib/visstr.o unitset.c  -I../include
/*
static void usage()
{
  printf("unit info set ver 1.0.1\n");
  printf("Option Specification:\n");
  printf("-b -s[<1..16>] <item_id value>: set unit base info\n");
  printf("-u -s[1..16] <item_id value>: set unit user info\n");
  printf("-t [-c <0..16>] <1..16> [-i <infoset id>] [-n <index> <item_id value>]: set unit infoset item\n");
  printf("-m : set info item\n");
  printf("-a  -s <1..16> <0|1|2|3> <0..100>:set unit status\n");
  printf("-i [-c <1..32>] -s <1..16> -n <infoset_id> set unit info\n");
  printf("-h for help.\n");
  return;
}
*/
static void setBaseInfo(uchar_t chassis,uchar_t slot)
{
 unit_base_info_t bi;
 //if (getBaseInfoFromDB(chassis,slot,&bi)!=TRUE)
 {
    printf("unit not found!\n");
    return;
 }
 printf("property:%s\nsn:%s\nmodel:%s\ncreation:%s\n",bi.property,bi.sn,bi.model,bi.creation);
}

static void setUserData(uchar_t chassis,uchar_t slot)
{
 unit_user_data_t udata;
 if (getUnitClass(chassis,slot)<1)
 {
     printf("Unit #%d not found\n",slot);
     return;
 }

 if (getUserDataFromDB(chassis,slot,&udata)!=TRUE)
 {
    printf("unit not found!\n");
    return;
 }
 printf("name:%s\nslocation:%s\nuserdata:%s\nupdev:%s\n",
     udata.name,udata.location,udata.userdata,udata.updev);
}

static void setUnitInfo(uchar_t chassis,uchar_t slot,uchar_t infoset_id)
{
    status_infoset_t infoset;
    //uchar_t uclass;
    //uclass=getUnitClass(chassis,slot);
    if (getUnitClass(chassis,slot)<1)
    {
       printf("Unit #%d not found\n",slot);
       return;
    }
    /*
    if (*uclass<1)
    {
       printf("Unit #%d not found\n",slot);
       return;
    }
    printf("uclass #%d\n",*uclass);
   */
    //infoset.infoitems[0].item_id=1;
   // infoset.infoitems[1].item_id=2;
    infoset.item_values[0]=1;
    infoset.item_values[1]=2;
    infoset.infoset_id=infoset_id;
    printf("slot=%d\n",slot);
    setStatusInfoSet(0,slot,infoset);
}
static void setInfoSet(uchar_t chassis,uchar_t slot,uchar_t infoset_id)
{
   status_infoset_ex_t *iset;
   int n;
   iset=getMapStatusInfoSet(chassis,slot,infoset_id);
   if (iset==NULL)
   {
     printf("infoset id:%d not found!\n",infoset_id);
     return;
   }
   //printf("uc=%d ut=%d\n",iset->unit_class,iset->unit_type);
   for (n=1;n<3;n++)
   {
      printf("%s:=%s\n",iset->item_name[n],iset->status_name[n]);
   }
}
static void insertUnitClass(uchar_t chassis,uchar_t slot,uchar_t infoset_id)
{

}
static void setUnitStatus(uchar_t chassis,uchar_t slot,uchar_t status)
{
   msgbuf_t msg_buf;
   msg_buf.mtype=1;
   msg_buf.mbuf[0]=0;
   msg_buf.mbuf[1]=slot;
   msg_buf.mbuf[2]='S';
   msg_buf.mbuf[3]=1;
   msg_buf.mbuf[4]=status;
   msg_buf.mbuf[5]=0;
   sendMsgQ(msg_buf,FALSE,0);
   sendSigToCC(SIGUSR2,1);
}
int addInfoset(int chassis,int slot,int infose_id,int infoset_type,int index,int item_count)
{
  status_infoset_t infoset;
  uchar_t *pus;
  char n;
  //char *av[16];
  //char **argv;
  //char *delim=";";
  //char opt_str[]="dbm -i 1=2;2=1 -G -s 1 -u";
  //char opt[16][6];
  

  if (slot<1 || slot>MAX_UNIT_COUNT)
     return -1;
  pus=getUnitStatusMap(0);
  if (*(pus+slot-1)<1)
       *(pus+slot-1)=3;
  infoset.infoset_id=infose_id;
  infoset.infoset_type=infoset_type;
  infoset.item_count=item_count;
  infoset.index=index;
  /*
  for (n=0;n<16;n++)
   av[n]=opt[n];
  
  argv=av;
  infoset.item_count=optsep(item_ids,delim,argv,15);
  */
  printf("infoset_id:%d\n",infoset.infoset_id);
 /* for (n=0;n<infoset.item_count;n++)
  {
    infoset.infoitems[n].item_id=n+1;
    printf("item_id%d:%d\n",n+1,infoset.infoitems[n].item_id);
  }  
*/   
  // insertStatusInfoSet(chassis,slot,infoset);
}

bool_t setByteInfoItem(uchar_t chassis,uchar_t slot,uchar_t infoset_id,uchar_t index,uchar_t itemm_id,uchar_t value,unsigned int useconds)
{
  status_infoset_t *si;
  si=getStatusInfoSetMap(chassis,slot,infoset_id, index);
  if (itemm_id<1 || itemm_id>MAX_ITEM_COUNT || NULL==si)
     return FALSE;
  si->item_values[itemm_id]=value;
  return TRUE;
}

bool_t setByteInfoItems(uchar_t chassis,uchar_t slot,uchar_t infoset_id,uchar_t index,uchar_t itemm_ids[],uchar_t values[],unsigned int useconds)
{
  int n;
  status_infoset_t *si;
  si=getStatusInfoSetMap(chassis,slot,infoset_id, index);
  if (NULL==si)
     return FALSE;
  for (n=0;n<MAX_ITEM_COUNT;n++)
  {
    if (itemm_ids[0]<1 || itemm_ids[0]>MAX_ITEM_COUNT)
      break;
    si->item_values[itemm_ids[n]]=values[n];
  }
  return TRUE;
}

bool_t setByteInfoItemsByStr(uchar_t chassis,uchar_t slot,uchar_t infoset_id,uchar_t index,char *id_values,unsigned int useconds)
{
  char ids[MAX_ITEM_COUNT][6];
  char values[MAX_ITEM_COUNT][6];
  uchar_t item_ids[MAX_ITEM_COUNT];
  uchar_t bytes[MAX_ITEM_COUNT];
  char n,m,count;
  
  
  count=getKeyValue(id_values,"/","=",(char*)ids,6,(char*)values,6,MAX_ITEM_COUNT);
  if (count<1)
     return FALSE;
 
    for (n=0;n<count;n++)
    {
      item_ids[n]=atoi(ids[n]);
      m=atoi(values[n]);
      bytes[n]=m&0xff;
      //printf("%d=%d\n",item_ids[n],bytes[n]);
    }
    item_ids[n]=0;
    return setByteInfoItems(0,slot,infoset_id,index,item_ids,bytes,0);
}

bool_t setShortInfoItem(uchar_t chassis,uchar_t slot,uchar_t infoset_id,uchar_t index,uchar_t itemm_id,short value,unsigned int useconds)
{
  status_infoset_t *si;
  si=getStatusInfoSetMap(chassis,slot,infoset_id, index);
  if (itemm_id<1 || itemm_id>MAX_ITEM_COUNT || NULL==si)
     return FALSE;
  si->item_values[itemm_id*2-1]=value>>8;
  si->item_values[itemm_id*2]=value & 0xff;
  return TRUE;
}

bool_t setShortInfoItems(uchar_t chassis,uchar_t slot,uchar_t infoset_id,uchar_t index,uchar_t itemm_ids[],short values[],unsigned int useconds)
{
  int n;
  status_infoset_t *si;
  //printf("slot=%d is_id=%d index=%d\n",slot,infoset_id,index);
  si=getStatusInfoSetMap(chassis,slot,infoset_id, index);
  if (NULL==si)
  {
   printf("slot=%d is_id=%d index=%d not exist!\n",slot,infoset_id,index);
   return FALSE;
  }
  //printf("here\n");
  for (n=0;n<MAX_ITEM_COUNT;n++)
  {
    if (itemm_ids[n]<1 || itemm_ids[n]>MAX_ITEM_COUNT)
      break;
    si->item_values[itemm_ids[n]*2-1]=values[n]>>8;
    si->item_values[itemm_ids[n]*2]=values[n] & 0xff;
    //printf("%d=%d..\n",itemm_ids[n],values[n]);
  }
  return TRUE;
}

bool_t setShortInfoItemsByStr(uchar_t chassis,uchar_t slot,uchar_t infoset_id,uchar_t index,char *id_values,unsigned int useconds)
{
  char ids[MAX_ITEM_COUNT][6];
  char values[MAX_ITEM_COUNT][6];
  uchar_t item_ids[MAX_ITEM_COUNT];
  short item_values[MAX_ITEM_COUNT];
  char n,m,count;
  
  
  count=getKeyValue(id_values,"/","=",(char*)ids,6,(char*)values,6,MAX_ITEM_COUNT);
  //printf("item count=%d %s\n",count,id_values);
  if (count<1)
     return FALSE;
 
    for (n=0;n<count;n++)
    {
      item_ids[n]=atoi(ids[n]);
      item_values[n]=atoi(values[n]);
    }
    item_ids[n]=0;
    return setShortInfoItems(0,slot,infoset_id,index,item_ids,item_values,0);
}

bool_t setInfoItems(uchar_t chassis,uchar_t slot,uchar_t infoset_id,uchar_t index,char *id_values,uchar_t infoset_type,unsigned int useconds)
{
  if (infoset_type==1)
     return setByteInfoItemsByStr(0,slot,infoset_id,index,id_values,useconds);
  else if (infoset_type==2)
     return setShortInfoItemsByStr(0,slot,infoset_id,index,id_values,useconds);
  else
     return FALSE;
}

static void usage()
{
  fprintf(stderr,"unit set ver 1.0.1\n");
  printf("Option Specification:\n");
  printf("\tOption Specification:\n");
  printf("\t-U [-c <0..31>] -s <1..16> -i itemID -v value : set unit user info\n");
  printf("\t-I [-c <0..31>] -s <1..16> -e infosetID -i itemID1=value;itemID2=value;... [-b|-w] id=value/id=value... : set unit info item\n");
  printf("-h for help.\n");
}

int main(int argc, char **argv)
{
    int oc,chassis=0,slot=0,infoset_id=0,item_id=0,item_value=0;                    
    char opt=0,field_value[32]="",id_values[60];           
    uchar_t item_byte=0;
    bool_t ret;
    char itype=1;

    while((oc = getopt(argc, argv, "ab:c:e:f:i:n:s:tIUvw:")) != -1)
    {
        switch(oc)
        {
            case 'b':
                item_value=atoi(optarg);
                item_byte=0xff&item_value;
                strcpy(id_values,optarg);
                //printf("item value %s.\n",optarg);
                break;
            case 'c':
                chassis=atoi(optarg);
                printf("Chassis is %s.\n",optarg);
                break;
            case 'e':
                infoset_id=atoi(optarg);
                //printf("infoset_id is %s.\n",optarg);
                break;
            case 'v':
                strncpy(field_value, optarg,30);
                printf("Unit item value is %s\n", optarg);
                break;
            case 's':
                slot=atoi(optarg);
                //printf("Unit is #%s\n", optarg);
                break;
            case 'f':
                infoset_id=atoi(optarg);
                //printf("infoset id is %s\n", optarg);
                break;
            case 'i':
                item_id=atoi(optarg);
                //printf("Unit data item id is %s\n", optarg);
                break;
            case 'w':
                itype=2;
                item_value=atoi(optarg);
                strcpy(id_values,optarg);
                //item_byte=0xff&item_value;
                printf("item value %s.\n",optarg);
               
                break;
            case 'U':
                printf("Set user data.\n");
                opt='U';
                break;;
            case 'I':
                //printf("Set info item.\n");
                opt='I';
                break;
            case '?':
                //ec = (char)optopt;
                printf("无效的选项字符 \' %c \'!\n", (char)optopt);
                usage();
                return -1;
            case ':':
                printf("缺少选项参数！\n");
                usage();
                return -1;

        }
    }
   
  switch(opt)
  {
     case 'I':
          //if (FALSE!=setUserDataItemToDB(chassis,slot,item_id,field_value))
          if (infoset_id!=0)
          {
            ret=setInfoItems(0,slot,infoset_id,1,id_values,itype,0);
            /*if (itype==1)
                ret==setByteInfoItem(0,slot,infoset_id,1,item_id,item_byte,0);
            else
                ret==setShortInfoItem(0,slot,infoset_id,1,item_id,item_value,0);
            */
            if  (FALSE!=ret)
                 printf("Set info item OK.slot=%d,infoset_id=%d,item_id=%d,item_byte=%d\n",slot,infoset_id,item_id,item_byte);
            else
                 printf("Set info item fail!\n");
           
            return 0;
          }
          else
              return -1;
     break;
     case 'U':
          if (FALSE!=setUserDataItemToDB(chassis,slot,item_id,field_value))
          {
            printf("Set user data OK.\n");
            return 0;
          }
          else
              return -1;
     break;
     default:
             usage();
             return -1;
  }
   return 0;
}
int _main(int argc, char **argv)
{
   int n;
   uchar_t chassis=0,slot=0;
   uchar_t infoset_id;

   if (argc<2 || !strncmp(argv[1],"-h",2))
   {
      usage();
      return 1;
   }
   printf("pid=%d\n",getpid());
   for (n=1;n<argc;n++)
   {
     if (!strncmp(argv[1],"-i",2) ||!strncmp(argv[1],"-a",2)||!strncmp(argv[1],"-b",2) ||!strncmp(argv[1],"-u",2))
     {
      if (!strncmp(argv[n],"-c",2))
      {
          chassis=atoi(argv[n+1]);
      }
      if (!strncmp(argv[n],"-s",2))
      {
          slot=atoi(argv[n+1]);
      }
      if (!strncmp(argv[n],"-n",2))
      {
          infoset_id=atoi(argv[n+1]);
      }
    }
   }
   //printf("slot=%d\n",slot);
   if (!strncmp(argv[1],"-i",2))
   {
     if (slot>0)
        setUnitInfo(chassis,slot,21);
     else
        usage();
   }
   else if (!strncmp(argv[1],"-a",2))
   {
     if (slot>0 && argc>3)
        setUnitStatus(chassis,slot,atoi(argv[3]));
     else
        usage();
   }
   else if (!strncmp(argv[1],"-b",2))
   {
     if (slot>0)
        setBaseInfo(chassis,slot);
     else
        usage();
   }
   else if (!strncmp(argv[1],"-u",2))
   {
     if (slot>0)
        setUserData(chassis,slot);
     else
        usage();
   }
   return 1;
}

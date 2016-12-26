#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <unit.h>
#include <infoset.h>
#include <visipc.h>
//gcc -o  /usr/local/bin/ushow -lvisipc -lvisdb ../vislib/unit.o ../vislib/visstr.o unitshow.c  -I../include
/*
static void usage()
{
  printf("unit info show ver 1.0.1\n");
  printf("Option Specification:\n");
  printf("-b [<1..16>]: show unit base info\n");
  printf("-u [1..16]: show unit user info\n");
  printf("-t [-c <0..16>] [-s <1..16>] [-i <infoset id>] [-n <index>]: show unit status\n");
  printf("-m : show info item\n");
  printf("-e <1..16>: show unit status\n");
  printf("-i [-c <1..32>] -s <1..16> -n <infoset_id> show unit info\n");
  printf("-a : show all\n");
  printf("-c : create DB\n");
  printf("-h for help.\n");
  return;
}
*/
static void showBaseInfo(uchar_t chassis,uchar_t slot)
{
 unit_base_info_t bi;
 unit_base_info_t *shm_bi;

 if (slot>0 && getUnitClass(chassis,slot)<1)
 {
     printf("Unit #%d not found!\n",slot);
     return;
 }
 shm_bi=getBaseInfoMap(0,slot);
 if (NULL==shm_bi)
 {
    printf("unit #%d base info not found!\n",slot);
    return;
 }
 printf("Unit #%d base info\n",slot);
 printChar("-",25);
 printf("\nProperty:%s\nSN:%s\nModel:%s\nCreation:%s\nHardware ver:%s\nFirmware ver:%s\n",shm_bi->property,shm_bi->sn,shm_bi->model,shm_bi->creation,shm_bi->hwver,shm_bi->fwver);
 printChar("=",25);
 printf("\n");
 /*
 if (getBaseInfoFromDB(chassis,slot,&bi)!=TRUE)
 {
    printf("unit #%d base info not found!\n",slot);
    return;
 }
 printf("Unit #%d base info\n",slot);
 printChar("-",25);
 printf("\nProperty:%s\nSN:%s\nModel:%s\nCreation:%s\nHardware ver:%s\nFirmware ver:%s\n",bi.property,bi.sn,bi.model,bi.creation,bi.hwver,bi.fwver);
 printChar("=",25);
  printf("\n");
 */
}

static void showUserData(uchar_t chassis,uchar_t slot)
{
 unit_user_data_t udata;
 if (getUnitClass(chassis,slot)<1)
 {
     printf("Unit #%d not found\n",slot);
     return;
 }
 if (getUserDataFromDB(chassis,slot,&udata)!=TRUE)
 {
    printf("unit #%d user data not found!\n",slot);
    return;
 }
 printf("Unit #%d user data\n",slot);
 printChar("-",30);
 printf("\nName:%s\nLocation:%s\nUser data:%s\nUp device:%s\n",
     udata.name,udata.location,udata.userdata,udata.updev);
 printChar("=",30);
 printf("\n");
}

static void showUnitInfo(uchar_t chassis,uchar_t slot)
{
    status_infoset_t infoset;
    status_infoset_ex_t iset_ex;
    unit_base_info_t bi;
    unit_user_data_t udata;
    uchar_t count=1,n,m;
    status_infoset_t *pis;
    uchar_t uclass;
    //uchar_t uclass;
    uclass=getUnitClass(chassis,slot);
    if (uclass<1)
    {
       printf("Unit #%d not found\n",slot);
       return;
    }
    if (FALSE!=getBaseInfoFromDB(chassis,slot,&bi))
    {
      printf("Unit #%d base info\n",slot);
      printChar("-",30);
      printf("\n%s:%s\n",getBaseInfoName(UNIT_PROPERTY_ID),bi.property);
      printf("%s:%s\n",getBaseInfoName(UNIT_MODEL_ID),bi.model);
      printf("%s:%s\n",getBaseInfoName(UNIT_SN_ID),bi.sn);
      printf("%s:%s\n",getBaseInfoName(UNIT_CREATION_ID),bi.creation);
      printf("%s:%s\n",getBaseInfoName(UNIT_HW_VER_ID),bi.hwver);
      printf("%s:%s\n",getBaseInfoName(UNIT_FW_VER_ID),bi.fwver);
      printChar("=",30);
      printf("\n");
    }

    if (FALSE!=getUserDataFromDB(chassis,slot,&udata))
    {
      printf("Unit #%d user data\n",slot);
      printChar("-",30);
      printf("\n单元盘名称:%s\n",udata.name);
      printf("单元盘联系:%s\n",udata.contact);
      printf("单元盘位置:%s\n",udata.location);
      printf("用户数据:%s\n",udata.userdata);
      printChar("=",30);
      printf("\n");
    }
    count=getUnitInfoSetsMap(chassis,slot,&pis);
    iset_ex.unit_type=1;
    
    for (n=0;n<count;n++)
    {
      iset_ex.infoset=pis;
      //getUnitClassTyp(chassis,slot,uchar_t *uclass,uchar_t *utype)
      if (TRUE!=getUnitClassType(chassis,slot,&iset_ex.unit_class,&iset_ex.unit_type))
         continue;
      printf("slot=%d unit_class=%d unit_type=%d\n",slot,iset_ex.unit_class,iset_ex.unit_type);
      //iset_ex.unit_type=getUnitType(chassis,slot);
      //iset_ex.infoset.infoset_id=pis->infoset_id;
      //iset_ex.unit_class=unit_class;
      printf("slot=%d infoset id=%d unit_class=%d index=%d item_count=%d\n",slot,iset_ex.infoset->infoset_id,iset_ex.unit_class,pis->index,pis->item_count);
      if (StatusToName(&iset_ex))
      {
        pis++;
        continue;
      }
      printf("Unit #%d %s\n",slot,iset_ex.infoset_name);
      for (m=0;m<pis->item_count;m++)
      {
        printf("%s:%s(item id=%d value=%d)\n",iset_ex.item_name[m],iset_ex.status_name[m],pis->infoitems[m].item_id,pis->infoitems[m].value);
      }

      //printf("slot=%d infoset id=%d index=%d item_count=%d\n",slot,pis->infoset_id,pis->index,pis->item_count);
      /*for (m=0;m<pis->item_count;m++)
         printf("item id=%d value=%d\n",pis->infoitems[m].item_id,pis->infoitems[m].value);
        printf("\n");
      */
       pis++;
    }
    /*
    infoset.infoitems[0].item_id=1;
    infoset.infoitems[1].item_id=2;
    infoset.infoitems[0].value=1;
    infoset.infoitems[1].value=2;
    infoset.infoset_id=infoset_id;
    showStatusInfoSet(0,slot,infoset);
    */
   panelInfo_t info;
   getUnitPanelInfo(chassis,slot,&info);
   printf("Unit #%d (%s)\n",slot,info.unit_name);
   for (n=0;n<info.group_count;n++)
     printf("PType=%d port 1 RX status:%d\n",info.port_group[n].port_type[n],info.port_group[n].port_status[n]);
   if (uclass==3)
   {
     otuInfo_t info;
     getOtuInfo(chassis,slot,&info);
     printf("unit #%d speed_rate=%s distance=%s wave_length=%s\n",slot,info.channel[0].max_speed,info.channel[0].distance,info.channel[0].wave_length);
   }
}
static void showInfoSet(uchar_t chassis,uchar_t slot,uchar_t infoset_id)
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

static void usage()
{
  fprintf(stderr,"unit info show ver 1.0.1\n");
  
  printf("Option Specification:\n");
  printf("-B [-c <0..31>] [-s <1..16>] [-l]: show unit base info\n");
  printf("-U [-c <0..31>] -s <1..16> [-l]: show unit user info\n");
  printf("-I [-c <0..31>] -s <1..16> {-f infosetID [-x <index>]} : show unit status\n");
  printf("-S [-c <0..31>] : show system info\n");
  printf("-h for help.\n");
}

int main(int argc, char **argv)
{
    int oc,chassis=0,slot=0,infoset_id=0,index=0,n;                    
    char opt=0,list=0;           
    
    while((oc = getopt(argc, argv, "BUISc:f:s:x:")) != -1)
    {
        switch(oc)
        {
            case 'c':
                chassis=atoi(optarg);
                //printf("Chassis is %s.\n",optarg);
                break;
            case 'x':
                index=atoi(optarg);
                //printf("Index is %s\n", optarg);
                break;
            case 's':
                slot=atoi(optarg);
                //printf("Unit is #%s\n", optarg);
                break;
            case 'f':
                infoset_id=atoi(optarg);
                //printf("infoset id is %s\n", optarg);
                break;
                break;
            case 'l':
                list=1;
                break;
            case 'I':
                //printf("Show info item.\n");
                opt='I';
                break;
            case 'B':
                //printf("Unit base info.\n");
                opt='B';
                break;
            case 'U':
                //printf("User data.\n");
                opt='U';
                break;
            case 'S':
                //printf("System info.\n");
                opt='I';
                break;
            case '?':
                //ec = (char)optopt;
                printf("无效的选项字符 \' %c \'!\n", (char)optopt);
                usage();
                return 0;
            case ':':
                printf("缺少选项参数！\n");
                usage();
                return 0;

        }
    }
   
  switch(opt)
  {
     case 'I':
          //if (FALSE!=setUserDataItemToDB(chassis,slot,item_id,field_value))
            //  printf("Show unit #%d info item OK.\n",slot);
           if (infoset_id!=0)
              showUnitInfo(chassis,slot);
           else
              showAllInfoset();
     break;
     case 'B':
          //if (FALSE!=setUserDataItemToDB(chassis,slot,item_id,field_value))
              //printf("Show unit #%d user item OK.\n",slot);
              if (slot>0)
                 showBaseInfo(chassis,slot);
              else
              {
                for (n=1;n<MAX_UNIT_COUNT;n++)
                    showBaseInfo(chassis,n);
              }
     break;
     case 'U':
             if (slot>0)
                 showUserData(chassis,slot);
              else
              {
                for (n=1;n<MAX_UNIT_COUNT;n++)
                    showUserData(chassis,n);
              }
     break;
     case 'S':
          //if (FALSE!=setUserDataItemToDB(chassis,slot,item_id,field_value))
              printf("System info.\n");
     break;
     default:
             usage();
             return 0;
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
   for (n=1;n<argc;n++)
   {
     if (!strncmp(argv[1],"-i",2) ||!strncmp(argv[1],"-e",2)||!strncmp(argv[1],"-b",2) ||!strncmp(argv[1],"-u",2))
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
     printf("slot=%d\n",slot);
     if (slot>0)
        showUnitInfo(chassis,slot);
     else
        usage();
   }
   else if (!strncmp(argv[1],"-e",2))
   {
     if (slot>0)
        showInfoSet(chassis,slot,infoset_id);
     else
        usage();
   }
  else if (!strncmp(argv[1],"-b",2))
   {
     if (slot>0)
        showBaseInfo(chassis,slot);
     else
        usage();
   }
   else if (!strncmp(argv[1],"-u",2))
   {
     if (slot>0)
        showUserData(chassis,slot);
     else
        usage();
   }
   return 1;
}

/******************************** Description *********************************
* 文件名称： visMbusMap.c.c
* 功能描述： 
*1、信息集共享内存的读写、添加、删除操作
*2、0-15字节存放1－16单元盘的大类、16－31字节存放1－16单元盘的小类
*4、32－63字节存命令报文
*5、64－95字节存数据报文
*6、96－111字节存放单元盘信息集在内存中的节点号位置
*7、120开始存放信息集
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/10/18        V1.0      李文峰        创建
*
* 编译命令:
*应用软件编译:gcc -o mbs  ../vislib/visstr.o mbusshm.c -I../include
*SO编译:gcc -o ish  ../vislib/visstr.o visMbusMap.c -I../include
gcc -c -o ../vislib/mbusshm.o mbusshm.c -I../include
gcc -shared -fpic -o libmsgq.so vismsg.c  -I../include
*Copyright (c) Visint Inc., 2002-2012. All Rights Reserved.
*******************************************************************************/
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
//存储池 
#define  INFO_SET_MAX_COUNT 80 
#define  STATUS_MBUS_SHM_KEY 75
#define  BASE_INFO_MBUS_SHM_KEY 77
#define  INFO_HEAD_SIZE 128
#define  CMD_MESSAGE_OFFSET 32  //命令报文
#define  DATA_MESSAGE_OFFSET 64 //数据报文
#define  INFO_SET_OFFSET 96 /*存放下一个Infoset的节点号*/

static int   usi_shm_id = -1;
static char *usi_shm_add=NULL;

//static int   usi_shm_id = -1;
//static char *usi_shm_add=NULL;

static bool_t initMbusShmget(uchar_t chassis) 
{ 
    char n;
    usi_shm_id = shmget(STATUS_MBUS_SHM_KEY+chassis,INFO_HEAD_SIZE+(sizeof(status_infoset_t) +sizeof(unit_base_info_t))*MAX_UNIT_COUNT, IPC_CREAT | 0666);
    
    if(usi_shm_id==-1)
    {
        perror("MbusMapget error");
        return FALSE;
    }
    usi_shm_add=(char*)shmat(usi_shm_id,NULL,0);
    if(usi_shm_add == (void *) -1)
    { 
      perror("MbusMapat");
      printf("MbusMap the MbusMap error: %s.\n", strerror(errno));
      return FALSE; 
    }
    //system("ipcs -m");
    return TRUE;
} 
/*
uchar_t *getUnitStatusMbusMap(uchar_t chassis,uchar_t slot)
{
  if (slot<1 || slot>MAX_UNIT_COUNT)
     return NULL;
  if(usi_shm_id==-1)
  {
     if (TRUE!=initMbusShmget(chassis))
       return NULL;
  }
  uchar_t *status;
  status=(uchar_t*)(usi_shm_add+INFO_HEAD_SIZE);
  return status;
}
*/
unit_base_info_t *getBaseInfoMbusMap(uchar_t chassis,uchar_t slot)
{
  if (slot<1 || slot>MAX_UNIT_COUNT)
     return NULL;
  if(usi_shm_id==-1)
  {
     if (TRUE!=initMbusShmget(chassis))
       return NULL;
  }
  unit_base_info_t *bi;
  bi=(unit_base_info_t*)(usi_shm_add+INFO_HEAD_SIZE+sizeof(status_infoset_t)*MAX_UNIT_COUNT+sizeof(unit_base_info_t)*(slot-1));
  return bi;
}

/*********************************************************************************
* 函数名称： getStatusInfoSetMbusMap
* 功能描述： 获取共享内存中的信息集。 
* 访问的表： 无
* 修改的表： 无
* 输入参数： uchar_t chassis,uchar_t slot,uchar_t infoset_id,uchar_t index
* 输出参数： 无
* 返 回 值： 成功返回status_infoset_t *,错误返回NULL
* 其它说明： 除了CC，其它进程不要随便修改返回的信息集的内容
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/10/18        V1.0      李文峰        创建
************************************************************************************/
status_infoset_t *getInfoSetMbusMap(uchar_t chassis,uchar_t slot,uchar_t infoset_id,uchar_t index)
{
   status_infoset_t *sis;
   char n;
   uchar_t  begin,*from_node,*to_node;
   
   if (slot<1 || slot>MAX_UNIT_COUNT)
     return NULL;
    if(usi_shm_id==-1)
    {
      if (TRUE!=initMbusShmget(chassis))
       return NULL;
    }
    sis=(status_infoset_t *)(usi_shm_add+INFO_HEAD_SIZE+(slot-1)*sizeof(status_infoset_t));  
    return sis;
}

 
bool_t getStatusInfoSetMbusMap(uchar_t chassis,uchar_t slot,status_infoset_t *infoset)
{
   status_infoset_t *sis;
   sis=getInfoSetMbusMap(chassis,slot,infoset->infoset_id,infoset->index);
   if (NULL==sis)
    return FALSE;
   memcpy(infoset,sis,sizeof(status_infoset_t));
   return TRUE;
}

void showInfoSetMbusMap(uchar_t chassis,uchar_t slot,uchar_t infoset_id,uchar_t index)
{
  status_infoset_t infoset;
  char n,m;
  infoset.infoset_id=infoset_id;
  infoset.index=index;
  if (TRUE!=getStatusInfoSetMbusMap(chassis,slot,&infoset))
  {
    printf("slot=%d infoset_id=%d not found\n",slot,infoset_id);
    return;
  }
  /*
  for (m=0;m<infoset.item_count;m++)
  {
    printf("item id[%d]=%d  value=%d ",m,infoset.infoitems[m].item_id,infoset.infoitems[m].value);
  }
  printf("\n");
  */
}

bool_t setStatusInfoSetMbusMap(uchar_t chassis,uchar_t slot,status_infoset_t infoset)
{
  status_infoset_t *sis;
  sis=getInfoSetMbusMap(chassis,slot,infoset.infoset_id,infoset.index);
  if (NULL!=sis)
  {
    memcpy(sis,&infoset,sizeof(status_infoset_t));
    return TRUE;
  }
  printf("slot=%d infoset_id=%d not found\n",slot,infoset.infoset_id);
  return FALSE;
}

void removeMbusMapget(uchar_t chassis)
{
   int MbusMapid;
   MbusMapid = shmget(STATUS_MBUS_SHM_KEY+chassis,INFO_HEAD_SIZE+sizeof(status_infoset_t)*INFO_SET_MAX_COUNT, IPC_CREAT | 0666);
  //删除共享内存段  
  if(shmctl(MbusMapid,IPC_RMID,NULL)<0)
     perror("error");

}

/**********************************************************************
* 函数名称： GetUnitStstusFromMbusMap
* 功能描述： 读取机箱内1-16单元盘的在位状态及类型 
* 访问的表： 无
* 修改的表： 无
* 输入参数： uchar_t chassis：机箱号
* 输出参数： 无
* 返 回 值： 1-16个单元盘的存在状态位,对应的字节为0－15，为非0为单元盘的大类、则存在，否则不存在；16－31字节为单元盘的小类
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/10/19        V1.0      李文峰        创建
* 
***********************************************************************/
uchar_t *getUnitStatusMbusMap(uchar_t chassis)
{
   if (chassis>31)
     return NULL;
   if(usi_shm_id==-1)
    {
      if (TRUE!=initMbusShmget(chassis))
       return NULL;
    }
   // return (uchar_t *)(usi_shm_add+INFO_HEAD_SIZE);
    return (uchar_t *)(usi_shm_add);
}



void setUnitStatusToMbusMap(uchar_t chassis,uchar_t slot,uchar_t uclass,uchar_t utype)
{
   if (slot<1 || slot>16)
     return;
   if(usi_shm_id==-1)
   {
     if (TRUE!=initMbusShmget(chassis))
       return;
   }
   uchar_t *uc;
   //uc=(uchar_t *)(usi_shm_add+INFO_HEAD_SIZE +slot-1);
   uc=(uchar_t *)(usi_shm_add+slot-1);
   //ut=(uchar_t *)(usi_shm_add+INFO_HEAD_SIZE +15+slot);
   *uc=uclass;
   *(uc+16)=utype;
}

static uchar_t *getMsgFromMbusMap(uchar_t chassis,uchar_t msgflag)
{
   if (chassis>31 || msgflag>1)
     return NULL;
   if(usi_shm_id==-1)
    {
      if (TRUE!=initMbusShmget(chassis))
       return NULL;
    }
    if (msgflag)
      return (uchar_t *)(usi_shm_add+CMD_MESSAGE_OFFSET);
    else
      return (uchar_t *)(usi_shm_add+DATA_MESSAGE_OFFSET);
}
static bool_t setMsgToMbusMap(uchar_t chassis,uchar_t msgflag,uchar_t *message)
{
   if (chassis>31 || msgflag>1)
     return FALSE;
   if(usi_shm_id==-1)
   {
     if (TRUE!=initMbusShmget(chassis))
       return FALSE;
   }
   uchar_t *msg;
   if (msgflag)
      msg=(uchar_t *)(usi_shm_add+CMD_MESSAGE_OFFSET);
   else
      msg=(uchar_t *)(usi_shm_add+DATA_MESSAGE_OFFSET);
   memcpy(msg,message,32);
   return TRUE;
}
uchar_t *getCmdMsgFromMbusMap(uchar_t chassis)
{
   return getMsgFromMbusMap(chassis,1);
}
bool_t setCmdMsgToMbusMap(uchar_t chassis, uchar_t *message)
{
  return setMsgToMbusMap(chassis,1,message);
}

uchar_t *getDataMsgFromMbusMap(uchar_t chassis)
{
   return getMsgFromMbusMap(chassis,0);
}
bool_t setDataMsgToMbusMap(uchar_t chassis, uchar_t *message)
{
   return setMsgToMbusMap(chassis,0,message);
}


static initUnits()
{
  int n;
  time_t t;
  char uc[][5]={"UKW","MC","OLP","OTU"};
  char uclass;
  unit_base_info_t *pbi,bi;
  uchar_t *pus;
  srand((unsigned) time(&t));//srand函数是随机数发生器的初始化函数，不用的话每次产生同样的随机数
  pus=getUnitStatusMbusMap(0);
  for (n=0;n<MAX_UNIT_COUNT;n++)
  {
    uclass=rand()%4;
    if (uclass<2)
       uclass=0;
    sprintf(bi.property,"%s1-XX-XX-XX-%d",uc[uclass],n+1);
    sprintf(bi.sn,"SN-XX-XX-XX-%d",n+1);
    sprintf(bi.model,"Model-XX-XX-XX-%d",n+1);
    if (uclass>0)
    {
       pbi=getBaseInfoMbusMap(0,n+1);
       
       if (pus!=NULL)
           *(pus+n)=uclass;
       if (pbi!=NULL)
          memcpy(pbi,&bi,sizeof(unit_base_info_t));
       else
          memset(pbi,0,sizeof(unit_base_info_t));
    }
  }
}
int main(int argc, char **argv) 
{  
    status_infoset_t infoset;
    int slot=0;
    uchar_t n;
    if (argc<2)
      return 0;
    memset(&infoset,0,sizeof(status_infoset_t));
    
    if (!strncmp(argv[1],"-i",2))
    {
      if (argc>5)
      {
        slot=atoi(argv[2]);
        infoset.infoset_id=atoi(argv[3]);
        infoset.index=atoi(argv[4]);
       // for (n=5;n<argc;n++)
        //  infoset.infoitems[n-5].item_id=atoi(argv[n]);
        infoset.item_count=argc-5;
        //printf("infoset.item_count:%d\n",infoset.item_count);
        //insertStatusInfoSet(0,slot,infoset); 
      }
    }
    else if (!strncmp(argv[1],"-d",2))
    {
       if (argc>4)
       {
        slot=atoi(argv[2]);
        showInfoSetMbusMap(0,slot,atoi(argv[3]),atoi(argv[4]));
       }
    }
    else if (!strncmp(argv[1],"-s",2))
    {
       if (argc>5)
       {
        slot=atoi(argv[2]);
        infoset.infoset_id=atoi(argv[3]);
        infoset.index=atoi(argv[4]);
        if (TRUE!=getStatusInfoSetMbusMap(0,slot,&infoset))
           return 1;
        for (n=5;n<argc;n++)
          infoset.item_values[n-5]=atoi(argv[n]);
        if (setStatusInfoSetMbusMap(0,slot,infoset)!=FALSE)
          printf("OK\n");
       }
    }
    else if (!strncmp(argv[1],"-r",2))
    {
       slot=atoi(argv[2]);
       //deleteSlotNodes(0,slot);
    }
    else if (!strncmp(argv[1],"-c",2))
    {
       initUnits();
    }
    else if (!strncmp(argv[1],"-t",2))
    {
       if (argc>4)
       {
         slot=atoi(argv[2]);
         setUnitStatusToMbusMap(0,slot,atoi(argv[3]),atoi(argv[4]));
       }
    }
    else if (!strncmp(argv[1],"-y",2))
    {
       uchar_t uc,ut,uclass;
       unit_base_info_t *bi;
       uchar_t *unit_status;

       if (argc>2)
       {
         slot=atoi(argv[2]);
         //getUnitClassType(0,slot,&uc,&ut);
         //uclass=getUnitClass(0,slot);
         printf("slot=%d uc=%d ut=%d uclass=%d\n",slot,uc,ut,uclass);
         if (uc>0)
         {
           bi=getBaseInfoMbusMap(0,slot);
           if (bi!=NULL)
             printf("property=%s\n",bi->property);
         }  
       }
       else
       {
          int n;
          
          unit_status=getUnitStatusMbusMap(0);
          if (unit_status==NULL)
             return 0;
          for (n=0;n<MAX_UNIT_COUNT;n++)
          {
            //getUnitClassType(0,n+1,&uc,&ut);
            //uclass=getUnitClass(0,n+1);
            uc=*(unit_status+n);
            if (uc>0)
            {
             // printf("slot=%d uc=%d ut=%d uclass=%d\n",n+1,uc,ut,uclass);
              bi=getBaseInfoMbusMap(0,n+1);
              if (bi!=NULL)
              {
                printf("#%d property=%s uclass=%d\n",n+1,bi->property,uc);
              }
            }  
          }
       }
    }
    else if (!strncmp(argv[1],"-b",2))
    {
       if (argc>2)
       {
         uchar_t uc,ut,uclass;
         slot=atoi(argv[2]);
         //getUnitClassType(0,slot,&uc,&ut);
         //uclass=getUnitClass(0,slot);
         printf("slot=%d uc=%d ut=%d uclass=%d\n",slot,uc,ut,uclass);    
       }
    }
    return 1;
} 

/******************************** Description *********************************
* 文件名称： callcenter.c
* 功能描述： 
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/9/23        V1.0      李文峰        创建
*
* 编译命令:gcc -o /usr/local/bin/ccenter callcenter.c -I../include -L=./
*
*Copyright (c) Visint Inc., 2002-2012. All Rights Reserved.
*******************************************************************************/

/*
#include "../port/port.h"
#include "../port/oport.h"
#include "../port/sfp.h"
#include "otu.h"
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <bits/siginfo.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <sqlite3.h>

#include <public.h>
#include <visipc.h>

#include <typedefs.h>
#include <chassis.h>
#include <unit.h>
#include <infoset.h>
#include <visdb.h>
#include <chassis.h>

#define TIMEOUT 3
#define NORMAL_PERIOD		3		/* second */
#define RUSHURGENT_PERIOD	50 * 1000	/* microsecond */

#define DEBUG_CC 1
#define MAX_REQ_COUNT 5

struct itimerval itv;
static char req_type=0;
static char msg_key[MAX_REQ_COUNT];
static int msgqid;

static char unit_class[MAX_CHASSIS_COUNT][MAX_UNIT_COUNT+1];
static char last_unit_class[MAX_CHASSIS_COUNT][MAX_UNIT_COUNT+1];
static char old_unit_class[MAX_CHASSIS_COUNT][MAX_UNIT_COUNT+1];

static void detectUnitInfoset(uchar_t chassis,uchar_t slot);
int getInfosetFromModBus(uchar_t chassis,uchar_t slot,uchar_t *message);

message_set_t message_set[MAX_UNIT_COUNT+1];

const char *binfo_name[MAX_BASE_INFO_COUNT+1]={
"property",
"harware ver",
"firmware ver",
"model",
"sn",
"creation",
"uptime",
};


static int onFetchInfoset( void *para, int n_column, char **column_value, char **column_name)
{
  int n;
  for (n=0;n<n_column;n++)
    printf("%s:%s\n",column_name[n],column_value[n]);
  return 0;
}
static int loadInfoset(uchar_t chassis,uchar_t slot,uchar_t uclass,uchar_t utype)
{
  sqlite3 *db=NULL;
  char *zErrMsg = 0;
  int rc;

  char sql[100];
  rc = sqlite3_open(DB_PATH, &db);
  //printf("uclass=%d,utype=%d\n",uclass,utype);
  if( rc )
  {
   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
   sqlite3_close(db);
   return;
  }
  sprintf(sql,"SELECT * FROM InfoSet WHERE include_class like '%c%d;%c' AND exclude_type NOT LIKE '%c%d:%d;%c';",'%',uclass,'%','%',uclass,utype,'%');
  //printf("sql:%s\n",sql);
  rc = sqlite3_exec(db, sql,  onFetchInfoset, NULL, &zErrMsg); 
  //printf("zErrMsg:%s\n",zErrMsg);
  sqlite3_close(db);
}

static void detectUnitStatus(uchar_t chassis)
{
   uchar_t  n;
   uchar_t *us_mbus,*us_shm;
   unit_base_info_t *mbus_bi,*shm_bi;
   uchar_t unit_class,unit_type;
   bool_t isvalid;

   us_mbus=getUnitStatusMbusMap(0);
   us_shm=getUnitStatusMap(0);
 
   if (us_mbus==NULL || us_shm==NULL)
   {
      printf("us_mbus==NULL || us_shm==NULL\n");
      return;
   }
   for (n=1;n<MAX_UNIT_COUNT;n++)
   {
      if (*(us_mbus+n)!=*(us_shm+n))
      {
          if (*(us_shm+n)==0)
          {
             if (*(us_mbus+n)!=127 && *(us_mbus+n)!=2)
             {
                shm_bi=getBaseInfoMap(0,n);
                mbus_bi=getBaseInfoMbusMap(0,n);
                if (mbus_bi!=NULL && shm_bi!=NULL)
                { 
                   memcpy(shm_bi,mbus_bi,sizeof(unit_base_info_t));
                   
                  // printf("loadUnitInfoset #%d property=%s\t%s\n",n,mbus_bi->property,shm_bi->property);
                   isvalid =checkUnitClassType(*mbus_bi,&unit_class,&unit_type);
                   if (isvalid !=FALSE)
                   {
                    //message_set[n].message_count=22;
                   //if (n==5)
                   // loadInfoset(0,n+1,unit_class,unit_type);
                    //isvalid=1
                    loadUnitInfoset(0,n,*shm_bi,&message_set[n]);
                   }
                  
                  /* else
                   {
                     printf("Invalid unit %d Inserted\n",n+1);
                     //continue;
                   }
                   */ 
                }
                if (mbus_bi!=NULL)
                {
                    if (isvalid !=FALSE)
                    {
                        printf("Unit %d (%s) Inserted UC=%d\n",n,mbus_bi->property,*(us_mbus+n));
                        //*(us_shm+n)=unit_class;
                        //insertAlarmLog(12,0,n,0);
                        sendSnmpTrap(52,0,n,0);
                        
                    }
                    else
                    {
                       printf("Invalid unit %d(%s uc=%d) Inserted!\n",n,mbus_bi->property,*(us_mbus+n));
                       *(us_mbus+n)=0;
                       //*(us_shm+n)=*(us_mbus+n);
                    }
                }
                else
                {
                     printf("Unit %d Inserted\n",n);
                    // *(us_shm+n)=unit_class;
                }
             }
             else
             {
                 printf("Unkown unit %d Inserted\n",n);
                 //*(us_shm+n)=127;
             }
         }
         else
         {
             printf("Unit %d Removed\n",n);
             //*(us_shm+n)=0;
             //insertAlarmLog(11,0,n,0);
             sendSnmpTrap(51,0,n,0);
         }

       *(us_shm+n)=*(us_mbus+n);
      }
     else if (isvalid!=FALSE) //if(*(us_shm+n)>0 && *(us_shm+n)!=127)
     {
       if (n<MAX_UNIT_COUNT)
       detectUnitInfoset(0,n);
     }
   }
}

static void detectUnitInfoset(uchar_t chassis,uchar_t slot)
{
  uchar_t n,m,k;
  uchar_t message[32];

  //status_infoset_t *si,*iset_shm;
  if (slot<0 || slot>=MAX_UNIT_COUNT)
      return;
  //si=getInfoSetMbusMap(0,n+1,0,0);
  //printf("message_set[slot-1].message_count=%d\n",message_set[slot-1].message_count);
  for (n=0;n<message_set[slot].message_count;n++)
  {
     if (message_set[slot].message[n].infoset_type==1)
        message[0]='G';
     else
        message[0]='B';
     k=1;
     for (m=0;m<message_set[slot].message[n].infoset_count;m++)
     {
       message[k++]=message_set[slot].message[n].infoset[m].infoset_id;
       message[k++]=message_set[slot].message[n].infoset[m].index;
       message[k++]=127;
     }
     //printf("slot=%d m0=%c m1=%d m2=%d m3=%d m4=%d m5=%d\n",slot,message[0],message[1],message[2],message[3],message[4],message[5]);
    getInfosetFromModBus(0,slot,message);
  }
/*
  iset_shm=getStatusInfoSetMap(0,n+1,21,1);
  if (si!=NULL && iset_shm!=NULL)
  {
    for (n=0;n<MAX_ITEM_COUNT;n++)
    {
      if (iset_shm->infoitems[n].value!=si->infoitems[n].value)
          iset_shm->infoitems[n].value=si->infoitems[n].value;
    }
  }
*/
}

static void detectUnitBaseInfo(uchar_t chassis,uchar_t slot)
{
  int val=slot;
  val<<=16;
  val|=0x0200;
  //printf("before detectUnitBaseInfo slot=%d\n",slot);
  sendSigToModBus(SIGUSR1,val);
  //printf("after detectUnitBaseInfo\n");
}
static bool_t getMsgFromUnit(char *mess,uchar_t size,ushort_t timeout)
{
  char unit=mess[1];
  char buf[32];
 //if (unit_class[mess[0]][mess[1]]<1)
  if (unit_class[0][unit]!=1) 
     return FALSE;
  mess[0]=mess[3];
  mess[1]=mess[4];
  mess[2]=mess[5];
  mess[3]='\0';
  #ifdef DEBUG_CC
  printf("inforset id=%d item_id=%d:",mess[0],mess[2]); 
  switch (mess[0])
  {
    	case UNIT_BASE_INFO_SET_ID:
    		   if (mess[2]<0 ||mess[2]>7)
    		   	 return FALSE;
    		   printf("%s:",binfo_name[mess[2]-1]);
     		   scanf("%s",buf);
     		   strcat(mess,buf);
     	     break;
     	case UNIT_STATUS_INFO_SET_ID:
     	     break;
  }
  #endif
  //strcpy(mess,unit_property[unit]);
  return TRUE;
}
static bool_t getBaseInfoFromUnit(uchar_t chassis,uchar_t slot,uchar_t item_id,char mess[],ushort_t timeout)
{
  bool_t ret;
  if (item_id>MAX_BASE_INFO_COUNT)
    return FALSE;
  mess[0]=chassis;
  mess[1]=slot;
  mess[2]='G';
  mess[3]=UNIT_BASE_INFO_SET_ID;
  mess[4]=1;
  mess[5]=item_id;
  mess[6]=0;
  ret= getMsgFromUnit(mess,6,0);
  if(ret!=FALSE)
  {
    strcpy(mess,mess+3);
  }
  return ret; 
}
static int getBaseInfoFromModBus(uchar_t chassis,uchar_t slot,unit_base_info_t *bi)
{
  int result=-1;
  ubibuf_t ubibuf;
  //return -1;
  if (slot<0 || slot>=MAX_UNIT_COUNT)
    return -1;
  ubibuf.mtype=100+slot;
  result=msgrcv(msgqid,&ubibuf, MAX_IPC_UBI_BUF,0,IPC_NOWAIT);
  if (result!=-1)
  {
     //memcpy(bi,buf.mbuf,MAX_IPC_UBI_BUF);
     printf("buf.mtype=%d slot=%d OK\n",ubibuf.mtype,slot);
   }
  else
  {
    perror("getBaseInfoFromModBuserror");
    printf("buf.mtype=%d slot=%d msgqid=%d\n",ubibuf.mtype,slot,msgqid);
  }
  return result;
}
static void insertUnit(char chassis,char slot)
{
 int n;
 char mess[MAX_ITEM_SIZE]={0,1,UNIT_BASE_INFO_SET_ID,1,1,0,0,0};
 unit_base_info_t *ubi;
 
 if (slot<1 || slot>=MAX_UNIT_COUNT)
   return;
 //ubi=GetBISetFromNvram(chassis,slot);
 
 //if (getBaseInfoFromDB(chassis,slot,&ubi)!=FALSE)
 //  return;
 printf("Unit #%d pugined SN:%s\n",slot,ubi->sn); 
 unit_class[0][slot]=1;
 for (n=1;n<MAX_BASE_INFO_COUNT;n++)
 {
    if (getBaseInfoFromUnit(chassis,slot,n,mess,TIMEOUT)!=TRUE)
     return;
    printf("Item id #%d messs:%s\n",n,mess);
   switch (n)
   {
   	 case UNIT_SN_ID:
   	 	if (strcmp(mess+3,ubi->sn))
        strncpy(ubi->sn,mess,15);
   	 	break;
   	 case UNIT_PROPERTY_ID:
   	 	  strncpy(ubi->property,mess,MAX_ITEM_SIZE-1);
   	 	break;
   	 case UNIT_HW_VER_ID:
   	 	    strncpy(ubi->hwver,mess,11);
   	 	break;
   	 case UNIT_FW_VER_ID:
   	 	    strncpy(ubi->fwver,mess,11);
   	 	break;
   	 case UNIT_MODEL_ID:
   	 	  strncpy(ubi->model,mess,MAX_ITEM_SIZE-1);
   	 	break;
   	 case UNIT_CREATION_ID:
   	 	  strncpy(ubi->creation,mess,11);
   	 	break;
   	 case UNIT_UPTIME_ID:
   	 	  strncpy(ubi->uptime,mess,11);
   	 	break;
   }
} 
 return; 
 if (getBaseInfoFromUnit(chassis,slot,UNIT_SN_ID,mess,TIMEOUT)!=TRUE)
  return;
 printf("Unit #%d pugined SN:in NVRAM %s --%s\n",slot,ubi->sn,mess); 
 if (!strcmp(mess,ubi->sn))
  return;
 //ubi->sn[0]='\0'; 
 strncpy(ubi->sn,mess,15);
 ubi->sn[16]='\0';
 //printf("Unit #%d pugined SN:%s --%s\n",slot,ubi->sn,mess);
 if (getBaseInfoFromUnit(chassis,slot,UNIT_MODEL_ID,mess,TIMEOUT)!=TRUE)
  return;
 if (!strcmp(mess,ubi->model))
  return;
 strcpy(ubi->model,mess);
 if (getBaseInfoFromUnit(chassis,slot,UNIT_PROPERTY_ID,mess,TIMEOUT)!=TRUE)
  return;
 if (!strcmp(mess,ubi->property))
 {
  SetItemToNvram(chassis,slot,UNIT_BASE_INFO_SET_ID,1,UNIT_PROPERTY_ID,mess);
 }
}
static void removeUnit(uchar_t chassis,uchar_t slot)
{
 uchar_t *us;
 if (slot<1 || slot>MAX_UNIT_COUNT)
   return;
 //us=GetUnitStstusFromNvram(chassis);
 us=getUnitStatusMap(chassis);
 if (us==NULL)
 	  return;
 us[slot]=0;
 printf("remove ok\n");
}
static int getInfosetMsg(msgbuf_t *buf)
{
  int result;
  result=msgrcv(msgqid,buf,MAX_IPC_MSG_BUF,0,IPC_NOWAIT);
 /* if (result==-1)
  {
      perror("msgrcv error");
  }
*/
  return result;
}
static void detectChassis(uchar_t chassis)
{
  int n,m,status;
  chassis_t *cinfo;
  time_t t;
  srand((unsigned) time(&t));
  
  cinfo=getChassisInfo(0);
  if (cinfo!=NULL)
  {
   m=rand()%2;
   status=rand()%2+1;
   cinfo->powersupply[0].status=status;
   cinfo->powersupply[1].type=rand()%2+1;
   cinfo->powersupply[0].volt=58;
   cinfo->powersupply[1].volt=55;
   if (status!=1)
     sendSnmpTrap(1,0,0,m+1);
   m=rand()%4;
   status=rand()%2+1;
   cinfo->fan[m].status=status;
   if (status!=1)
     sendSnmpTrap(3,0,0,m+1);
   sendSnmpTrap(5,0,0,0);
   sendSnmpTrap(7,0,0,1);
   printf("detectChassis..\n");
   //exit(0);
  }
}
static void getUnitStatus(uchar_t chassis)
{
  //char us[MAX_UNIT_COUNT+1]={1,1,1,1,1,0,1,0,1,0,2,1,3,1,1};
  int n;
  msgbuf_t buf;
  char changed[MAX_UNIT_COUNT]={0};
  bool_t flag=FALSE;

  buf.mtype=1;
  if (getInfosetMsg(&buf)==-1)
    return;
  if (!memcmp(buf.mbuf,unit_class[0],MAX_UNIT_COUNT))
    return;

  uchar_t *unit_status;
  unit_base_info_t *bi;
  bi=getBaseInfoMap(0,1);
  unit_status=getUnitStatusMap(0);
  if (unit_status==NULL || bi==NULL)
    return;
  for (n=1;n<MAX_UNIT_COUNT;n++)
  {
    if (unit_class[0][n]!=buf.mbuf[n])
    {
      if (buf.mbuf[n]>0)
      {
         bi=getBaseInfoMap(0,n);
         if (bi==NULL)
           continue;
         if (!strncmp(bi->property,"OTU",3))
               unit_class[0][n]=UC_OTU;
         else if (!strncmp(bi->property,"OLP",3))
                 unit_class[0][n]=UC_OLP;
         else 
               unit_class[0][n]=0;
         if (unit_class[0][n]>0)
             printf("Unit #%d (%s) Inserted UC=%d\n",n,bi->property,unit_class[0][n]);
         else
            printf("Unkown unit #%d(%s) Inserted\n",n,"bi->property");
      }
      else
      {
         printf("Unit #%d Removed\n",n);
      }
      *(unit_status+n)=unit_class[0][n];
      unit_class[0][n]=buf.mbuf[n];
    }
  }
  return;
  for (n=0;n<MAX_UNIT_COUNT;n++)
  {
    //if (unit_class[0][n]!=buf.mbuf[n])
    //   detectUnitBaseInfo(0,n+1);
    //unit_class[0][n]=us[n];
    //insertUnit(chassis,n);
    if (unit_class[0][n]!=buf.mbuf[n])
    {
      if (flag!=FALSE)
         flag=TRUE;
      if (buf.mbuf[n]>0)
      {
        if (buf.mbuf[n]>0)
        {
           if (bi==NULL)
             continue;
           if (!strncmp(bi->property,"OTU",3))
                 unit_class[0][n]=3;
            else if (!strncmp(bi->property,"OLP",3))
                 unit_class[0][n]=2;
            if (unit_class[0][n]>0)
             //   changed[n]=1;
                printf("Unit #%d (%s) Inserted\n",n+1,bi->property);
            else
                printf("Unkown Unit #%d (%s) Inserted\n",n+1,bi->property);
           //detectUnitBaseInfo(0,n+1);
           //usleep(20);
        }
      }
      else if (unit_class[0][n]>0)
      {
         unit_class[0][n]=0;
         printf("Unit #%d Removed..\n",n+1);
         changed[n]=2;
      }  
    }
  }
  if (flag!=FALSE)
  {
    uchar_t *unit_status;
    unit_base_info_t *bi;
    bi=getBaseInfoMap(0,n+1);
    unit_status=getUnitStatusMap(0);
    if (unit_status!=NULL && bi!=NULL)
    {
        for (n=0;n<MAX_UNIT_COUNT;n++)
        {
           if (changed[n]==0)
           {
               continue;
           }
           else if (changed[n]>0)
           {
               //unit_status[n]=unit_class[n];
           }
           else
           {
              unit_status[n]=0;
           }
        }
        printf("setUnitStatusToShm OK\n");
    }
    else
      printf("getUnitStatusMap error\n");
  }
  //SetUnitStstusToNvram(chassis,unit_status);
}
static void getUnitBaseInfo(uchar_t chassis,uchar_t slot)
{
  int n;
  //msgbuf_t buf;
  int unit;
  msgbuf_t buf;
  unit_base_info_t *bi,ubi;

  printf("getUnitBaseInfo\n");
  for (n=1;n<=MAX_UNIT_COUNT;n++)
  {
    buf.mtype=100+n;
    if (getBaseInfoFromModBus(0,n,&ubi)!=-1)
    {
       unit=buf.mtype-100;
       if (unit>0 && unit<=MAX_UNIT_COUNT)
       {
          if (unit_class[0][unit]<1)
          {
            uchar_t *unit_status;
            unit_status=getUnitStatusMap(0);
            if (!strncmp(ubi.property,"OTU",3))
                 unit_class[0][unit]=3;
            else if (!strncmp(buf.mbuf,"OLP",3))
                 unit_class[0][unit]=2;
            printf("Unit #%d (%s) Inserted\n",unit,ubi.property);
            bi=getBaseInfoMap(0,unit);
            if (bi!=NULL)
               strcpy(bi->property,buf.mbuf);
            else
                printf("getBaseInfoMap(%d) is NULL",unit);
            if (unit_status!=NULL)
               *(unit_status+unit)=unit_class[0][unit];
          }
       }
       //printf("Unit #%d base info(buf.mtype=%d):%s\n",buf.mtype-100,buf.mtype,buf.mbuf);
    }
   else
       printf("getBaseInfoFromModBus #%d error\n",n);
  }
}
static void init()
{
 // checkUnitStatus(0);
}

void msgHandle()
{
  msgbuf_t buf;
  int n,m;
  
 // printf("msgqid=%d\n",msgqid);
  for (n=1;n<8;n++)
  {
    buf.mtype=1;
    if (msgrcv(msgqid,&buf,MAX_IPC_MSG_BUF,0,IPC_NOWAIT)==-1)
    {
      perror("msgrcv error");
      return;
    }
    if ('U'==buf.mbuf[2])
    {
      for (m=0;m<MAX_UNIT_COUNT;m++)
      {
          if(buf.mbuf[m+5]!=unit_class[0][m])
          {
            unit_class[0][m]=buf.mbuf[m+5];
            if (unit_class[0][m]>0)
                printf("Unit #%d Inserted\n",m+1);
            else
                printf("Unit #%d Removed\n",m+1);
         }
      }
      continue;
    }
    else if ('S'==buf.mbuf[2])
    {
      sendMsgToModBus(buf);
      usleep(200);
      sendSigToModBus(SIGUSR1,1);
    }
    printf("n=%d slot=%d CMD=%c infoset_id=%d index=%d\n",n,buf.mbuf[1],buf.mbuf[2],buf.mbuf[3],buf.mbuf[4]);
    if(buf.mbuf[3]==2 && buf.mbuf[2]=='g')
      printf("%s\n",buf.mbuf+6 );
    else
    {
      m=5;
      while (m<MAX_IPC_MSG_BUF-1)
      {
        if (buf.mbuf[m]==0)
         break;
        printf("item id=%d value=%d\n",buf.mbuf[m],buf.mbuf[m+1]);
       m=m+2;
     }
   }
  }
}
void test()
{
	char key,buf[10];
	uchar_t slot;
	
	printf("=======================\n");
         //key = fgetc(stdin); 
         //scanf("%c",&key);
         printf("Select Unit(1--16)\n");
         scanf("%d",&slot);	
        //fgets(buf, 2, stdin);
	//slot=atoi(buf);
        printf("buf:%s slot=%d\n",buf,slot);	
        if (slot<1 || slot>16)
		return;
	printf("Select operation(1--3)\n");
	printf("1:init \n");
	printf("2:Insert Unit\n");
	printf("3:Remove Unit\n");
	//key=getch();
        key = fgetc(stdin); 
        scanf("%c",&key);
        printf("slot=%d key=%c\n",slot,key); 
        switch (key)
	{
		case '1':
                        //InitNvram();
			break;
		case '2':
                         insertUnit(0,slot);
			break;
		case '3':
			removeUnit(0,slot);
			break;
	}	
}

/*********************************************************************************
* 函数名称： getInfosetFromModBus
* 功能描述： 读报文信息集并存到在共享内存里的信息集。 
* 访问的表： 无
* 修改的表： 无
* 输入参数： uchar_t chassis,uchar_t slot,uchar_t *message,uchar_t msg_size
* 输出参数： 无
* 返 回 值： 成功返回0,错误返回-1
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/10/18        V1.0      李文峰        创建
*例子:
* 
************************************************************************************/
int getInfosetFromModBus(uchar_t chassis,uchar_t slot,uchar_t *message)
{
 
 uchar_t n,m,item_count;
 uchar_t infoset_id,index;
 uchar_t item_id;
 uchar_t value;
 status_infoset_t *infoset;
 char opt;
 time_t t;
 
 srand((unsigned) time(&t));
 value=rand()%3;
 if (value)
   return;
 opt=message[0];
 
 for (n=3;n<30;n++)
 {
   if (message[n]==127 ||message[n]==0)
   {
     infoset_id=message[n-2];
     index=message[n-1];
     //infoset=getStatusInfoSetMap(chassis,slot,infoset_id,index);
     infoset=getInfoSet(chassis,slot,infoset_id,index);
     if (infoset!=NULL)
     {
       item_count=infoset->item_count;
       if (item_count>=MAX_ITEM_COUNT)
         item_count=MAX_ITEM_COUNT-1;
       else if (item_count==0)
       {
          printf("item_count==0\n");
          continue;
       }
       if (item_count>0)
          item_id=rand()%item_count;
       else
           item_id=0;
       value=rand()%2+1;
       //printf("getInfosetFromModBus infoset->item_count=%d\n",infoset->item_count);
       if (infoset->item_values[item_id]!=value)
       {
          infoset->item_values[item_id]=value;
          printf("unit #%d infoset_id=%d index=%d item_id=%d changed to %d\n",slot,infoset_id,index,item_id,value);
       }
     }
   }
 }
}
/*
static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec  = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}
*/


void sigHandle(int sig, siginfo_t *si,void *myact)
{
  char key;
  int slot,infoset_id,index;
  union sigval sig_val;
  msgbuf_t msg_buf;

  if (sig == SIGUSR1)
  {
    alarm(0);
    /*
    slot=si->si_value.sival_int>>16;
    infoset_id=(si->si_value.sival_int & 0x0FFFF)>>8;
    index=si->si_value.sival_int & 0x00FF;
    printf("\nRecieved SIGUSR1 from %d slot=%d infoset id=%d index=%d\n",si->si_pid,slot,infoset_id,index);
    msg_buf.mtype=2;
    msg_buf.mbuf[0]=0;
    msg_buf.mbuf[1]=slot;
    msg_buf.mbuf[2]='G';
    msg_buf.mbuf[3]=infoset_id;
    msg_buf.mbuf[4]=index;
    sendMsgQ(msg_buf,FALSE,0);
    sig_val.sival_int=1;
    usleep(200);
    alarm(10);
    sigqueue(si->si_pid,SIGUSR1,sig_val);
    printf("sigqueue OK\n");
    */
   if (si->si_value.sival_int==0)
   {
     //getUnitStatus(0);
      detectUnitStatus(0);
   }
   else if (si->si_value.sival_int>0)
   {
     printf("detectUnitInfoset %d\n",si->si_value.sival_int);
     detectUnitInfoset(0,si->si_value.sival_int);
   }
   alarm(10);
  }
  else if (sig == SIGUSR2)
  {
     if (si->si_value.sival_int<0)
         return;
     alarm(0);
     slot=si->si_value.sival_int>>16;
     slot&=0xff;
     infoset_id=(si->si_value.sival_int & 0x0FFFF)>>8;
     index=si->si_value.sival_int & 0x00FF;
     printf("\nRecieved SIGUSR2 from %d slot=%d infoset id=%d index=%d\n",si->si_pid,slot,infoset_id,index);
     if (infoset_id==1)
     {
         //detectUnitStatus(0);
         getUnitStatus(0);
     }
     else if (infoset_id==2)
     {
        //detectUnitBaseInfo(0);
        getUnitBaseInfo(0,slot);
     }
     else
         msgHandle();
     //printf("msgHandle OK\n");
     alarm(10);
    //kill(si->si_pid,SIGUSR2);		
  }
  else if (sig == SIGALRM)
  {
    int n;
    //alarm(0);
    /*for (n=1;n<MAX_UNIT_COUNT;n++)
    {
      if (getInfosetFromModBus(0,n,2,1))
        break;
      usleep(20);
    }
    */
    alarm(0);
    //printf("Before detectUnitStatus\n");
    detectUnitStatus(0);
    //pause();
    //printf("After detectUnitStatus\n");
    alarm(10);
    //printf("SIGALRM\n");
  }
}
bool_t sig_init()
{
    struct sigaction act;
    
    sigemptyset(&act.sa_mask);  
    act.sa_flags=SA_SIGINFO;    
    act.sa_sigaction=sigHandle;

    
    sigaction(SIGALRM,&act,NULL); 
    sigaction(SIGINT,&act,NULL);
    sigaction(SIGQUIT,&act,NULL);
    sigaction(SIGTSTP,&act,NULL);
    sigaction(SIGTTIN,&act,NULL);
    sigaction(SIGTERM,&act,NULL);
    sigaction(SIGKILL,&act,NULL);
    sigaction(SIGUSR1,&act,NULL);
    sigaction(SIGUSR2,&act,NULL);
    sigaction(SIGRTMIN,&act,NULL);
    return TRUE;
}

int sendMsgToModBus(msgbuf_t msg_buf)
{
  pid_t pid=findModBusPid();
  if (pid<1)
  {
    printf("mbus not run\n");
    return -1;
  }
  
  if (sendMsgQ(msg_buf,FALSE,0)<0)
  {
   perror("sendMsgQ error");
   return -1;
  }
  if(sendSig(pid,SIGUSR1,1)<0)
  {
   perror("sendSig error");
   return -1;
  }
  return 0;
}

int main(int argc, char **argv)
{
  int n;
  /*FILE *fp;

// write pid
if ((fp = fopen("/var/run/ccenter.pid", "w")) != NULL)
{
   fprintf(fp, "%d", getpid());
   fclose(fp);
}
*/
  uchar_t *us_mbus;
  

  us_mbus=getUnitStatusMbusMap(0);
  if (us_mbus!=NULL)
  {
   for (n=0;n<16;n++)
    if (*(us_mbus+n)!=0)
     printf("n=%d uc=%d\n",n,*(us_mbus+n));
  }
  //return 0;
  sig_init();
  msgqid=CreateMsgQ(0);
//nvram_create_base_info();
  //init();
  //test();
  /* Most of time it goes to sleep */
  alarm(10);
  /*for (n=1;n<MAX_UNIT_COUNT;n++)
   if (getInfosetFromModBus(0,n,2,1))
       break;;
  */
   clearShm(0);
   insertBpuInfoSet(0);
   detectChassis(0);
  //return;
  //detectUnitStatus(0);
  //printf("\ninsertBpuInfoSet:%d\n",insertBpuInfoSet(0));
  //exit(0);
  //loadUnitInfoset(0,0,NULL,&message_set[0]);
  //loadUnitInfoset(0,0,NULL,&message_set[MAX_UNIT_COUNT]);
  while (1)
  {	
   pause();
  }
  return 1;
}


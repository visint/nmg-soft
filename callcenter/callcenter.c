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
#include <otu.h>
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
static void doSimulator(int max_count);

message_set_t message_set[MAX_UNIT_COUNT+1];


static void detectUnitStatus(uchar_t chassis)
{
   uchar_t  n;
   uchar_t *us_shm;
   unit_base_info_t *shm_bi;
   uchar_t unit_class,unit_type;
   boolean_t isvalid;

   us_shm=getUnitStatusMap(0);
}



static void insertUnit(char chassis,char slot)
{
 int n;
 char mess[MAX_ITEM_SIZE]={0,1,UNIT_BASE_INFO_SET_ID,1,1,0,0,0};
 unit_base_info_t *ubi;
 
 if (slot<0 || slot>MAX_UNIT_COUNT)
   return;
 //ubi=GetBISetFromNvram(chassis,slot);
 
 //if (getBaseInfoFromDB(chassis,slot,&ubi)!=FALSE)
 //  return;
 printf("Unit #%d pugined SN:%s\n",slot,ubi->sn); 
 unit_class[0][slot]=1;
 for (n=1;n<MAX_BASE_INFO_COUNT;n++)
 {
    //if (getBaseInfoFromUnit(chassis,slot,n,mess,TIMEOUT)!=TRUE)
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
   	 	  //strncpy(ubi->uptime,mess,11);
   	 	break;
   }
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
     sendSnmpTrap(25,0,0,m+1);
   //sendSnmpTrap(5,0,0,0);
   //sendSnmpTrap(7,0,0,1);
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
  boolean_t flag=FALSE;

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
   // if (getBaseInfoFromModBus(0,n,&ubi)!=-1)
    {
       unit=buf.mtype-100;
       //if (unit>0 && unit<=MAX_UNIT_COUNT)
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
  }
}

void msgHandle()
{
}

void sigHandle(int sig,struct siginfo *si,void *myact)
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
     //printf("detectUnitInfoset %d\n",si->si_value.sival_int);
     //detectUnitInfoset(0,si->si_value.sival_int);
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
     doSimulator(1);
    //printf("Before detectUnitStatus\n");
    //detectUnitStatus(0);
    //pause();
    //printf("After detectUnitStatus\n");
    alarm(10);
    //printf("SIGALRM\n");
  }
}
boolean_t sig_init()
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
    //sigaction(SIGKILL,&act,NULL);
    sigaction(SIGUSR1,&act,NULL);
    sigaction(SIGUSR2,&act,NULL);
    sigaction(SIGRTMIN,&act,NULL);
    return TRUE;
}

void doSimulator(max_count)
{
  time_t t;
  char uc[][20]={"UKW","NMU","MC","OTU111-25-10-80","OLP111-1-1"};

  char plugin,slot;
  unit_base_info_t *bi;
  uchar_t *us_shm;
  char uclass=0;
  int n,count;
  char property[32]={0};

  us_shm=getUnitStatusMap(0);
  if (NULL==us_shm)
     return;
  srand((unsigned) time(&t));
  count=rand()%16+1;
  if (count>max_count)
     count=max_count;
  
  for (n=1;n<=count;n++)
  {
  plugin=rand()%3;
  slot=rand()%16+1;
  
  //printf("count=%d plugin=%d slot=%d us=%d\n",count,plugin,slot,*(us_shm+slot));
  if (0==plugin)
  {
    //if (uclass==*(us_shm+slot))
    if (*(us_shm+slot)!=0)
    {
      //unit_class[0][slot]=uclass;
      *(us_shm+slot)=0;
      printf("Unit %d Removed\n",slot);
      sendSnmpTrap(51,0,slot,0);
    }
    return;
  }
  else if (1==plugin)
  {
    struct timeval tpstart;  

    if (slot<15)
      uclass=rand()%2+3;
    else
      uclass=rand()%4+1;
    //printf("count=%d plugin=%d slot=%d uc=%d us=%d\n",count,plugin,slot,uclass,*(us_shm+slot));
    if (uclass==*(us_shm+slot))
      continue;
    bi=getBaseInfoMap(0,slot);
    if (NULL==bi)
      return;
    if (uclass==3)
         sprintf(bi->property,"%s-%d",uc[uclass],1270 +20*slot);
    else if (uclass==4)
         sprintf(bi->property,"%s-%d",uc[uclass],slot);
    else if (uclass==1)
         sprintf(bi->property,"%s-%d",uc[uclass],slot);
    else
         continue;
    
    sprintf(bi->sn,"SN-XX-XX-XX-%d",slot);
    if (3==uclass)
       sprintf(bi->model,"OTU-XX-XX-XX-%d",slot);
    else if (4==uclass)
       sprintf(bi->model,"OLP-XX-XX-XX-%d",slot);
    else if (1==uclass)
       sprintf(bi->model,"NMU-XX-XX-XX-%d",slot);
    else
       sprintf(bi->model,"UKW-XX-XX-XX-%d",slot);
    sprintf(bi->fwver,"01.00.01");
    sprintf(bi->hwver,"01.00.01");
    sprintf(bi->creation,"2012-12-%d",slot);
    gettimeofday(&tpstart,NULL);
    bi->uptime=tpstart.tv_sec;
    if (*(us_shm+slot)<1)
    {
       //unit_class[0][slot]=uclass;
       *(us_shm+slot)=uclass;
       printf("Unit %d (%s) Inserted\n",slot,bi->property);
    }
    else if (*(us_shm+slot)!=uclass)
    {
       //unit_class[0][slot]=uclass;
       *(us_shm+slot)=uclass;
       printf("Unit %d Changed as %s\n",slot,bi->property);
    }
    else
        continue;
    property[0]=uclass;
    property[1]=1;
    property[2]=1;
    property[3]=1;
    property[4]=1;
    if (3==uclass)
    {
      otuProperty_t otu,*potu;
      memcpy(&otu.unit_property,property,sizeof(unitProperty_t));
      //printf("#%d uclass=%d\n",slot,otu.unit_property.unit_class);
      otu.channel_count=2;
      otu.channel_port_count=2;
      otu.port_type=3;
      otu.channel_property[0].distance=60;
      otu.channel_property[0].max_speed=1000;
      sprintf(otu.channel_property[0].wave_length,"C%d",20+slot);
      //memcpy(&otu.unit_property,&property,sizeof(unitProperty_t));
      memcpy(property,&otu,sizeof(otuProperty_t));
      potu=(otuProperty_t *)property;

     // printf("size:%d p0=%d p1=%d dis=%d wl=%s\n",sizeof(otuProperty_t),property[0],property[1],potu->channel_property[0].distance,potu->channel_property[0].wave_length);
     // loadInfosets(0,slot,(char*)otu);
    }
    //else
    loadInfosets(0,slot,property);
    sendSnmpTrap(52,0,slot,0);
  }
}
/*
  count=rand()%3+1;
  for (n=0;n<count;n++)
  {
  plugin=1-rand()%3;
  slot=rand()%16;
  if (slot<1)
      slot=1;
  if (plugin<0)
  {
   removeUnit(0,slot);
   //printf("aotu remove #%d\n",slot);
  }
  else if (plugin>0)
  {
    uclass=rand()%4+1;
   
    if (uclass==3)
         sprintf(bi.property,"%s-%d",uc[uclass],1270 +20*slot);
    else if (uclass==4)
      sprintf(bi.property,"%s-%d",uc[uclass],slot);
    else
         sprintf(bi.property,"%s-XX-XXX%d",uc[uclass],slot);
    sprintf(bi.sn,"SN-XX-XX-XX-%d",slot);
    sprintf(bi.model,"Model-XX-XX-XX-%d",slot);
    insertUnit(0,slot,bi);
    //printf("aotu insert #%d\n",slot);
  }
  else
  {
    if (unit_class[slot]>0)
      status_infoset[slot].item_values[rand()%3]=rand()%2+1;
  }
 }
*/
}
int main(int argc, char **argv)
{
  int n;
  /*uchar_t *us_mbus;
  

  us_mbus=getUnitStatusMbusMap(0);
  if (us_mbus!=NULL)
  {
   for (n=0;n<16;n++)
    if (*(us_mbus+n)!=0)
     printf("n=%d uc=%d\n",n,*(us_mbus+n));
  }
*/
  //return 0;
  clearShm(0);
  insertBpuInfoSet(0);
  doSimulator(16);
  sig_init();
  //msgqid=CreateMsgQ(0);
  alarm(10);

  //insertBpuInfoSet(0);
  //detectChassis(0);
  while (1)
  {	
   pause();
  }
  return 1;
}


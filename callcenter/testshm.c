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
#include <olp.h>
//arm-linux-gcc -o shm  testshm.c -I../include -L=./ -L/home/visint/src/arm/lib -lvispace -lsqlite3 -liconv
#define  STATUS_SHM_KEY 76
static int   usi_shm_id = -1;
static char *usi_shm_add=NULL;

static int   ubi_shm_id = -1;

static char *ubi_shm_add=NULL;
void initSys()
{
    clearShm(0);
    insertBpuInfoSet(0);
}
void insertOlp(short slot)
{
  char n;
    unit_base_info_t *pbi;
    char property[32]={0};
    olp1P1Info_t *olp;
    char *values;
    uchar_t *us_shm;
    char *p,buf[32]={0xE8,0xFE,0x88,0xFF,0xE8,0xFE,0xE8,0xFE,0x92,0xFF,0x4C,0xFF,0x4D,0x01,1,1,1,1,1,1,1,1,1,1};

    us_shm=getUnitStatusMap(0);
    if (NULL==us_shm)
        return;

    pbi=getBaseInfoMap(0,slot);
    if (NULL==pbi)
        return;

    property[0]=UC_OLP;
    property[1]=1;
    property[2]=1;
    property[3]=1;
    property[4]=1;
    loadInfosets(0,slot,property);
     *(us_shm+slot)=UC_OLP;
    
    values=getInfoSetValues(0,slot,65,1);
    if (values!=NULL)
    {
      /**(values+0)=(char)0xE8;
      *(values+1)=(char)0xFE;
      *(values+2)=(char)0xE8;
      *(values+3)=(char)0xFE;
      *(values+4)=(char)0xE8;
      *(values+5)=(char)0xFE;
      values[6]=(char *)0xE8;
      values[7]=(char *)0xFE;
      values[8]=(char *)0xE8;
      values[9]=(char *)0xFE;
      values[10]=0xE8;
      values[11]=0xFE;
      values[12]=0x4D;
      values[13]=0x01;
      for (n=14;n<28;n++)
       values[n]=1;
      */
      memcpy(values,buf,32);
    }
    olp=(olp1P1Info_t *)getOlp1P1Info(0,slot);
    //p=buf;
    //olp=(olp1P1Info_t *)p;
    if (NULL==olp)
    {
      printf("Olp is null\n");
      return;
    }
    //memcpy((char *)olp,buf,sizeof(olp1P1Info_t));
    printf("values add=%d olp add=%d\n",values,olp);
    //olp=(olp1P1Info_t*)usi_shm_add;
    /*olp->mode=1;
    olp->line=1;
    olp->rule=1;
    olp->ret_mode=1;
    olp->rx1_led=1;
    olp->rx2_led=1;
    olp->tx_led=1;
    olp->rx1_wave_len=1;
    olp->rx2_wave_len=1;
    olp->tx_wave_len=1;
    olp->ret_time=330;
    olp->rx1_power=-280;
    olp->rx2_power=-280;
    olp->tx_power=-180;
    olp->l1_power=-120;
    olp->l2_power=-110;
    olp->tx_alm_power=-280;
    printf("olp=%d\n",olp);
    */
    printf("\nSlot:%d\nmode=%d line=%d rule=%d ret_mode=%d rx1=%d wave_len=%d ret_time=%d rx1=%d rx2=%d tx=%d l1=%d l2=%d alm=%d\n",slot,olp->mode,olp->line,olp->rule,olp->ret_mode,olp->rx1_led,olp->tx_wave_len,olp->ret_time,olp->rx1_power,olp->rx2_power,olp->tx_power,olp->l1_power,olp->l2_power,olp->tx_alm_power);
}

#define  INFO_SET_MAX_COUNT 170
#define  INFO_SET_MAX_COUNT_ERVERY_UNIT 10
#define  STATUS_SHM_KEY 76
#define  BASE_INFO_SHM_KEY 78
#define  INFO_HEAD_SIZE 200

#define  CMD_MESSAGE_OFFSET 36  //命令报文
#define  DATA_MESSAGE_OFFSET 72 //数据报文

//#define  INFO_SET_HEAD_OFFSET 96 //存放单元盘Infoset的节点开始位置
//#define  INFO_SET_COUNT_OFFSET 112  //存放单元盘Infoset的节点数

#define  INFO_UNIT_PROPERTY_OFFSET 108 //存放单元盘属性，如波道数，每单元盘的属性为4个字节
#define  INFO_UNIT_PROPERTY_SIZE 68

#define  INFO_SET_HEAD_OFFSET 176 //存放单元盘Infoset的节点的信息，如节点数
#define  INFO_SET_OFFSET 208  //存放单元盘Infoset的节点

void main () 
{
 //clearShm(0);
 char n;
 static int   usi_shm_id = -1;
 static char *usi_shm_add=NULL;
 olp1P1Info_t *olp;
 char *p,buf[32]={0xE8,0xFE,0x88,0xFF,0xE8,0xFE,0xE8,0xFE,0x92,0xFF,0x4C,0xFF,0x4D,0x01,1,1,1,1,1,1,1,1,1,1};
 char * values;
    usi_shm_id = shmget(STATUS_SHM_KEY,INFO_HEAD_SIZE+sizeof(status_infoset_t)*INFO_SET_MAX_COUNT, IPC_CREAT | 0666);
    //printf("usi_shm_id =%d\n",usi_shm_id );
    if(usi_shm_id==-1)
    {
        perror("shmget error");
        return FALSE;
    }
    usi_shm_add=(char*)shmat(usi_shm_id,NULL,0);
    if(usi_shm_add == (void *) -1)
    { 
      perror("shmat");
      printf("map the shm error: %s.\n", strerror(errno));
      return FALSE; 
    }
    usi_shm_add=  usi_shm_add+INFO_HEAD_SIZE;
    values=usi_shm_add+6;
    memcpy(values,buf,30);
    olp=(olp1P1Info_t *)values;
    printf("\nSlot:%d\nmode=%d line=%d rule=%d ret_mode=%d rx1=%d wave_len=%d ret_time=%d rx1=%d rx2=%d tx=%d l1=%d l2=%d alm=%d\n",1,olp->mode,olp->line,olp->rule,olp->ret_mode,olp->rx1_led,olp->tx_wave_len,olp->ret_time,olp->rx1_power,olp->rx2_power,olp->tx_power,olp->l1_power,olp->l2_power,olp->tx_alm_power);
}
void main2_ () 
{ 
    //char *usi_shm_add;
    //int iset_off;
    //int iset_add[16]={0};
    char n;
    unit_base_info_t *pbi;
    char property[32]={0};
    olp1P1Info_t *olp;

    uchar_t *us_shm;
    
    /*
    us_shm=getUnitStatusMap(0);
    if (NULL==us_shm)
        return;

    pbi=getBaseInfoMap(0,5);
    if (NULL==pbi)
        return;

    usi_shm_id = shmget(STATUS_SHM_KEY+1,360, IPC_CREAT | 0666);
    */
    //printf("usi_shm_id =%d\n",usi_shm_id );
    /*if(usi_shm_id==-1)
    {
        perror("shmget error");
        return ;
    }
    usi_shm_add=(char*)shmat(usi_shm_id,NULL,0);
    if(usi_shm_add == (void *) -1)
    { 
      perror("shmat");
      printf("map the shm error: %s.\n", strerror(errno));
      return ;
    }
*/
    initSys();
    for (n=1;n<16;n++)
    {
      //if (n%2==0)
      insertOlp(n);
    }
    //insertOlp(5);
    //insertOlp(7);
    //insertOlp(8);
    //printf("%d\n",insertOlpInfoSet(0,5,1));
    /*property[0]=UC_OLP;
    property[1]=1;
    property[2]=1;
    property[3]=1;
    property[4]=1;
    loadInfosets(0,5,property);
     *(us_shm+5)=UC_OLP;
    
    olp=getOlp1P1Info(0,5);
    if (NULL==olp)
    {
      printf("Olp is null\n");
      return;
    }
    //olp=(olp1P1Info_t*)usi_shm_add;
    olp->mode=1;
    olp->line=1;
    olp->rule=1;
    olp->ret_mode=1;
    olp->ret_time=330;
    olp->rx1_power=-280;
    olp->rx2_power=-280;
    olp->tx_power=-180;
    olp->l1_power=-120;
    olp->l2_power=-110;
    olp->tx_alm_power=-280;
    printf("ret_mode=%d\n",olp->ret_mode);
    printf("\nHere!\nmode=%d line=%d rule=%d ret_mode=%d ret_time=%d rx1=%d rx2=%d tx=%d l1=%d l2=%d alm=%d\n",olp->mode,olp->line,olp->rule,olp->ret_mode,olp->ret_time,olp->rx1_power,olp->rx2_power,olp->tx_power,olp->l1_power,olp->l2_power,olp->tx_alm_power);
    */
    return ;
}

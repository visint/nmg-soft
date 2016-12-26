#include <stdio.h>
#include <string.h>
#include <infoset.h>
#include <visipc.h>
#include <chassis.h>
#include <ddm.h>
#include <olp.h>
#include <otu.h>
#include <infoset.h>
/*int loadInfoset(uchar_t chassis,uchar_t slot,uchar_t unit_class,uchar_t unit_type)
{
  sqlite3 *db=NULL;
   int rc;
   char sql[200]="";
  char infoset_name[30]="";
  result_t result;
  int n,m,item_count=0,status_count=0;
  char   col_name[10][16]={ "","", "","", "","","", "",""};
  char   dataset[FIELD_MAX_COUNT][FIELD_MAX_SIZE]={ "","", "","", "","","", "",""};
}
*/
int insertBpuInfoSet(uchar_t  chassis)
{
  status_infoset_t infosets[2];
  int n,infoset_count=0;
  //deleteSlotNodes(chassis,0);

  memset(&infosets[0],0,sizeof(status_infoset_t));
  infosets[0].infoset_id=CHASSIS_INFOSET_ID;
  //infoset_count=1;
  infosets[0].index=1;     
  infosets[0].infoset_type=0;
  infosets[0].infoset_size=24;
  infosets[0].item_count=17;
  
   memset(&infosets[1],0,sizeof(status_infoset_t));
   infosets[1].infoset_id=CHASSIS_THRESHOLD_INFOSET_ID;
   infosets[1].index=1;     
   infosets[1].infoset_type=2;
   infosets[1].infoset_size=12;
   infosets[1].item_count=8;

  for (n=0;n<MAX_ITEM_COUNT;n++)
  {
   infosets[0].item_values[n]=1;
   infosets[1].item_values[n]=1;
  }
  if (insertInfoSet(chassis,0,infosets,2)!=-1)
  {
    return 1;
  }
  else
    return -1;
}

int insertNmuInfoSet(uchar_t  chassis)
{
  int n;
  status_infoset_t infosets[2];

  memset(&infosets[0],0,sizeof(status_infoset_t));
  //memset(&infoset[1],0,sizeof(status_infoset_t));
  infosets[0].infoset_id=OLP_STATUS_INFOSET_ID;
  //infosets[1].infoset_id=DDM_INFOSET_ID;
  //infoset_count=1;
  infosets[0].index=1;     
  infosets[0].infoset_type=0;
  infosets[0].infoset_size=18;
  infosets[0].item_count=11;
  /*
  infosets[1].index=1;     
  infosets[1].infoset_type=2;
  infosets[1].infoset_size=20;
  infosets[1].item_count=10;
  */
  for (n=0;n<MAX_ITEM_COUNT;n++)
  {
     infosets[0].item_values[n]=1;
     //infosets[1].infoitems[n].value=0;
  }
  if (insertInfoSet(chassis,16,infosets,1)!=-1)
    return 1;
  else
    return -1;
}

int insertOtuInfoSet(uchar_t chassis,uchar_t slot,uchar_t unit_type)
{
  status_infoset_t infosets[4];
  ddmThreshold_t *ddmt,*pddmt;

  int n,infoset_count=0;
  //deleteSlotNodes(chassis,0);

  memset(&infosets[0],0,sizeof(status_infoset_t));
  memset(&infosets[1],0,sizeof(status_infoset_t));
  memset(&infosets[2],0,sizeof(status_infoset_t));
  memset(&infosets[3],0,sizeof(status_infoset_t));
  //memset(&infosets[4],0,sizeof(status_infoset_t));

  infosets[0].infoset_id=UNIT_PROPERTY_INFOSET_ID;
  infosets[1].infoset_id=OTU_STATUS_INFOSET_ID;
  infosets[2].infoset_id=DDM_INFOSET_ID;
  infosets[3].infoset_id=DDM_THRESHOLD_ID;
  //infosets[4].infoset_id=DDM_THRESHOLD_ID;

  //infoset_count=1;
  infosets[0].index=1;     
  infosets[0].infoset_type=0;
  infosets[0].infoset_size=sizeof(otuProperty_t);
  infosets[0].item_count=14;

  infosets[1].index=1;     
  infosets[1].infoset_type=1;
  infosets[1].infoset_size=21;
  infosets[1].item_count=21;
  
  infosets[2].index=1;     
  infosets[2].infoset_type=2;
  infosets[2].infoset_size=20;
  infosets[2].item_count=10;

  infosets[3].index=1;     
  infosets[3].infoset_type=1;
  infosets[3].infoset_size=20;
  infosets[3].item_count=20;
   /*
  infosets[4].index=2;     
  infosets[4].infoset_type=2;
  infosets[4].infoset_size=20;
  infosets[4].item_count=10;
   */

  for (n=0;n<MAX_ITEM_SIZE;n++)
  {
     infosets[0].item_values[n]=0;
     infosets[1].item_values[n]=0;
     infosets[2].item_values[n]=0;
     infosets[3].item_values[n]=0;
     //infosets[4].item_values[n]=0;
  }
  infosets[0].item_values[5]=3;
  infosets[0].item_values[6]=1;
  infosets[0].item_values[7]=1;
  infosets[0].item_values[8]=1;
  infosets[0].item_values[9]=1;
  infosets[0].item_values[10]=1;
  /*
  ddmt.rxPowerMax=1;   //< 输入最大光功率单位:0.1dBm
  ddmt.rxPowerMin=-500;  !< 输入最小光功率单位:0.1dBm
  ddmt.txPowerMax=1;  < 输出光功率单位:0.1dBm
  ddmt.txPowerMin=-500;  < 输出最小光功率单位:0.1dBm
  ddmt.tempMax=800;     < SFP  最大温度单位:0.1°C
  ddmt.tempMin=-20;     < SFP  最小温度单位:0.1°C
  ddmt.voltMax=8000;    < SFP  最大电压单位:毫伏
  ddmt.voltMin=3000;     < SFP  最小电压单位:毫伏
  ddmt.rxBiasMax=30000;   < SFP  最大偏置电流单位:毫安
  ddmt.rxBiasMin=5000;   < SFP  最小偏置电流单位:毫安
  memcpy(&infosets[2].item_values,&ddmt,sizeof(ddmThreshold_t));
  memcpy(&infosets[3].item_values,&ddmt,sizeof(ddmThreshold_t));
*/
  //return insertStatusInfoSet(chassis,0,infoset);
  printf("insertOtuInfoSet %d\n",slot);
  if (insertInfoSet(chassis,slot,infosets,4)!=-1)
  {
    //groupDdmThreshold_t *gddmt=getGroupDdmThreshold(0,slot,1);
    pddmt=getPortDdmThreshold(0, slot,1);
    if (pddmt!=NULL)
      {
      for (n=1;n<3;n++)
       {
       pddmt=getPortDdmThreshold(0, slot,n);
       if (pddmt!=NULL)
         {
         pddmt->rxPowerMax=1;  /*!< 输入最大光功率单位:dBm*/
         pddmt->rxPowerMin=-50;  /*!< 输入最小光功率单位:dBm*/
         pddmt->txPowerMax=1;  /*!< 输出光功率单位:dBm*/
         pddmt->txPowerMin=-50;  /*!< 输出最小光功率单位:dBm*/
         pddmt->voltMax=8;     /*!< SFP  最大电压单位:伏*/
         pddmt->voltMin=3;     /*!< SFP  最小电压单位:伏*/
         pddmt->rxBiasMax=30;   /*!< SFP  最大偏置电流单位:毫安*/
         pddmt->rxBiasMin=5;   /*!< SFP  最小偏置电流单位:毫安*/
         pddmt->tempMax=80;     /*!< SFP  最大温度单位:°C*/
         pddmt->tempMin=-20;     /*!< SFP  最小温度单位:°C*/
         pddmt++;
         }
       }
     }
   return 5;
  }
  else
    return -1;
}

int insertOlpInfoSet(uchar_t chassis,uchar_t slot,uchar_t unit_type)
{
  status_infoset_t infosets[2];
  int n;
  memset(&infosets[0],0,sizeof(status_infoset_t));
  memset(&infosets[1],0,sizeof(status_infoset_t));
  //memset(&infoset[1],0,sizeof(status_infoset_t));
  infosets[0].infoset_id=UNIT_PROPERTY_INFOSET_ID;
  infosets[0].index=1;     
  infosets[0].infoset_type=1;
  infosets[0].infoset_size=5;
  infosets[0].item_count=5;
  infosets[0].item_values[0]=4;
  infosets[0].item_values[1]=1;
  infosets[0].item_values[2]=1;
  infosets[0].item_values[3]=1;
  infosets[0].item_values[4]=1;
  infosets[0].item_values[5]=4;
  infosets[0].item_values[6]=1;
  infosets[0].item_values[7]=1;
  infosets[0].item_values[8]=1;
  infosets[0].item_values[9]=1;
  infosets[0].item_values[10]=1;

  infosets[1].infoset_id=OLP_STATUS_INFOSET_ID;
  //infosets[1].infoset_id=DDM_INFOSET_ID;
  //infoset_count=1;
  infosets[1].index=1;     
  infosets[1].infoset_type=0;
  infosets[1].infoset_size=21;
  infosets[1].item_count=12;
  /*
  infosets[1].index=1;     
  infosets[1].infoset_type=2;
  infosets[1].infoset_size=20;
  infosets[1].item_count=10;
  */
 /* for (n=0;n<MAX_ITEM_COUNT;n++)
  {
     infosets[0].item_values[n]=1;
     infosets[1].infoitems[n].value=0;
  }
*/
  if (insertInfoSet(chassis,slot,infosets,2)!=-1)
  {
    olp1P1Info_t  *olp;
    olp=getOlp1P1Info(chassis,slot);
    if(olp!=NULL)
    {
       olp->mode=1;
       olp->line=1;
       olp->rule=1;
       olp->ret_mode=1;
       olp->ret_time=0;
       olp->rx1_power=-210;
       olp->rx2_power=-220;
       olp->tx_power=-120;
       olp->l1_power=-280;
       olp->l2_power=-280;
       olp->tx_alm_power=-280;
       olp->rx1_led=1;
       olp->rx2_led=1;
       olp->tx_led=1;
    }
    return 1;
  }
  else
    return -1;
}

int sendMessageToUnit1(uchar_t chassis,uchar_t slot,char message[],unsigned int useconds)
{
  char n;
  uchar_t uclass,utype;
  uchar_t item_ids[20]={0};
  short item_val[20]={0};

  printf("#%d:%c infosetid=%d index=%d ",slot,message[0],message[1],message[2]);
  if (message[0]=='s')
  {
    for (n=0;n<MAX_MESSAGE_SIZE;n++)
    {
     if (message[3+2*n]!=0)
       printf("%d=%d ",message[3+2*n],message[3+2*n+1]);
     else
       break;
    }
  }
  else if (message[0]=='S')
  {
    short value;
    for (n=0;n<MAX_MESSAGE_SIZE;n++)
    {
     if (message[3+3*n]!=0)
     {
       memcpy(&value,&message[3+3*n+1],sizeof(short));
       item_ids[n]=message[3+3*n];
       item_val[n]=value;
       printf("%d=%d ",message[3+3*n],value);
     }
     else
       break;
    }
  }
  else
       return -1;
  printf("useconds=%d\n",useconds);
  if (useconds>0)
  {
    printf("sleep %d micro seconds..\n",useconds);
    usleep(useconds);
  }
  if (FALSE!=getUnitClassType(0,slot,&uclass,&utype))
   {
    if (4==uclass)
     {
      olp1P1Info_t  *olp=getOlp1P1Info(chassis,slot);
      if (olp!=NULL)
        {
         for (n=0;n<10;n++)
            {
           switch (item_ids[n])
               {
              case 1:
                   //olp->mode=item_value[n]
                     //printf("mode=%d\n",item_val[n]);
                     if (item_val[n]>0 && item_val[n]<3)
                         olp->mode=item_val[n];
                break;
              case 2:
                    // printf("line=%d\n",item_val[n]);
                     if (item_val[n]>0 && item_val[n]<3)
                         olp->line=item_val[n];
                break;
              case 3:
                     //printf("rule=%d\n",item_val[n]);
                     if (item_val[n]>0 && item_val[n]<3)
                         olp->rule=item_val[n];
                break;
               case 4:
                      printf("ret mode=%d\n",item_val[n]);
                      if (item_val[n]>0 && item_val[n]<3)
                         olp->ret_mode=item_val[n];
                break;
                case 5:
                     //printf("ret time=%d\n",item_val[n]);
                     if (item_val[n]>=0 && item_val[n]<1000)
                         olp->ret_time=item_val[n];
                   break;
               case 9:
                     //printf("l1 sw power=%d\n",item_val[n]);
                     if (item_val[n]>=OLP_MIN_L1_SW_POWER && item_val[n]<OLP_MAX_L1_SW_POWER)
                         olp->l1_power=item_val[n];
                   break;
               case 10:
                     //printf("l12 sw power=%d\n",item_val[n]);
                     if (item_val[n]>=OLP_MIN_L2_SW_POWER && item_val[n]<OLP_MAX_L2_SW_POWER)
                         olp->l2_power=item_val[n];
                     break;
               case 11:
                     //printf("l12 sw power=%d\n",item_val[n]);
                     if (item_val[n]>=OLP_MIN_TX_ALM_POWER && item_val[n]<OLP_MAX_TX_ALM_POWER)
                         olp->tx_alm_power=item_val[n];
                break;
               }
            }
        }
     }
   }
  return 0;
}

int setShortInfoItem(
    uchar_t chassis,
    uchar_t slot,
    //char_t infoset_type;//'s'-字节型 'S'-双字节 'W'-混合型
    uchar_t infoset_id,
    uchar_t index,
    uchar_t item_id,
    short value,
    unsigned int useconds)
{
  char message[MAX_MESSAGE_SIZE]={0};
  message[0]='S';
  message[1]=infoset_id;
  message[2]=index;
  message[3]=item_id;
  memcpy(&message[4],&value,sizeof(short));
  message[6]=0;
  sendMessageToUnit(chassis,slot,message,useconds);
   
}

int setByteInfoItem(
    uchar_t chassis,
    uchar_t slot,
    //char_t infoset_type;//'s'-字节型 'S'-双字节 'W'-混合型
    uchar_t infoset_id,
    uchar_t index,
    uchar_t item_id,
    char value,
    unsigned int useconds)
{
  char message[MAX_MESSAGE_SIZE]={0};
  message[0]='s';
  message[1]=infoset_id;
  message[2]=index;
  message[3]=item_id;
  message[4]=value;
  message[5]=0;
  sendMessageToUnit(chassis,slot,message,useconds);
}
int setUnitInfoSet(setInfoSet_t infoset)
{
  int ret=-1;

  switch (infoset.infoset_id)
  {
    case OTU_STATUS_INFOSET_ID:
      break;
    case OLP_STATUS_INFOSET_ID:
       ret=setOlpInfo(infoset);
      break;
    case DDM_THRESHOLD_ID:
      break;
    case CHASSIS_THRESHOLD_INFOSET_ID:
      break;
    default:
     return -1;
  }
  if (ret==-1)
     return -1;
  return 0;
}


int loadInfosets(uchar_t chassis,uchar_t slot,char property[])
{
  otuProperty_t *otu;
  //printf("uclass=%d\n",property[0]);
  switch (property[0])
  {
    case 3:
           otu=(otuProperty_t *)property;
           //printf("speed=%d wl:%s\n",otu->channel_property[0].max_speed,otu->channel_property[0].wave_length);
           insertOtuInfoSets(chassis,slot,*otu);
    break;
    case 4:
          insertOlpInfoSet(0,slot,1);
    break;
  }
 // printf("uc=%d\n",property[0]);
}

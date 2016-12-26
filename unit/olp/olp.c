#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unit.h>
#include <visipc.h>
#include <olp.h>

/*
bool_t getOlp1P1Info(uchar_t chassis,uchar_t slot,olp1StatusInfo_t *info)
{
   char *values;
   chassis_t *cinfo=NULL;
   values=getInfoSetValues(chassis,0,CHASSIS_INFOSET_ID,0);
   if (NULL!=values)
   {
      cinfo=(chassis_t *)(values+5);
      //printf("getChassisInfo NOT null!\n");  
   }

  uchar_t n,m,uclass=0,utype=0;
  unit_base_info_t *bi;
  status_infoset_t *sis;
  //char *p,property[20]="",port_type[8]="";
  //char buf[30]="OTU111-25-10-80-1550";
  if (TRUE!=getUnitClassType(chassis,slot,&uclass,&utype) || uclass!=4)
      return FALSE;
  
  sis=getStatusInfoSetMap(chassis,slot,OLP_STATUS_INFOSET_ID,1);
  if (NULL==sis)
     return FALSE;
  //printf("uclass=%d\n",uclass);
  info->operating_mode=sis->item_values[1];
  info->line=sis->item_values[2];
  info->rule=sis->item_values[3];
  info->ret_mode=sis->item_values[4];
  return TRUE;
}

bool_t getOlp1P1Data(uchar_t chassis,uchar_t slot,olp1Data_t *info)
{
  uchar_t n,m,uclass=0,utype=0;
  unit_base_info_t *bi;
  status_infoset_t *sis;
  //char *p,property[20]="",port_type[8]="";
  //char buf[30]="OTU111-25-10-80-1550";
  if (TRUE!=getUnitClassType(chassis,slot,&uclass,&utype) || uclass!=4)
      return FALSE;
  
  sis=getStatusInfoSetMap(chassis,slot,OLP_DATA_INFOSET_ID,1);
  if (NULL==sis)
     return FALSE;
  //printf("uclass=%d\n",uclass);
  
  info->power.rx1_power=sis->item_values[1];
  info->power.rx1_power=info->power.rx1_power<<8;
  info->power.rx1_power|=sis->item_values[2];

  info->power.rx2_power=sis->item_values[3];
  info->power.rx2_power=info->power.rx2_power<<8;
  info->power.rx2_power|=sis->item_values[4];

  info->power.tx_power=sis->item_values[5];
  info->power.tx_power=info->power.tx_power<<8;
  info->power.tx_power|=sis->item_values[6];

  info->power.l1_power=sis->item_values[7];
  info->power.l1_power=info->power.l1_power<<8;
  info->power.l1_power|=sis->item_values[8];

  info->power.l2_power=sis->item_values[9];
  info->power.l2_power=info->power.l2_power<<8;
  info->power.l2_power|=sis->item_values[10];
  
  info->power.tx_alm_power=sis->item_values[11];
  info->power.tx_alm_power=info->power.tx_alm_power<<8;
  info->power.tx_alm_power|=sis->item_values[12];

  info->ret_time=sis->item_values[13];
  info->ret_time=info->ret_time<<8;
  info->ret_time|=sis->item_values[14];

  //printf("info->ret_time=%d\n",info->ret_time);
  return TRUE;
}
*/
olp1P1Info_t  *getOlp1P1Info(uchar_t chassis,uchar_t slot)
{
  char *values;
  short value;

  olp1P1Info_t *olp=NULL;
  //printf("getOlp1P1Info #%d uc=%d\n",slot,getUnitClass(chassis,slot));
  if (4!=getUnitClass(chassis,slot))
     return NULL;
  values=getInfoSetValues(chassis,slot,OLP_STATUS_INFOSET_ID,1);
  if (NULL!=values)
  {
      olp=(olp1P1Info_t*)(values);  
  }
  return  olp;
}

int setOlpInfo(setInfoSet_t infoset)
{
  char n,m=3,flag;
  char message[512];
  short value;
  olp1P1Info_t  *olp;
  
  olp=getOlp1P1Info(0,infoset.slot);
  if (olp==NULL)
    return -1;
  memset(message,0,MAX_MESSAGE_SIZE*sizeof(char));
  for (n=0;n<MAX_SETTING_ITEM_COUNT;n++)
  { 
    flag=1;
    if (infoset.infoItems[n].item_id<1)
      break;
    switch (infoset.infoItems[n].item_id)
    {
       case OLP1_MODE:
         //olp->mode=infoset.infoItems[n].item_value & 0x3;
         value=infoset.infoItems[n].item_value & 0x3;
         break;
       case OLP1_LINE :
         //olp->line=infoset.infoItems[n].item_value & 0x3;
         value=infoset.infoItems[n].item_value & 0x3;
         break;
       case OLP1_RULE:
         //olp->rule=infoset.infoItems[n].item_value & 0x3;
         value=infoset.infoItems[n].item_value & 0x3;
         break;
       case OLP1_RET_MODE:
         //olp->ret_mode=infoset.infoItems[n].item_value & 0x3;
         value=infoset.infoItems[n].item_value & 0x3;
         break;
       case OLP1_RET_TIME:
         if (infoset.infoItems[n].item_value<OLP_MIN_RET_TIME)
             value=OLP_MIN_RET_TIME;
         else if (infoset.infoItems[n].item_value>OLP_MAX_RET_TIME)
             value=OLP_MAX_RET_TIME;
         else
             value=infoset.infoItems[n].item_value;
         //value=olp->ret_time;
         break;
       case OLP1_L1_SW_POWER:
         if (infoset.infoItems[n].item_value<OLP_MIN_L1_SW_POWER)
             value=OLP_MIN_L1_SW_POWER;
         else if (infoset.infoItems[n].item_value>OLP_MAX_L1_SW_POWER)
             value=OLP_MAX_L1_SW_POWER;
         else
             value=infoset.infoItems[n].item_value;
             //value=olp->l1_power;
             //printf("n=%d L1_SW_POWER=%d l1_power=%d value=%d\n",n,infoset.infoItems[n].item_value,olp->l1_power,value);
         break;
       case OLP1_L2_SW_POWER :
         if (infoset.infoItems[n].item_value<OLP_MIN_L2_SW_POWER)
             value=OLP_MIN_L2_SW_POWER;
         else if (infoset.infoItems[n].item_value>OLP_MAX_L2_SW_POWER)
             value=OLP_MAX_L2_SW_POWER;
         else
             value=infoset.infoItems[n].item_value;
             //value=olp->l2_power;
         break;
      case OLP1_TX_ALM_POWER:
         if (infoset.infoItems[n].item_value<OLP_MIN_TX_ALM_POWER)
             value=OLP_MIN_TX_ALM_POWER;
         else if (infoset.infoItems[n].item_value>OLP_MAX_TX_ALM_POWER)
             value=OLP_MAX_TX_ALM_POWER;
         else
             value=infoset.infoItems[n].item_value;
             //olp->tx_alm_power=value;
         break;
       case OLP1_R1_WAVE_LEN:
       case OLP1_R2_WAVE_LEN:
       case OLP1_TX_WAVE_LEN:
         //olp->ret_mode=infoset.infoItems[n].item_value & 0x3;
         value=infoset.infoItems[n].item_value & 0x3;
         break;
       default:
             flag=0;
    }
   if (flag)
   {
      message[m]=infoset.infoItems[n].item_id;
      //memcpy(&message[m+1],&value,sizeof(short));
      message[m+1]=(value & 0XFF00)>>8;
      message[m+2]= (value & 0XFF);
      m+=3;
      if (m>29)
        break;
   }
  }
   //printf("set olp l1_p=%d l2_p=%d tx=%d\n",olp->l1_power,olp->l2_power,olp->tx_alm_power);
  if (m>3)
   {
     message[0]='S';
     message[1]=infoset.infoset_id;
     message[2]=infoset.index;
     sendMessageToUnit(infoset.chassis,infoset.slot,message,infoset.useconds);
   }
   //printf("rx1:%d l1:%d\n",olp->rx1_power,olp->l1_power);
   /*if (olp->rx1_power<olp->l1_power)
       sendSnmpTrap(409,0,infoset.slot,0);
   if (olp->rx2_power<olp->l2_power)
       sendSnmpTrap(411,0,infoset.slot,0);
   if (olp->tx_power<olp->tx_alm_power)
       sendSnmpTrap(413,0,infoset.slot,0);
   */
}

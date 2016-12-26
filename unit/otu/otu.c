#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unit.h>
#include <infoset.h>
#include <visipc.h>
#include <ddm.h>
#include <otu.h>
/*
otuInfo_t * getOtuInfo(uchar_t chassis,uchar_t slot,uchar_t channel)
{
  uchar_t n,m,uclass=0,utype=0;
  unit_base_info_t *bi;
  status_infoset_t *sis;
  char *p,property[20]="",port_type[8]="";

  char *values;
  otuInfo_t *otu=NULL;

  //char buf[30]="OTU111-25-10-80-1550";
  if (TRUE!=getUnitClassType(chassis,slot,&uclass,&utype) || uclass!=3)
      return FALSE;
  //printf("slot=%d uclass=%d\n",slot,uclass);
  bi=getBaseInfoMap(chassis,slot);
  if (bi==NULL)
     return FALSE;
  info->channel_count=1;
  //p=bi->property;
  p=strchr(bi->property,'/');
  while (p!=NULL)
   {
     info->channel_count++;
     p++;
     p=strchr(p,'/');
   }
   
  //info->channel_count=bi->property[3]-48;
  sscanf(bi->property,"%[0-9,A-Z]-%[0-9,A-Z]-%[0-9,A-Z]-%[0-9,A-Z]-%[0-9,A-Z]",property,port_type,info->channel[0].max_speed,info->channel[0].distance,info->channel[0].wave_length);
  info->channel[0].port_count=bi->property[7]-48;
   if (info->channel[0].port_count>2)
        info->channel[0].port_count=2;
  for (n=0;n<info->channel[0].port_count;n++)
  {
   values=getInfoSetValues(chassis,0,CHASSIS_INFOSET_ID,0);
   if (NULL!=values)
   {
      cinfo=(chassis_t *)(values+5);
      //printf("getChassisInfo NOT null!\n");  
   }
     //sis=getStatusInfoSetMap(chassis,slot,OPORT_INFOSET_ID,n+1);
     sis=getInfoSet(chassis, slot,OTU_STATUS_INFOSET_ID,n+1)
     if (NULL!=sis)
     {
       info->channel[0].rx_status[n]=sis->item_values[0];
       info->channel[0].tx_status[n]=sis->item_values[1];
     }
  }
  return TRUE;
}
*/
otuInfo_t *getOtuInfo(uchar_t chassis,uchar_t slot,uchar_t channel)
{
  char *values;
  otuInfo_t *otu=NULL;
  values=getInfoSetValues(chassis,0,OTU_STATUS_INFOSET_ID,channel);
  if (NULL!=values)
  {
      otu=(otuInfo_t*)(values);
      //printf("getChassisInfo NOT null!\n");  
  }
  return  otu;
}


otuProperty_t *getOtuProperty(uchar_t chassis,uchar_t slot,uchar_t index)
{
  char *values;
  otuProperty_t *otu=NULL;
  if (3!=getUnitClass(chassis,slot))
     return NULL;
  values=getInfoSetValues(chassis,slot,UNIT_PROPERTY_INFOSET_ID,index);
  if (NULL!=values)
  {
      otu=(otuProperty_t*)(values+5);  
  }
  return  otu;
}

otuChannelStatus_t *getOtuChannelStatus(uchar_t chassis,uchar_t slot,uchar_t group)
{
  char *values;
  otuChannelStatus_t *otu=NULL;
  if (3!=getUnitClass(chassis,slot))
     return NULL;
  values=getInfoSetValues(chassis,slot,OTU_STATUS_INFOSET_ID,group);
  if (NULL!=values)
  {
      otu=(otuChannelStatus_t*)(values);  
  }
  else
       printf("getOtuChannelStatus is null\n");
  return  otu;
}

int insertOtuInfoSets(uchar_t chassis,uchar_t slot,otuProperty_t otu)
{
  #define OTU_INFOSET_COUNT 10

  status_infoset_t infosets[OTU_INFOSET_COUNT];
  ddmThreshold_t *ddmt,*pddmt;

  int n,m,infoset_count=0;
  //deleteSlotNodes(chassis,0);

  for (n=0;n<OTU_INFOSET_COUNT;n++)
    memset(&infosets[n],0,sizeof(status_infoset_t));

  infosets[0].infoset_id=UNIT_PROPERTY_INFOSET_ID;
  infosets[1].infoset_id=DDM_INFOSET_ID;
  infosets[2].infoset_id=DDM_INFOSET_ID;
  infosets[3].infoset_id=DDM_THRESHOLD_ID;
  infosets[4].infoset_id=DDM_THRESHOLD_ID;
  infosets[5].infoset_id=OTU_STATUS_INFOSET_ID;
  infosets[6].infoset_id=OTU_SFP_PROPERTY_INFOSET_ID;
  infosets[7].infoset_id=OTU_SFP_PROPERTY_INFOSET_ID;
  infosets[8].infoset_id=OTU_SFP_FEC_INFOSET_ID;
  infosets[9].infoset_id=SFP_ERROR_CODE_TEST_ID;
  //infosets[4].infoset_id=DDM_THRESHOLD_ID;

  //infoset_count=1;
  infosets[0].index=1;     
  infosets[0].infoset_type=1;
  infosets[0].infoset_size=21;
  infosets[0].item_count=21;

  infosets[1].index=1;
  infosets[1].infoset_type=1;
  infosets[1].infoset_size=21;
  infosets[1].item_count=21;
  
  infosets[2].index=2;     
  infosets[2].infoset_type=2;
  infosets[2].infoset_size=20;
  infosets[2].item_count=10;

  infosets[3].index=1;     
  infosets[3].infoset_type=1;
  infosets[3].infoset_size=20;
  infosets[3].item_count=20;
  
  infosets[4].index=2;     
  infosets[4].infoset_type=1;
  infosets[4].infoset_size=20;
  infosets[4].item_count=20;
   
  infosets[5].index=1;     
  infosets[5].infoset_type=1;
  infosets[5].infoset_size=28;
  infosets[5].item_count=28;

  infosets[6].index=1;     
  infosets[6].infoset_type=2;
  infosets[6].infoset_size=32;
  infosets[6].item_count=8;

  infosets[7].index=2;     
  infosets[7].infoset_type=2;
  infosets[7].infoset_size=32;
  infosets[7].item_count=8;

  infosets[8].index=1;
  infosets[8].infoset_type=2;
  infosets[8].infoset_size=32;
  infosets[8].item_count=8;

  infosets[9].index=1;
  infosets[9].infoset_type=2;
  infosets[9].infoset_size=32;
  infosets[9].item_count=8;
   /*
  infosets[4].index=2;     
  infosets[4].infoset_type=2;
  infosets[4].infoset_size=20;
  infosets[4].item_count=10;
   */
  //memcpy(&infosets[0].item_values,&otu,sizeof(otuProperty_t));

  for (n=0;n<MAX_ITEM_SIZE;n++)
  {
     for (m=0;m<OTU_INFOSET_COUNT;m++)
       infosets[m].item_values[n]=0;
  }
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
  //printf("insertOtuInfoSet %d\n",slot);
  if (insertInfoSet(chassis,slot,infosets,OTU_INFOSET_COUNT)!=-1)
  {
    //groupDdmThreshold_t *gddmt=getGroupDdmThreshold(0,slot,1);
    printf("insertOtuInfoSets #%d OK\n",slot);
    otuProperty_t *otu_p;
    otu_p=getOtuProperty(0,slot,1);
    if (NULL!=otu_p)
    {
       //memcpy(otu_p,&otu,sizeof(otuProperty_t));
       //memcpy(&otu_p->channel_property[0],&otu.channel_property[0],sizeof(channel_t));
       otu_p->unit_property.unit_class=otu.unit_property.unit_class;
       otu_p->unit_property.unit_type=otu.unit_property.unit_type;
       otu_p->unit_property.firmware_type=otu.unit_property.firmware_type;
       otu_p->unit_property.hardware_type=otu.unit_property.hardware_type;
       otu_p->unit_property.struct_type=otu.unit_property.struct_type;
 
       otu_p->channel_count=otu.channel_count;
       otu_p->channel_port_count=otu.channel_port_count;
       otu_p->port_type=otu.port_type;
       otu_p->channel_property[0].distance=otu.channel_property[0].distance;
       otu_p->channel_property[0].max_speed=otu.channel_property[0].max_speed;
       strncpy(otu_p->channel_property[0].wave_length,otu.channel_property[0].wave_length,6);
       //printf("#%d distance=%d speed=%d\n",slot,otu.channel_property[0].distance,otu.channel_property[0].max_speed);
    }
    //  memcpy(otu_p,&otu,sizeof(otuProperty_t));
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
   return OTU_INFOSET_COUNT;
  }
  else
    return -1;
}

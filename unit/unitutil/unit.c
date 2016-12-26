#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <unit.h>
#include <infoset.h>
#include <visipc.h>
#include <otu.h>
//gcc -c -o ../vislib/unit.o unit.c -I../include
static status_infoset_t sinfoset;
static status_infoset_ex_t sinfoset_ex;

#ifdef LANG
static const char *bi_item_name[]={"Property","Hardware ver","Firmware ver","Model","SN","Creation","Uptime"};
#else
static const char *bi_item_name[]={"属性","硬件版本","固件版本","型号","系列号","创建日期","运行时间"};
#endif
static const char *unit_name[]={"OTU","OLP","OBP","AWG"};

static const char *unit_title[]={"","MNU","MCU","OTU","OLP","OBP","AWG"};

unitProperty_t *getUnitProperty(uchar_t chassis,uchar_t slot,uchar_t index)
{
  char *values;
  unitProperty_t *unit=NULL;
  if (3!=getUnitClass(chassis,slot))
     return NULL;
  values=getInfoSetValues(chassis,slot,UNIT_PROPERTY_INFOSET_ID,index);
  if (NULL!=values)
  {
      unit=(unitProperty_t*)(values+5);  
  }
  return  unit;
}

int isValidUnitClass(uchar_t uclass,uchar_t utype)
{
   if (SUPPORT_UNIT)
        return 0;
   else
        return -1;
}
const char *getUnitTitle(uchar_t chassis,uchar_t slot)
{
  uchar_t uclass=0;
  uclass=getUnitClass(chassis,slot);
  if (uclass<1)
      return unit_title[0];
  if (uclass>0 && uclass<7)
      return unit_title[uclass];
  else
      return unit_title[0];
}
const char *getBaseInfoName(uchar_t item_id)
{
  if (item_id>7 || item_id<1)
    return NULL;
  const char **p;
  p=bi_item_name;
  //printf("%s\n",p[item_id-1]);
  return p[item_id-1];
}

boolean_t getUnitTypeByName(uchar_t chassis,uchar_t slot,uchar_t *unit_class,uchar_t *unit_type,char *unit_short_name)
{
  *unit_class=UC_OTU;
  *unit_type=1;
  if (unit_short_name!=NULL)
    strcpy(unit_short_name,"OTU");
  return TRUE;
}

boolean_t isValidUnit(unit_base_info_t bi)
{
  if (!strncmp(bi.property,"OTU",3) || !strncmp(bi.property,"OLP",3))
      return TRUE;
  else 
      return FALSE;

}

boolean_t checkUnitClassType(unit_base_info_t bi,uchar_t *unit_class,uchar_t *unit_type)
{
  //printf("%s\n",bi.property);
  boolean_t isvalid=TRUE;
  if (!strncmp(bi.property,"OTU",3))
  {
      *unit_class=3;
      *unit_type=1;
  }
  else if (!strncmp(bi.property,"OLP",3))
  {
      *unit_class=4;
      *unit_type=1;
  }
  else
  {
     *unit_class=127;
     *unit_type=0;
     isvalid=FALSE;
  }
  return isvalid;
}

//boolean_t getUnitPanelInfo(uchar_t chassis,uchar_t slot,unitInfo_t *info)

boolean_t getUnitPanelInfo(uchar_t chassis,uchar_t slot,panelInfo_t *info)
{
  uchar_t n,m,uclass=0,utype=0,count=0,port_count;
  unit_base_info_t *bi;
  status_infoset_t *sis;
  char *p,property[20]="";

  if (TRUE!=getUnitClassType(chassis,slot,&uclass,&utype))
      return FALSE;
  if (uclass<1)
      return FALSE;
  //printf("unit:%d uclass=%d\n",slot,uclass);
  bi=getBaseInfoMap(chassis,slot);
  if (bi==NULL)
     return FALSE;
  switch (uclass)
   {
     case UC_OTU:
       info->group_count=1;
       p=strchr(bi->property,'/');
       while (p!=NULL && info->group_count<MAX_GROUP_COUNT)
       {
         info->group_count++;
         p++;
         p=strchr(p,'/');
      }
       //printf("Unit #%d is OTU %s\n",slot,bi->property);
      info->port_group[0].port_count=bi->property[7]-48;
      info->port_group[0].port_class[0]=bi->property[8]-48;
      info->port_group[0].port_type[0]=1;
      
      if (info->port_group[0].port_count>MAX_GROUP_PORT_COUNT)
          info->port_group[0].port_count=MAX_GROUP_PORT_COUNT;
      for (n=0;n<info->group_count;n++)
      {
          info->port_group[n].port_count=info->port_group[0].port_count;
           
          for (m=0;m<info->port_group[n].port_count;m++)
          {
           info->port_group[n].port_class[m]=info->port_group[0].port_class[0];
           info->port_group[n].port_type[m]=info->port_group[0].port_type[0];
           count++;
           //sis=getStatusInfoSetMap(chassis,slot,OTU_STATUS_INFOSET_ID ,count);
            sis=getInfoSet(chassis,slot,OTU_STATUS_INFOSET_ID,1);
            if (NULL!=sis)
            {
              info->port_group[n].port_status[count-1]=(uchar_t)sis->item_values[0];
              //printf("Unit #%d PType=%d port 1 RX status:%d\n",slot,info->port_type[n],sis->infoitems[0].value);
            }
            else
            {
              //printf("Unit #%d n=%d m=%d sis is NULL\n",slot,n,m);
              return FALSE;
            }
          }
       }
       strcpy(info->unit_name,"OTU");  
    break;
    case UC_OLP:
       info->group_count=1;
       info->port_group[0].port_count=2;
       info->port_group[0].port_class[0]=PC_ADAPTER;
       info->port_group[0].port_class[1]=PC_ADAPTER;
       info->port_group[0].port_type[0]=1;
       info->port_group[0].port_type[1]=1;
       info->port_group[0].port_status[0]=1;
       info->port_group[0].port_status[1]=2;
       strcpy(info->unit_name,"OLP");
       if (bi->property[7]!=2)
           strcpy(info->port_group[0].group_name,"1+1");
       else
           strcpy(info->port_group[0].group_name,"1:1");
       break;
    default:
         return FALSE;
  }
  return TRUE;
}
/*
boolean_t getOtuInfo(uchar_t chassis,uchar_t slot,uchar_t channel,otuInfo_t *info)
{
  uchar_t n,m,uclass=0,utype=0;
  unit_base_info_t *bi;
  status_infoset_t *sis;
  char *p,property[20]="",port_type[8]="";
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
     sis=getStatusInfoSetMap(chassis,slot,OPORT_INFOSET_ID,n+1);
     if (NULL!=sis)
     {
       info->channel[0].rx_status[n]=sis->infoitems[0].value;
       info->channel[0].tx_status[n]=sis->infoitems[1].value;
     }
  }
  return TRUE;
}
*/
/*
status_infoset_t *getStatusInfoSet(uchar_t chassis,uchar_t slot)
{ 
  sinfoset.infoitems[1].value=2;
  sinfoset.infoitems[2].value=1;
  return &sinfoset;   
}
*/
/*
boolean_t getOlp1P1Info(uchar_t chassis,uchar_t slot,olp1StatusInfo_t *info)
{
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
  info->operating_mode=sis->infoitems[1].value;
  info->line=sis->infoitems[2].value;
  info->rule=sis->infoitems[3].value;
  info->ret_mode=sis->infoitems[4].value;
  return TRUE;
}

boolean_t getOlp1P1Data(uchar_t chassis,uchar_t slot,olp1Data_t *info)
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
  
  info->power.rx1_power=sis->infoitems[1].value;
  info->power.rx1_power=info->power.rx1_power<<8;
  info->power.rx1_power|=sis->infoitems[2].value;

  info->power.rx2_power=sis->infoitems[3].value;
  info->power.rx2_power=info->power.rx2_power<<8;
  info->power.rx2_power|=sis->infoitems[4].value;

  info->power.tx_power=sis->infoitems[5].value;
  info->power.tx_power=info->power.tx_power<<8;
  info->power.tx_power|=sis->infoitems[6].value;

  info->power.l1_power=sis->infoitems[7].value;
  info->power.l1_power=info->power.l1_power<<8;
  info->power.l1_power|=sis->infoitems[8].value;

  info->power.l2_power=sis->infoitems[9].value;
  info->power.l2_power=info->power.l2_power<<8;
  info->power.l2_power|=sis->infoitems[10].value;
  
  info->power.tx_alm_power=sis->infoitems[11].value;
  info->power.tx_alm_power=info->power.tx_alm_power<<8;
  info->power.tx_alm_power|=sis->infoitems[12].value;

  info->ret_time=sis->infoitems[13].value;
  info->ret_time=info->ret_time<<8;
  info->ret_time|=sis->infoitems[14].value;

  //printf("info->ret_time=%d\n",info->ret_time);
  return TRUE;
}
*/
int loadUnitInfoset(uchar_t chassis,uchar_t slot,unit_base_info_t bi,message_set_t *mset)
{
  uchar_t unit_class,unit_type;
  status_infoset_t infoset;
  uchar_t *unit_status;
  status_infoset_t *infosets=NULL;
  otuProperty_t *otu_property;
  int n;

  if (checkUnitClassType(bi,&unit_class,&unit_type)!=TRUE || unit_class==127)
  {
   //printf("unit is invalid checkUnitClassType property:%s uc=%d\n",bi.property,unit_class);
   return -1;
  }
//printf("checkUnitClassType property:%s uc=%d OK message_count=%d unit_class=%d unit_type=%d\n",bi.property,unit_class,mset->message_count,unit_class,unit_type);
  
  //return;
  for (n=0;n<MAX_ITEM_COUNT;n++)
      infoset.item_values[n]=2;
  printf("unit #:%d uc=%d ut=%d\n",slot,unit_class,unit_type);
  //deleteSlotNodes(chassis,slot);
  //return 0;
  switch (unit_class)
  {
    case UC_OTU:

          /*
          infoset.infoset_id=OPORT_INFOSET_ID ;
          mset->message[0].infoset_count=2;
          
          infoset.infoset_type=1;
          infoset.infoset_size=2;
          infoset.item_count=2;

          deleteSlotNodes(0,slot);
          for (n=1;n<3;n++)
          {
            infoset.index=n;
            insertStatusInfoSet(chassis,slot,infoset);
            mset->message[0].infoset[n-1].infoset_id=infoset.infoset_id;
            mset->message[0].infoset_type=infoset.infoset_type;
            mset->message[0].infoset[n-1].index=n;
            mset->message[0].infoset[n-1].item_count=2;
          }
          
          infoset.infoset_id=11;
          infoset.infoset_type=2;
          infoset.item_count=4;
          mset->message[1].infoset_type=infoset.infoset_type;
          mset->message[1].infoset_count=2;
          for (n=1;n<3;n++)
          {
            infoset.index=n;
            insertStatusInfoSet(chassis,slot,infoset);
            mset->message[1].infoset[n-1].infoset_id=infoset.infoset_id;
            mset->message[1].infoset[n-1].index=n;
            mset->message[1].infoset[n-1].item_count=4;
          }
          mset->message_count=2;
          */
          insertOtuInfoSet(chassis,slot,unit_type);
          otu_property=getOtuProperty(0,slot,1);
          if (otu_property!=NULL)
          {
            otu_property->unit_property.unit_type=1;
            otu_property->channel_port_count=2;
            otu_property->port_type=1;
            otu_property->channel_property[0].distance=20;
            otu_property->channel_property[0].max_speed=20;
            sprintf(otu_property->channel_property[0].wave_length,"%d",1470+(slot-1)*20);
          }
          mset->message_count=getInfoSets(0,slot,&infosets);
          //mset->message[0].infoset_count=2;
          
          for (n=0;n<mset->message_count;n++)
          {
            if (infosets!=NULL)
            {
               mset->message[n].infoset_type=infosets->infoset_type;
               //mset->message[0].infoset_count=2;
               mset->message[n].infoset[0].infoset_id=infosets->infoset_id;
               mset->message[n].infoset[0].index=infosets->index;
               mset->message[n].infoset[0].item_count=infosets->item_count;
               //memcpy(&mset->message[n],infosets,sizeof(status_infoset_t));
            }
            infosets++;
        }
          break;
    case UC_OLP:
          //printf("load olp infoset\n");
          /*
          infoset.infoset_id=OLP_STATUS_INFOSET_ID ;
          mset->message[0].infoset_count=7;
          
          infoset.infoset_type=1;
          infoset.infoset_size=1;
          infoset.item_count=2;
          deleteSlotNodes(0,slot);
 
          infoset.index=1;
          insertStatusInfoSet(chassis,slot,infoset);

          infoset.infoset_id=OLP_DATA_INFOSET_ID;
          mset->message[0].infoset_count=7;
          
          infoset.infoset_type=2;
          infoset.infoset_size=1;
          infoset.item_count=7;
          insertStatusInfoSet(chassis,slot,infoset);
          */
          insertOlpInfoSet(chassis,slot,unit_type);
          mset->message_count=getInfoSets(0,slot,&infosets);
          //mset->message[0].infoset_count=2;
          for (n=0;n<mset->message_count;n++)
          {
            if (infosets!=NULL)
            {
               mset->message[n].infoset_type=infosets->infoset_type;
               //mset->message[0].infoset_count=2;
               mset->message[n].infoset[0].infoset_id=infosets->infoset_id;
               mset->message[n].infoset[0].index=infosets->index;
               mset->message[n].infoset[0].item_count=infosets->item_count;
               //memcpy(&mset->message[n],infosets,sizeof(status_infoset_t));
            }
            infosets++;
        }
          break;
    default:
         return -1;
  }
  
  unit_status=getUnitStatusMap(chassis);
  if (unit_status!=NULL)
  {
     *(unit_status+slot)=unit_class;
     *(unit_status+MAX_UNIT_COUNT+slot)=unit_type;
  }
  
  return 0;
}

status_infoset_ex_t *getMapStatusInfoSet(uchar_t chassis,uchar_t slot,uchar_t infoset_id)
{
    sinfoset.infoset_id=infoset_id;
    sinfoset_ex.infoset=&sinfoset;
    getUnitType(chassis,slot,&sinfoset_ex.unit_class,&sinfoset_ex.unit_type,NULL);
    getStatusInfoSet(chassis,slot);
   // if (mapStatusInfoSet(chassis,slot,&sinfoset_ex)!=-1)
    //  return &sinfoset_ex;
   // else
      return NULL;
}



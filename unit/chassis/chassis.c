#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unit.h>
#include <infoset.h>
#include <visipc.h>
#include <chassis.h>

chassis_t *getChassisInfo(uchar_t chassis)
{
   char *values;
   chassis_t *cinfo=NULL;
   values=getInfoSetValues(chassis,0,CHASSIS_INFOSET_ID,1);
   if (NULL!=values)
   {
      cinfo=(chassis_t *)(values);
      //printf("getChassisInfo NOT null!\n");  
   }
   else
        printf("getChassisInfo is null!\n");
  /*
   if (cinfo!=NULL)
     printf("cinfo->temp=%d\n",cinfo->temp);
   else
      printf("cinfo-temp=NULL\n");
  */
   return  cinfo;
}

chassisThreshold_t *getChassisThreshold(uchar_t chassis)
{
  char *values;
  chassisThreshold_t *threshold=NULL;
  values=getInfoSetValues(chassis,0,CHASSIS_THRESHOLD_INFOSET_ID,1);
  if (NULL!=values)
   {
      threshold=(chassisThreshold_t *)(values);  
   }
   return threshold;
}
int setChassisThreshold(setInfoSet_t infoset)
{
  char n,m=3,flag;
  char message[MAX_MESSAGE_SIZE];
  short value;
  chassisThreshold_t  *threshold;
  
  threshold=getChassisThreshold(0);
  if (threshold==NULL)
    return -1;
  memset(message,0,MAX_MESSAGE_SIZE*sizeof(char));
  for (n=0;n<MAX_SETTING_ITEM_COUNT;n++)
  { 
    flag=1;
    if (infoset.infoItems[n].item_id<1)
      break;
    switch (infoset.infoItems[n].item_id)
    {
       case MIN_CHASSIS_TEMP:
         threshold->min_temperature=infoset.infoItems[n].item_value;
         value=threshold->min_temperature;
         break;
       case MAX_CHASSIS_TEMP :
         threshold->max_temperature=infoset.infoItems[n].item_value;
         value=threshold->max_temperature;
         break;
       case MIN_PWR_VOLT:
         threshold->min_volt=infoset.infoItems[n].item_value;
         value=threshold->min_volt;
         break;
       case MAX_PWR_VOLT:
         threshold->max_volt=infoset.infoItems[n].item_value;
         value=threshold->max_volt;
         break;
       default:
             flag=0;
    }
   if (flag)
    {
      message[m]=infoset.infoItems[n].item_id;
      //memcpy(&message[m+1],&value,sizeof(char));
      message[m+1]=(value & 0xff00)>>8;
      message[m+2]=value & 0xff;
      m+=3;
    }
  }
   if (m>3)
   {
     message[0]='S';
     message[1]=infoset.infoset_id;
     message[2]=infoset.index;
     sendMessageToUnit(infoset.chassis,infoset.slot,message,infoset.useconds);
   }
  return 0;
}

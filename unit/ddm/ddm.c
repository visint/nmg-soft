#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unit.h>
#include <infoset.h>
#include <visipc.h>
#include <ddm.h>
#include <otu.h>

ddm_t *getPortDdmInfo(uchar_t chassis,uchar_t slot,uchar_t port)
{
  ddm_t *ddm=NULL;
  char *values;
  uchar_t group;
  
  //printf("getChassisInfo!%d\n",DDM_INFOSET_ID);
  if (port<1 || port>8)
  {
   //printf("getChassisInfo port must be 1 to 8!\n");
   return NULL;
  }
  group=(port+1)/2;
  values=getInfoSetValues(chassis,slot,DDM_INFOSET_ID,group);
  if (NULL!=values)
  { 
      ddm=(ddm_t *)(values);
      if (port % 2==0)
       ddm++;
      //printf("getChassisInfo NOT null!\n");  
  }
  else
      printf("getPortDdmInfo is null slot=%d group=%d port%d!\n",slot,group,(port+1)%2);
  return ddm;
}

ddmThreshold_t *getPortDdmThreshold(uchar_t chassis,uchar_t slot,uchar_t index)
{
  ddmThreshold_t *ddmt=NULL;
  char *values;
  
  if (index<1 || index>4)
    return NULL;
  values=getInfoSetValues(chassis,slot,DDM_THRESHOLD_ID,index);
  if (NULL!=values)
   {
      ddmt=(ddmThreshold_t *)(values);
      //printf("getChassisInfo NOT null!\n");  
   }
  return ddmt;
}

groupDdmThreshold_t *getGroupDdmThreshold(uchar_t chassis,uchar_t slot,uchar_t group)
{
  groupDdmThreshold_t *ddmt=NULL;
  char *values;
  
  if (group<1 || group>4)
    return NULL;
  values=getInfoSetValues(chassis,slot,DDM_THRESHOLD_ID,group);
  if (NULL!=values)
   {
      ddmt=(groupDdmThreshold_t *)(values); 
   }
  return ddmt;
}

#include <stdio.h>
#include <typedefs.h>
#include <infoset.h>
#include <olp.h>

int parseMessage(char *message)
{
  #define MAX_SET_ITEM 10

  uchar_t infoset_id;
  uchar_t index;
  uchar_t item_ids[MAX_SET_ITEM]={0}; //存放设置项的信息项ID
  //char    byte_vals[10];
  //short   short_vals[10];
  short item_vals[MAX_SET_ITEM]={0};//存放设置项的信息项的值
  short value;
  int n,m=0;

  infoset_id=(uchar_t)message[1];
  index=(uchar_t)message[2];

  if ('S'==message[0])
  {
    int item_index;
    for (n=0;n<MAX_SET_ITEM;n++)
    {
      int item_idx;
      item_idx=3+3*n;
      if (message[item_idx]!=0) 
      {
       //memcpy(&value,&message[item_idx+1],sizeof(short));
       item_ids[m]=message[item_idx];
       value=message[item_idx+1]<<8;
       //item_vals[m]=value;
       //value=value | message[item_idx+2] & 0xFF;
       item_vals[m]=value | message[item_idx+2] & 0xFF;
       printf("item id=%d item value=%d ",message[item_idx],item_vals[m]);
       m++;
      }
       else //设置项结束
          break;
    }
    //TO DO MESSAGE
  }
  else if ('s'==message[0])
  {
   for (n=0;n<MAX_SET_ITEM;n++)
    {
     if (message[3+2*n]!=0)
     {
        item_ids[n]=message[3+2*n];
        item_vals[n]=message[3+2*n+1];
        printf("%d=%d ",message[3+2*n],message[3+2*n+1]);
     }
     else
       break;
    }
   //TO DO MESSAGE
  }
  else
      return -1;
  value=280;
  printf("\n%x %x\n",(value & 0xff00)>>8,value & 0xff);
  return 0;
}

int main(int argc, char **argv)
{
  char message[MAX_MESSAGE_SIZE]={'S',65,1,OLP1_MODE,0,1,OLP1_LINE,0,2,OLP1_L1_SW_POWER,0XFE,0XE8};
parseMessage(message);
return 0;
}

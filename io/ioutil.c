#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <malloc.h>
#include "sys/types.h"
#include "sys/stat.h"
#include "stdarg.h"
#include "termios.h"
#include "linux/serial.h"

#include <sys/ipc.h>    
#include <sys/sem.h>


#include <chassis.h>
#include <olp.h>

#define TIOCSRS485 0x542F


/****************************************************************
 * Constants
 ****************************************************************/
#define SEMKEY 1235L 

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

#define own_addr_0     32+22   /*PB22*/
#define own_addr_1     32+23   /*PB23*/
#define own_addr_2     32+24   /*PB24*/
#define own_addr_3     32+25    /*PB25*/

#define bp_int_line    64+15   /*PC15*/
#define reset_bp_cpu   32+20   /*PB20*/

#define led_pwr1        32+1   /*PB1*/
#define led_pwr2        32+3   /*Pb3*/
#define led_fan_green   32     /*PB0*/
#define led_fan_red     32+2   /*PB2*/
#define led_run_green   28     /*PA28*/
#define led_run_red     29     /*PA29*/
#define led_board       25     /*PA25*/

#define setup_require    4     /*PA4*/
#define setup_respone    5     /*PA5*/

//low level communication command
#define cmd_get_fix_info      0x01  //获取板卡固定信息
#define cmd_get_runtime       0x02  //获取板卡动态信息
#define cmd_get_alarm         0x03  //获取板卡告警信息

#define cmd_bp_valid_info     0x04

#define cmd_set_runtime_1 0x05  //设置信息集，预留7个
#define cmd_set_runtime_2 0x06
#define cmd_set_runtime_3 0x07
#define cmd_set_runtime_4 0x08
#define cmd_set_runtime_5 0x09
#define cmd_set_runtime_6 0x0a
#define cmd_set_runtime_7 0x0b

#define cmd_set_fix_para1 0x20  //设置固定信息，预留11条
#define cmd_set_fix_para2 0x21
#define cmd_set_fix_para3 0x22
#define cmd_set_fix_para4 0x23
#define cmd_set_fix_para5 0x24
#define cmd_set_fix_para6 0x25
#define cmd_set_fix_para7 0x26
#define cmd_set_fix_para8 0x27
#define cmd_set_fix_para9 0x28
#define cmd_set_fix_para10 0x29
#define cmd_set_fix_para11 0x2a


#define cmd_update_start 0x80
#define cmd_update_proc  0x81
#define cmd_update_end   0x82

#define BP_ADDR             17 //背板CPU  RS485地址
#define master_nmu_addr     0
#define slave_nmu_addr      15

#define RS485_TX_LEN  512    //发送长度
#define RS485_RX_LEN  512    //接收长度

#define led_green_state    1 //LED 绿色
#define led_red_state      2 //LED 红色
#define led_off_state      3 //LED 熄灭

#define led_pwr1_num   1  //LED 序号
#define led_pwr2_num   2
#define led_fan_num    3
#define led_run_num    4

//#define DEBUG_4U

//backplane_info_t bp_runtime_info_temp;
//chassis_all_t chassis_info;

//unsigned char nmu_own_addr=0xff;
//unsigned char first_power_on=1; //识别初次上电
//int rs485_comm_error=0;
//key_t semkey_ttys1;    
int     semid_ttys1;

/****************************************************************
 * RS485发送校验
 ****************************************************************/
unsigned char tx_crc_calc(char *p_buff,int len)
{
    unsigned char result=0;
    int i;
    for(i=0; i<len; i++)result+=p_buff[i];
    return (0-result);
}

/****************************************************************
 * RS485接收校验
 ****************************************************************/
unsigned char rx_crc_calc(char *p_buff,int len)
{
    unsigned char result=0;
    int i;
    for(i=0; i<len; i++)result+=p_buff[i];
    return result;
}

int get_sem_tty(void)
{   
//semkey_ttys1=ftok("/proc",101); 
    semid_ttys1=semget(SEMKEY,1,0666);    
    if(semid_ttys1==-1)  return EXIT_FAILURE;	
    else return EXIT_SUCCESS;
}
void p()	
   {	
	   struct sembuf sem_p;    
	   sem_p.sem_num=0;    
	   sem_p.sem_op=-1;    
	   sem_p.sem_flg=0;
	   //printf("start p operation \n");    
	   if(semop(semid_ttys1,&sem_p,1)==-1)	
	   printf("p operation is fail\n"); 		   
   }	
   
   /*\u4fe1\u53f7\u91cf\u7684V\u64cd\u4f5c*/	
   void v()    
   {	
	   struct sembuf sem_v;    
	   sem_v.sem_num=0;    
	   sem_v.sem_op=1;	
	   sem_v.sem_flg=0;
	   //printf("start v operation \n");    
	   if(semop(semid_ttys1,&sem_v,1)==-1)	
	   printf("v operation is fail\n");    
   }


/****************************************************************
 * 板卡数据读写函数:
 参数1，card_addr:      板卡地址
 参数2，access_cmd:   访问命令
 参数3，send_buff       发送缓冲区地址
 参数4，send_len         发送缓冲区长度
 参数5，receive_buff     接收缓冲区地址
 参数6，receive_len       接收缓冲区长度
 ****************************************************************/
 
int card_access(unsigned char card_addr,int access_cmd, void *send_buff, int send_len,void * receive_buff,int receive_len)
{
    char txBuffer[1024];
    char rxBuffer[1024];
    int fd;
    int i;
    int rev_count;
    struct termios tty_attributes;
    struct serial_rs485 rs485conf;
    txBuffer[0]=card_addr;
    txBuffer[1]=access_cmd;
    memcpy(txBuffer+2,send_buff,send_len);
    //printf("bre-p\n");
    p();
    //printf("client-p\n");
    if ((fd = open("/dev/ttyS1",O_RDWR|O_NOCTTY|O_NONBLOCK))<0)
    {
        fprintf (stderr,"Open error on %s\n", strerror(errno));
		  close(fd);
	v();
        return EXIT_FAILURE;
    }
    else
    {
        tcgetattr(fd,&tty_attributes);

        // c_cflag
        // Enable receiver
        tty_attributes.c_cflag |= CREAD;

        // 8 data bit
        tty_attributes.c_cflag |= CS8;

        // c_iflag
        // Ignore framing errors and parity errors.
        tty_attributes.c_iflag |= IGNPAR;
        tty_attributes.c_iflag &=~(INLCR|ICRNL|IGNCR|IXON);

        tty_attributes.c_oflag&=~(ONLCR|OCRNL|ONOCR|ONLRET);

        // c_lflag
        // DISABLE canonical mode.
        // Disables the special characters EOF, EOL, EOL2,
        // ERASE, KILL, LNEXT, REPRINT, STATUS, and WERASE, and buffers by lines.

        // DISABLE this: Echo input characters.
        tty_attributes.c_lflag &= ~(ICANON);

        tty_attributes.c_lflag &= ~(ECHO);

        // DISABLE this: If ICANON is also set, the ERASE character erases the preceding input
        // character, and WERASE erases the preceding word.
        tty_attributes.c_lflag &= ~(ECHOE);

        // DISABLE this: When any of the characters INTR, QUIT, SUSP, or DSUSP are received, generate the corresponding signal.
        tty_attributes.c_lflag &= ~(ISIG);

        // Minimum number of characters for non-canonical read.
        tty_attributes.c_cc[VMIN]=1;

        // Timeout in deciseconds for non-canonical read.
        tty_attributes.c_cc[VTIME]=0;

        // Set the baud rate
        cfsetospeed(&tty_attributes,B115200);
        cfsetispeed(&tty_attributes,B115200);

        tcsetattr(fd, TCSANOW, &tty_attributes);
        // Set RS485 mode:
        rs485conf.flags |= SER_RS485_ENABLED;
        if (ioctl (fd, TIOCSRS485, &rs485conf) < 0)
        {
            printf("ioctl error\n");
        }
        txBuffer[RS485_TX_LEN-1]=tx_crc_calc(txBuffer,RS485_TX_LEN-1);
        usleep(10000);
        write(fd,txBuffer,RS485_TX_LEN);
        usleep(220000);
        rev_count=read(fd,rxBuffer,RS485_RX_LEN);
        if((rev_count==(RS485_RX_LEN))   )
        {

            if(rx_crc_calc(rxBuffer,(RS485_RX_LEN))==0)
            {
                memcpy(receive_buff,(rxBuffer+1),receive_len);
            }
            else
            {
                close(fd);
	        v();
                printf("rx_crc_calc error!\n");
                return EXIT_FAILURE;
            }

        }
        else
        {
            close(fd);
	     v();
            printf("rev_count!=(RS485_RX_LEN)!\n");
            return EXIT_FAILURE;
        }

    }

    close(fd);
     v();
    return EXIT_SUCCESS;
}

int ioSet(uchar_t classis,uchar_t slot,uchar_t *unit_class, uchar_t op_cmd)
{
#define MAX_ERR_COUNT 5

int access_result,error_count;
unsigned char rev_tmp[1024];
if(get_sem_tty()==EXIT_FAILURE)
{
 printf("Set #%d fail!\n",slot+1);
 return EXIT_FAILURE;
}
if(classis==0){
io_set_retry:	
      access_result=card_access(slot,op_cmd,unit_class,500,rev_tmp,500);
      if(access_result==EXIT_FAILURE)
      {   
          error_count++;
          if(error_count>MAX_ERR_COUNT)return EXIT_FAILURE;
             usleep(220000);
          printf("io_set failed  retry\n");
             goto io_set_retry;
      }
      printf("\nSet #%d OK!\n",slot+1);
      return access_result;
   }
else
{//exteral classis
   printf("ioSet #%d fail!\n",slot+1);
   return EXIT_FAILURE;
}
}

/*
int ioGet(uchar_t classis,uchar_t slot,uchar_t *unit_class, uchar_t op_cmd)
{
int access_result;
unsigned char tx_tmp[1024];
if(get_sem_tty()==EXIT_FAILURE){printf("no sem\n");return EXIT_FAILURE;}
if(classis==0){
      access_result=card_access(slot+1,op_cmd,tx_tmp,500,unit_class,500);
      return access_result;
   }
else
   {//exteral classis

   return EXIT_FAILURE;
   }
}
*/
int sendMessageToUnit(uchar_t chassis,uchar_t slot,char message[],unsigned int useconds)
{
  int n;
  printf("sendMessageToUnit flag=%c\n",message[0]);
  for (n=0;n<32;n++)
   printf("0x%0x,",message[n]);
  printf("\n");
  //parseMessage(message);
  ioSet(chassis,slot,message, cmd_set_runtime_1);
}


/****************************************************************
 * Main
 ****************************************************************/
/*
int main(void)
{
uchar_t rx_tmp[1024];

get(0,13,rx_tmp,cmd_get_fix_info);

return 0;
}
*/


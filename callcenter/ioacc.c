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


#include <visipc.h>
#include <chassis.h>
#include <olp.h>
#include <otu.h>
#include <ddm.h>
#define TIOCSRS485 0x542F

//arm-linux-gcc -o ioacc  ioacc.c -I../include -L=./ -L/home/visint/src/arm/lib -lvispace -lsqlite3 -liconv

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
#define CMD_GET_FIX_INFO      0x01  //获取板卡固定信息
#define CMD_GET_RUN_TIME       0x02  //获取板卡动态信息
#define CMD_GET_ALARM         0x03  //获取板卡告警信息

#define cmd_bp_valid_info     0x04

#define CMD_GET_RUN_TIME1 0x05  //设置信息集，预留7个
#define CMD_GET_RUN_TIME2 0x06
#define CMD_GET_RUN_TIME3 0x07
#define CMD_GET_RUN_TIME4 0x08
#define CMD_GET_RUN_TIME5 0x09
#define CMD_GET_RUN_TIME6 0x0a
#define CMD_GET_RUN_TIME7 0x0b

#define CMD_SET_FIX_INFO1 0x20  //设置固定信息，预留11条
#define CMD_SET_FIX_INFO2 0x21
#define CMD_SET_FIX_INFO3 0x22
#define CMD_SET_FIX_INFO4 0x23
#define CMD_SET_FIX_INFO5 0x24
#define CMD_SET_FIX_INFO6 0x25
#define CMD_SET_FIX_INFO7 0x26
#define CMD_SET_FIX_INFO8 0x27
#define CMD_SET_FIX_INFO9 0x28
#define CMD_SET_FIX_INFO10 0x29
#define CMD_SET_FIX_INFO11 0x2a


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

#define SAVE_TO_SHM 1
backplane_info_t bp_runtime_info_temp;
chassis_all_t chassis_info;

unsigned char nmu_own_addr=0xff;
unsigned char first_power_on=1; //识别初次上电
int rs485_comm_error=0;
key_t semkey_ttys1;
int semid_ttys1;


/****************************************************************
 * gpio_export
 ****************************************************************/
int gpio_export(unsigned int gpio)
{
    int fd, len;
    char buf[MAX_BUF];

    fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
    if (fd < 0)
    {
        perror("gpio/export");
        return fd;
    }

    len = snprintf(buf, sizeof(buf), "%d", gpio);
    write(fd, buf, len);
    close(fd);

    return 0;
}

/****************************************************************
 * gpio_unexport
 ****************************************************************/
int gpio_unexport(unsigned int gpio)
{
    int fd, len;
    char buf[MAX_BUF];

    fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
    if (fd < 0)
    {
        perror("gpio/export");
        return fd;
    }

    len = snprintf(buf, sizeof(buf), "%d", gpio);
    write(fd, buf, len);
    close(fd);
    return 0;
}

/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0)
    {
        perror("gpio/direction");
        return fd;
    }

    if (out_flag)
        write(fd, "out", 4);
    else
        write(fd, "in", 3);

    close(fd);
    return 0;
}

/****************************************************************
 * gpio_set_value
 ****************************************************************/
int gpio_set_value(unsigned int gpio, unsigned int value)
{
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0)
    {
        perror("gpio/set-value");
        return fd;
    }

    if (value)
        write(fd, "1", 2);
    else
        write(fd, "0", 2);

    close(fd);
    return 0;
}

/****************************************************************
 * gpio_get_value
 ****************************************************************/
int gpio_get_value(unsigned int gpio, unsigned int *value)
{
    int fd, len;
    char buf[MAX_BUF];
    char ch;

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_RDONLY);
    if (fd < 0)
    {
        perror("gpio/get-value");
        return fd;
    }

    read(fd, &ch, 1);

    if (ch != '0')
    {
        *value = 1;
    }
    else
    {
        *value = 0;
    }

    close(fd);
    return 0;
}

/****************************************************************
 * gpio_set_edge
 ****************************************************************/

int gpio_set_edge(unsigned int gpio, char *edge)
{
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0)
    {
        perror("gpio/set-edge");
        return fd;
    }

    write(fd, edge, strlen(edge) + 1);
    close(fd);
    return 0;
}

/****************************************************************
 * gpio_fd_open
 ****************************************************************/

int gpio_fd_open(unsigned int gpio)
{
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_RDONLY | O_NONBLOCK );
    if (fd < 0)
    {
        perror("gpio/fd_open");
    }
    return fd;
}

/****************************************************************
 * gpio_fd_close
 ****************************************************************/

int gpio_fd_close(int fd)
{
    return close(fd);
}


/****************************************************************
 * 获取网管卡本身地址
 ****************************************************************/
unsigned char get_own_addr(void)
{
    int tmp,val;

    gpio_export(own_addr_0);
    gpio_export(own_addr_1);
    gpio_export(own_addr_2);
    gpio_export(own_addr_3);
//set input
    gpio_set_dir(own_addr_0, 0);
    gpio_set_dir(own_addr_1, 0);
    gpio_set_dir(own_addr_2, 0);
    gpio_set_dir(own_addr_3, 0);
//get own  card number
    gpio_get_value(own_addr_0,&tmp);
    val|=tmp;
    gpio_get_value(own_addr_0,&tmp);
    val|=(tmp<<1);
    gpio_get_value(own_addr_0,&tmp);
    val|=(tmp<<2);
    gpio_get_value(own_addr_0,&tmp);
    val|=(tmp<<3);
    return (unsigned char)val;
}



/****************************************************************
 * LED 初始化
 ****************************************************************/

void led_init(void)
{
    gpio_export(led_pwr1);
    gpio_export(led_pwr2);
    gpio_export(led_fan_green);
    gpio_export(led_fan_red);
    gpio_export(led_run_green);
    gpio_export(led_run_red);
    gpio_export(setup_require);

    gpio_set_dir(led_pwr1, 1);
    gpio_set_dir(led_pwr2, 1);
    gpio_set_dir(led_fan_green, 1);
    gpio_set_dir(led_fan_red, 1);
    gpio_set_dir(led_run_green, 1);
    gpio_set_dir(led_run_red, 1);
    gpio_set_dir(setup_require, 1);

//all led off
    gpio_set_value(led_pwr1,1);
    gpio_set_value(led_pwr2,1);
    gpio_set_value(led_fan_green,0);
    gpio_set_value(led_fan_red,0);
    gpio_set_value(led_run_green,0);
    gpio_set_value(led_run_red,0);

    gpio_set_value(setup_require,0);

}


/****************************************************************
 * LED 操作，参数1为LED序号，参数2为LED颜色状态
 ****************************************************************/

void led_control(unsigned char led_num ,unsigned char state)
{
    switch(led_num)
    {
    case led_pwr1_num:

        switch(state)
        {
        case  led_green_state:
            gpio_set_value(led_pwr1,0);
            break;
        case led_off_state:
            gpio_set_value(led_pwr1,1);
            break;
        default:
            break;
        }
        break;
    case led_pwr2_num:

        switch(state)
        {
        case  led_green_state:
            gpio_set_value(led_pwr2,0);
            break;
        case led_off_state:
            gpio_set_value(led_pwr2,1);
            break;
        default:
            break;
        }
        break;
    case led_fan_num:
        switch(state)
        {
        case  led_green_state:
            gpio_set_value(led_fan_green,1);
            gpio_set_value(led_fan_red,0);

            break;
        case  led_red_state:
            gpio_set_value(led_fan_green,0);
            gpio_set_value(led_fan_red,1);

            break;

        case led_off_state:
            gpio_set_value(led_fan_green,0);
            gpio_set_value(led_fan_red,0);

            break;
        default:
            break;
        }
        break;
    case led_run_num:
        switch(state)
        {
        case  led_green_state:
            gpio_set_value(led_run_green,1);
            gpio_set_value(led_run_red,0);

            break;
        case  led_red_state:
            gpio_set_value(led_run_green,0);
            gpio_set_value(led_run_red,1);

            break;

        case led_off_state:
            gpio_set_value(led_run_green,0);
            gpio_set_value(led_run_red,0);

            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
}



/****************************************************************
 * RS485发送校验
 ****************************************************************/
/*
unsigned char tx_crc_calc(char *p_buff,int len)
{
    unsigned char result=0;
    int i;
    for(i=0; i<len; i++)result+=p_buff[i];
    return (0-result);
}
*/
/****************************************************************
 * RS485接收校验
 ****************************************************************/
/*
unsigned char rx_crc_calc(char *p_buff,int len)
{
    unsigned char result=0;
    int i;
    for(i=0; i<len; i++)result+=p_buff[i];
    return result;
}
*/

void sem_tty_init(void)
{
    semkey_ttys1=ftok("/proc",101);
    semid_ttys1=semget(SEMKEY,1,0666|IPC_CREAT);
    if(semid_ttys1==-1)   
      printf("creat sem is fail\n");

    union semun
    {
        int val;
        struct semid_ds *buf;
        ushort *array;
    } sem_u;

    sem_u.val=1;
    semctl(semid_ttys1,0,SETVAL,sem_u);

}
/*
void p()
   {
	   struct sembuf sem_p;
	   sem_p.sem_num=0;
	   sem_p.sem_op=-1;
	      printf("serve_p_op\n");
	   if(semop(semid_ttys1,&sem_p,1)==-1)
	   printf("p operation is fail\n");
   }
*/
/*\u4fe1\u53f7\u91cf\u7684V\u64cd\u4f5c*/
/*  void v()
  {
   struct sembuf sem_v;
   sem_v.sem_num=0;
   sem_v.sem_op=1;

   printf("serve_v_op\n");
   if(semop(semid_ttys1,&sem_v,1)==-1)
   printf("v operation is fail\n");
  }
*/

/****************************************************************
 * 板卡数据读写函数:
 参数1，card_addr:      板卡地址
 参数2，access_cmd:   访问命令
 参数3，send_buff       发送缓冲区地址
 参数4，send_len         发送缓冲区长度
 参数5，receive_buff     接收缓冲区地址
 参数6，receive_len       接收缓冲区长度
 ****************************************************************/
/*
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
 // printf("haha-bre-p\n");

    p();
	printf("server-p\n");

    if ((fd = open("/dev/ttyS1",O_RDWR|O_NOCTTY|O_NONBLOCK))<0)
    {
        fprintf (stderr,"Open error on %s\n", strerror(errno));
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
                return EXIT_FAILURE;
            }

        }
        else
        {
            close(fd);
	     v();
            return EXIT_FAILURE;
        }

    }

    close(fd);
     v();
    return EXIT_SUCCESS;
}
*/


/****************************************************************
 * 中断 初始化
 ****************************************************************/

int interrupt_init(int pin_num)
{
    int int_fd;
    gpio_export(pin_num);
    gpio_set_dir(pin_num, 0);
    gpio_set_edge(pin_num, "both");
    int_fd = gpio_fd_open(pin_num);
    return int_fd;
}

/**************************************************************
【功能】      插入单元盘
【参数说明】
            chassis      机箱号（0-MAX_CHASSIS_COUNT）,0为主机箱
            uchar_t      slot 槽位号（0-MAX_UNIT_COUNT）,0为背板（0-MAX_UNIT_COUNT）
【返回值说明】
【提示】
***************************************************************/
static void insertUnit(char chassis,char slot,enum UnitClass uclass,uchar_t uType,unit_base_info_t bi,char send_trap)
{
    struct timeval tpstart;
    unit_base_info_t *pbi;
    uchar_t *us_shm;
    char property[32]= {0};
    olp1P1Info_t *olp;
    char *values;
    if (uclass<1)
       return;
    us_shm=getUnitStatusMap(0);
    if (NULL==us_shm)
        return;

    pbi=getBaseInfoMap(0,slot);
    if (NULL==pbi)
        return;
        
    //memcpy(pbi,&bi,sizeof(unit_base_info_t));
    //gettimeofday(&tpstart,NULL);
    //pbi->uptime=tpstart.tv_sec;
    property[0]=uclass;
    property[1]=1;
    property[2]=1;
    property[3]=1;
    property[4]=1;
    loadInfosets(0,slot,property);
    *(us_shm+slot)=uclass;
    if (send_trap)
        sendSnmpTrap(52,0,slot,0);
    values=getInfoSetValues(chassis,slot,DDM_INFOSET_ID,1);
    if (NULL==values)
    {
      printf("ddm is null\n");
      return;
    }
    else
        printf("ddm is OK\n");
   /*
    olp=getOlp1P1Info(0,slot);
    if (NULL==olp)
    {
      printf("Olp is null\n");
      return;
    }
    
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
    */
    //printf("ret_mode=%d\n",olp->ret_mode);
    //printf("\nSlot:%d mode=%d line=%d rule=%d ret_mode=%d ret_time=%d rx1=%d rx2=%d tx=%d l1=%d l2=%d alm=%d\n",slot,olp->mode,olp->line,olp->rule,olp->ret_mode,olp->ret_time,olp->rx1_power,olp->rx2_power,olp->tx_power,olp->l1_power,olp->l2_power,olp->tx_alm_power);
}

static void removeUnit(char chassis,char slot)
{
    uchar_t *us_shm;
    char property[32]= {0};

    us_shm=getUnitStatusMap(0);
    if (NULL==us_shm)
        return;
    *(us_shm+slot)=0;
    printf("Unit %d removed\n",slot);
    sendSnmpTrap(51,0,slot,0);
}

void initSys()
{
    clearShm(0);
    insertBpuInfoSet(0);
}

int io_get(uchar_t slot,uchar_t *pData_get, uchar_t op_cmd)
{
#define MAX_ERR_COUNT 5
    int access_result,error_count=0;
    unsigned char tx_tmp[1024];
io_read_retry:
    access_result=card_access(slot,op_cmd,tx_tmp,500,pData_get,500);
    if(access_result==EXIT_FAILURE)
    {
        error_count++;
        if(error_count>MAX_ERR_COUNT)
            return EXIT_FAILURE;
        usleep(220000);
        printf("io_get #%d fail\n",slot);
        goto io_read_retry;
    }

    return access_result;
}

#define	ALARM_INFOSET_ID  99


/*
get message real length
*/
int get_dataset_length(uchar_t infoset_id)
{
    switch (infoset_id)
    {
    case OLP_STATUS_INFOSET_ID:
        return sizeof(olp1P1Info_t);
        break;
    case  ALARM_INFOSET_ID :
        //return (sizeof(alarm_item)*10);
        
        return 40;
        break;
    default :
        return 0;
        break;
    }

}
/*
|count  2Byte|id |index | len 2Byte | message |   id   | index | len |message |
*/
#define MAX_DATASET_COUNT 10
int get_parseMessage(char *message,uchar_t slot)
{
    uchar_t infoset_count,infoset_id,infoset_index,infoset_len;
    uchar_t *pDataSet;
    char test_copyto[MAX_DATASET_COUNT][500];//for test only
    int m=0;
    int test=0;
    infoset_count=(short )(*message); //get message count
    pDataSet=message+6;  //first message position
    for(; m < infoset_count; m++)
    {   short len=(short)(*(pDataSet-2));
        printf("infoset_count=%d m=%d infoset_id=%d infoset_size=%d\n",infoset_count,m,*(pDataSet-4),len);
        memcpy(	test_copyto[m],pDataSet,get_dataset_length(*(pDataSet-4)));
        pDataSet =pDataSet+ ((short)*(pDataSet-2))+4; //next message start position
    }
//#ifdef DEBUG_4U
    //for debug
    olp1P1Info_t  *p_olp=(olp1P1Info_t  *)test_copyto[0];
    printf("\nret time= %d",p_olp->ret_time);
    printf("\nrx1= %d",p_olp->rx1_power);
    for(; test<28; test++)
        printf("\n%2x,",test_copyto[0][test]);
//#endif
}
/*
static short exchange(short value)
{
  printf("%0x H:%0x-L:%0x\n",0xffff & value,(value & 0XFF00)>>8,value & 0XFF);
  return (value & 0xff00)>>8 | ((value & 0xff)<<8);
}
*/
//|count  2Byte|id |index | len 2Byte | message |   id   | index | len |message |
static void saveMessageToShm(uchar_t chassis,uchar_t slot,char *message)
{
  uchar_t infoset_count,index;
  uchar_t *pInfoSet;
  short pInfoSet_size,infoset_id,val;
  int n,m;
  char *values,*v;
  infoset_count=(short )(*message);
  if (infoset_count>10)
      infoset_count=10;
  pInfoSet=message+2;  //first message position
  olp1P1Info_t *olp,*olp1,*olp2;
  for (n=0;n<132;n++)
  {
    printf("%d:%0x|",n,*(message+n));
    if (n%32==0)
     printf("\n");
 }
  for(m=0; m < infoset_count; m++)
  {
     //memcpy(test_copyto[m],n,get_dataset_length(*(n-4)));
     infoset_id=*(pInfoSet);
     pInfoSet_size=(short)(*(pInfoSet+2));
     if (pInfoSet_size>32)
       pInfoSet_size=32;
     index=*(pInfoSet+1);
     printf("infoset_id=%d index=%d size=%d\n",infoset_id,index,pInfoSet_size);
     values=getInfoSetValues(0,slot,infoset_id,index);
     if (values!=NULL)
     {
        char buf[32],buf2[32];
        printf("infoset_id=%d index=%d found!\n",infoset_id,index);
        if (pInfoSet_size>32)
           pInfoSet_size=32;
        memcpy(values,pInfoSet+4,pInfoSet_size);
        for (n=0;n<pInfoSet_size;n++)
           printf("%d:%x\n",n,values[n]);
       /* for (n=0;n<28;n++)
          *(values+n)=*(pInfoSet+n);
        memcpy(buf,values,pInfoSet_size);
        */
        if (DDM_INFOSET_ID==infoset_id)
        {
            ddm_t *ddm;
            ddm=getPortDdmInfo(0,slot,index);
            if (NULL!=ddm)
            { 
              //for (n=0;n<pInfoSet_size;n++)
              //  printf("%d:%x\n",n,values[n]);
              printf("rxPower=%d txPower=%d rxVolt=%d Bias=%d temperature=%d\n",ddm->rxPower,ddm->txPower,ddm->rxVolt,ddm->rxBiasCurrent,ddm->temperature);
           }
        }
        if (OLP_STATUS_INFOSET_ID==infoset_id)
        {
           olp=getOlp1P1Info(0,slot);
           /*olp2=(olp1P1Info_t*)(buf);
           if (olp!=NULL)
          {
           
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
}
           return;
           //memcpy(olp,buf,pInfoSet_size);
           //val=olp->ret_time;
           //printf("%0x:%0x\n",(val & 0xff00)>>8,((val & 0xff)));

           exchange(olp->ret_time);
           exchange(olp->rx1_power);
           exchange(olp->rx2_power);
           exchange(olp->tx_power);
           exchange(olp->l1_power);
           exchange(olp->l2_power);
           exchange(olp->tx_alm_power);

           olp->ret_time=exchange(olp->ret_time);
           olp->rx1_power=exchange(olp->rx1_power);
           olp->rx2_power=exchange(olp->rx2_power);
           olp->tx_power=exchange(olp->tx_power);
           olp->l1_power=exchange(olp->l1_power);
           olp->l2_power=exchange(olp->l2_power);
           olp->tx_alm_power=exchange(olp->tx_alm_power);
           
           
           printf("ret_time=%d infoset_id=%d\n",olp->ret_time,infoset_id);
        printf("mode=%d line=%d rule=%d ret_mode=%d rx1=%d rx2=%d tx=%d l1=%d l2=%d alm=%d\n",olp->mode,olp->line,olp->rule,olp->ret_mode,olp->rx1_power,olp->rx2_power,olp->tx_power,olp->l1_power,olp->l2_power,olp->tx_power);
*/
        }
        
        
        /*v=(char*)olp;
        for (n=0;n<pInfoSet_size;n++)
        {
         //*(values+n)=*(pInfoSet+n);
         //printf("%d:%0x-%0x\n",n+1,*(pInfoSet+n),*(values+n));
         printf("[%d] %0x=%0x\n",n,*(v+n),*(buf+n));
        }
        */
       // printf("ret_time2=%d\n",olp->ret_time);
        /*
        olp2=(olp1P1Info_t*)buf;
        memcpy(olp,olp2,sizeof(olp1P1Info_t));
        printf("mode=%d line=%d rule=%d ret_mode=%d rx1=%d rx2=%d tx=%d l1=%d l2=%d alm=%d\n",olp2->mode,olp2->line,olp2->rule,olp2->ret_mode,olp2->rx1_power,olp2->rx2_power,olp2->tx_power,olp2->l1_power,olp2->l2_power,olp2->tx_power);
        */
        // for (n=0;n<pInfoSet_size;n++)
        //    printf("%d:%0x-%0x\n",n+1,*(pInfoSet+n),*(buf+n));
        //printf();
              
     }
     //pInfoSet =pInfoSet+ ((short)*(pInfoSet-2))+4; //next message start position
       pInfoSet =pInfoSet+pInfoSet_size+4;
  }
}
/*

修改下面结构体

typedef struct {
backplane_info_t 	bp_runtime_info;
card_fix_Info_t        card_fix_info[MAX_CARD_COUNT];
char                         runtime_info[MAX_CARD_COUNT][500];// modify to fix length
}chassis_all_t;



typedef struct
{
  uchar_t  mode;
  uchar_t  line;
  uchar_t  rule; 
  uchar_t  ret_mode;
  short ret_time; 
  short rx1_power;
  short rx2_power; 
  short tx_power; 
  short l1_power;
  short l2_power;
  short tx_alm_power;
  uchar_t rx1_led;
  uchar_t rx2_led;
  uchar_t tx_led;
  //add the following items
  uchar_t rx1_wave_length;
  uchar_t rx2_wave_length;
  uchar_t tx_wave_length;
  uchar_t data_rev[4];
}olp1P1Info_t;

//classis.h
typedef struct
{
uchar_t Card_Type;
uchar_t Card_SubType;
uchar_t Card_hardware;
uchar_t Card_software;
uchar_t Card_mech;
char property[30];
char sn[30];
char model[30];
char creation[30];
char fwver[30];
char hwver[30];
char Card_parm7[30];
}card_fix_Info_t;



typedef struct
{
short    alarm_code;
uchar_t  sub_index;
uchar_t  sub_val;
}alarm_item;

alarm_item olp_alarm[10];  //10 message  ,4bytes/message


*/

void insertOlp(short slot)
{
  char n;
    unit_base_info_t *pbi;
    char property[32]={0};
    olp1P1Info_t *olp;

    uchar_t *us_shm;
    

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
    
    olp=getOlp1P1Info(0,slot);
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
    printf("\nSlot=%d\nmode=%d line=%d rule=%d ret_mode=%d ret_time=%d rx1=%d rx2=%d tx=%d l1=%d l2=%d alm=%d\n",slot,olp->mode,olp->line,olp->rule,olp->ret_mode,olp->ret_time,olp->rx1_power,olp->rx2_power,olp->tx_power,olp->l1_power,olp->l2_power,olp->tx_alm_power);
  
}
int main1(void)
{
  unit_base_info_t binfo;
  initSys();
  insertOlp(5);
    //insertOlp(7);
  insertOlp(8);
  //insertUnit(0,5,UC_OLP,1,binfo,0);
  //insertUnit(0,8,UC_OLP,1,binfo,0);
}

int main(void)
{
    struct pollfd fdset[2];
    int nfds = 2;
    int int_bp_fd, int_setup_respone,timeout, rc;
    char buf[MAX_BUF],send_tmp_buff[500],rev_tmp_buff[500];
    int len,i,n,access_result;
    enum UnitClass dev_type;
    int io_access_err=0;
    unit_base_info_t *bi,binfo;
    uchar_t *us_shm;
    char uclass=0;

    #ifdef SAVE_TO_SHM
    initSys();
    us_shm=getUnitStatusMap(0);
    
    if (NULL==us_shm)
        return 0;
    #endif
    sem_tty_init	();
    nmu_own_addr=get_own_addr();
    led_init();                                 //初始化LED
    int_bp_fd = interrupt_init(bp_int_line);    //初始化背板CPU中断线
    timeout = POLL_TIMEOUT;
    memset(&chassis_info,0,sizeof(chassis_all_t));
    
    while (1)
    {
        //insertUnit(0,7,UC_OLP,1,binfo,0);
        memset((void*)fdset, 0, sizeof(fdset));
        
        fdset[1].fd = int_bp_fd;
        fdset[1].events = POLLPRI;
        
        rc = poll(fdset, nfds, timeout);
        //insertUnit(0,7,UC_OLP,1,binfo,0);
        if (fdset[1].revents & POLLPRI)  //背板中断发生
        {
            char message[512]={0};
            //insertUnit(0,7,UC_OLP,1,binfo,0);
            len = read(fdset[1].fd, buf, MAX_BUF);//清除中断标志
            
            if(first_power_on==1)//第一次上电读取全部信息
            {
                first_power_on=0;
                //insertUnit(0,7,UC_OLP,1,binfo,0);
                io_get( BP_ADDR,rev_tmp_buff, CMD_GET_RUN_TIME);
                memcpy(&bp_runtime_info_temp,rev_tmp_buff,sizeof(backplane_info_t));
                memcpy(&(chassis_info.bp_runtime_info),&bp_runtime_info_temp,sizeof(backplane_info_t));
                //for (i=0;i<15;i++)
                 // printf("%d ,",bp_runtime_info_temp.bp_exist[i]);
                //insertUnit(0,8,UC_OLP,1,binfo,0);
                for(i=0; i<15; i++) //read fix information
                {          
                    if(bp_runtime_info_temp.bp_exist[i]==0)
                    {                    
                        io_get( i+1,rev_tmp_buff,  CMD_GET_FIX_INFO);
                        //insertUnit(0,8,UC_OLP,1,binfo,0);
                        //insertUnit(0,7,UC_OLP,1,binfo,0);
                        memcpy(&(chassis_info.card_fix_info[i]),rev_tmp_buff,sizeof(card_fix_Info_t));
                        //insertUnit(0,8,UC_OLP,1,binfo,0);
                        io_get( i+1,message,  CMD_GET_RUN_TIME);
                        //get_parseMessage(message,i+1);  //parse message and copy to share memory            
                        //insertUnit(0,4,UC_OLP,1,binfo,0);
                        #ifdef SAVE_TO_SHM
                        bi=getBaseInfoMap(0,i+1);
                        if (NULL!=bi)
                        {
                          uclass=chassis_info.card_fix_info[i].Card_Type;
                          if (UC_OLP==uclass)
                          {
                            sprintf(bi->property,"%s-%d","OLP11-1-1",i+1);
                          }
                          else
                          {
                           sprintf(bi->property,"%s-%d","OTU-1-1",i+1);
                           uclass=UC_OTU;
                          }
                        strncpy(bi->model,chassis_info.card_fix_info[i].model,30);
                        strncpy(bi->sn,chassis_info.card_fix_info[i].sn,15);
                        strncpy(bi->fwver,chassis_info.card_fix_info[i].fwver,10);
                        strncpy(bi->hwver,chassis_info.card_fix_info[i].hwver,10);
                        strncpy(bi->creation,chassis_info.card_fix_info[i].creation,10);
                        printf("Unit #%d model:%s\n",i+1,chassis_info.card_fix_info[i].model);
                        insertUnit(0,i+1,uclass,1,*bi,0);
                        //insertUnit(0,4,UC_OLP,1,binfo,0);
                        //io_get( i+1,chassis_info.runtime_info[i],  CMD_GET_RUN_TIME);
                        //for (n=0;n<60;n++)
                        // printf("%d\n",  message[n]);
                        saveMessageToShm(0,i+1,message);
                        }
                        #endif
                        printf("Unit #%d exists\n",i+1);
                    }
                }
            }
            else
            {
                //中断读取变化的板卡信息
	        //printf("Here $$$!\n");
                io_get( BP_ADDR,rev_tmp_buff,  CMD_GET_RUN_TIME);
                //printf("Here 2@@@!\n");
                memcpy(&bp_runtime_info_temp,rev_tmp_buff,sizeof(backplane_info_t));
                //for (i=0;i<15;i++)
                  //printf("%d ;",bp_runtime_info_temp.bp_exist[i]);
                for(i=0; i<15; i++)
                {
                    if(bp_runtime_info_temp.bp_exist[i]<chassis_info.bp_runtime_info.bp_exist[i])//判断是否有板卡插入
                    {
                        // 板卡插入:读取固定信息
                        io_get( i+1,rev_tmp_buff,  CMD_GET_FIX_INFO);
                        memcpy(&(chassis_info.card_fix_info[i]),rev_tmp_buff,sizeof(card_fix_Info_t));
                        io_get( i+1,chassis_info.runtime_info[i],  CMD_GET_RUN_TIME);
                        //get_parseMessage(chassis_info.runtime_info[i],i+1);  //parse message and copy to share memory

                        printf("Unit #%d inserted\n",i+1);
                        //printf("\nUnit %d= %s|%s|%s|%s",i+1,chassis_info.card_fix_info[i].Card_parm1,chassis_info.card_fix_info[i].Card_parm2,chassis_info.card_fix_info[i].Card_parm3,chassis_info.card_fix_info[i].Card_parm4);
                        #ifdef SAVE_TO_SHM
                        bi=getBaseInfoMap(0,i+1);
                        if (NULL!=bi)
                        {
                         olp1P1Info_t *olp;
                         char *values;
                         
                         uclass=chassis_info.card_fix_info[i].Card_Type;
                         if (UC_OLP==uclass)
                         {
                            sprintf(bi->property,"%s-%d","OLP11-1-1",i+1);
                         }
                         else
                         {
                           sprintf(bi->property,"%s-%d","OTU-1-1",i+1);
                           uclass=UC_OTU;
                          }
                         strncpy(bi->model,chassis_info.card_fix_info[i].model,30);
                         strncpy(bi->sn,chassis_info.card_fix_info[i].sn,15);
                         strncpy(bi->fwver,chassis_info.card_fix_info[i].fwver,10);
                         strncpy(bi->hwver,chassis_info.card_fix_info[i].hwver,10);
                         strncpy(bi->creation,chassis_info.card_fix_info[i].creation,10);
                         insertUnit(0,i+1,uclass,1,*bi,1);
                         io_get( i+1,message, CMD_GET_RUN_TIME);
                         saveMessageToShm(0,i+1,message);
                         /*olp=getolpP1Info(0,i+1);
                         values=getInfoSetValues(0,i+1,OLP_STATUS_INFOSET_ID,1);
                         if (values!=NULL)
                         {
                         }
                         */
                        }
                      #endif
                    }
                    else if(bp_runtime_info_temp.bp_exist[i]>chassis_info.bp_runtime_info.bp_exist[i] )
                    {
                        //板卡拔出，清除固定信息
                        memset(&(chassis_info.card_fix_info[i]),0,sizeof(card_fix_Info_t));
                        printf("Unit %d removed\n",i+1);
                        #ifdef SAVE_TO_SHM
                        removeUnit(0,i+1);
                        #endif
                    }
                    else if((bp_runtime_info_temp.bp_exist[i]==0) && (chassis_info.bp_runtime_info.bp_exist[i]==0) && (bp_runtime_info_temp.bp_data_valid[i]==0))
                    {
                        //数据变化，只读取数据
                        //data chanage
                        // dev_type=chassis_info.card_fix_info[i].Card_Type;
                        printf("Unit #%d data chanage\n",i+1);//debug
                        //io_get( i+1,chassis_info.runtime_info[i],  CMD_GET_RUN_TIME);
                        #ifdef SAVE_TO_SHM
                        io_get( i+1,message,  CMD_GET_RUN_TIME);
                        saveMessageToShm(0,i+1,message);
                        #endif
                        //get_parseMessage(chassis_info.runtime_info[i],i+1);  //parse message and copy to share memory
                    }
                }
                memcpy(&(chassis_info.bp_runtime_info),&bp_runtime_info_temp,sizeof(backplane_info_t));
            }
        }
    //printf("\n");
    }//end while(1)

    gpio_fd_close(int_bp_fd);
    gpio_fd_close(int_setup_respone);

    return 0;
}





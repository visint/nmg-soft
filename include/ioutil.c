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
#include <chassis.h>
#include <olp.h>
#include <otu.h>

#define TIOCSRS485 0x542F


/****************************************************************
 * Constants
 ****************************************************************/

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

backplane_info_t bp_runtime_info_temp;
chassis_all_t chassis_info;

unsigned char nmu_own_addr=0xff;
unsigned char first_power_on=1; //识别初次上电
int rs485_comm_error=0;


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
/*
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
*/


/****************************************************************
 * LED 操作，参数1为LED序号，参数2为LED颜色状态
 ****************************************************************/
/*
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
*/


/****************************************************************
 * RS485发送校验
 ****************************************************************/
static unsigned char tx_crc_calc(char *p_buff,int len)
{
    unsigned char result=0;
    int i;
    for(i=0; i<len; i++)result+=p_buff[i];
    return (0-result);
}

/****************************************************************
 * RS485接收校验
 ****************************************************************/
static unsigned char rx_crc_calc(char *p_buff,int len)
{
    unsigned char result=0;
    int i;
    for(i=0; i<len; i++)result+=p_buff[i];
    return result;
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
    if ((fd = open("/dev/ttyS1",O_RDWR|O_NOCTTY|O_NONBLOCK))<0)
    {
        fprintf (stderr,"Open error on %s\n", strerror(errno));
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
                return EXIT_FAILURE;
            }

        }
        else
        {
            close(fd);
            return EXIT_FAILURE;
        }

    }

    close(fd);
    return EXIT_SUCCESS;
}



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

void initSys()
{

}

/****************************************************************
 * Main
 ****************************************************************/
/*
int main(void)
{
    struct pollfd fdset[2];
    int nfds = 2;
    int int_bp_fd, int_setup_respone,timeout, rc;
    char buf[MAX_BUF],send_tmp_buff[64];
    int len,i,access_result;
    enum UnitClass dev_type;
    nmu_own_addr=get_own_addr();
    led_init();                                 //初始化LED
    int_bp_fd = interrupt_init(bp_int_line);    //初始化背板CPU中断线
    int_setup_respone=interrupt_init(setup_respone); //初始化背板CPU中断线

    timeout = POLL_TIMEOUT;
    memset(&chassis_info,0,sizeof(chassis_all_t));
    while (1)
    {
        memset((void*)fdset, 0, sizeof(fdset));
        fdset[0].fd = int_setup_respone;
        fdset[0].events = POLLPRI;


        fdset[1].fd = int_bp_fd;
        fdset[1].events = POLLPRI;

        rc = poll(fdset, nfds, timeout);

        if (fdset[0].revents & POLLPRI)  //设置触发
        {
            len = read(fdset[0].fd, buf, MAX_BUF);
        }

        //printf("Polling\n");
        if (fdset[1].revents & POLLPRI)  //背板中断发生
        {
            //printf("interrupt raise\n");
            len = read(fdset[1].fd, buf, MAX_BUF);//清除中断标志

            if(first_power_on==1)//第一次上电读取全部信息
            {
                first_power_on=0;
pwron_bp_retry:
                access_result= card_access(BP_ADDR,cmd_get_runtime,send_tmp_buff,sizeof(send_tmp_buff),&bp_runtime_info_temp,sizeof(backplane_info_t));
                if(access_result==EXIT_FAILURE)
                {
                    usleep(220000);
                    goto pwron_bp_retry;
                }
                memcpy(&(chassis_info.bp_runtime_info),&bp_runtime_info_temp,sizeof(backplane_info_t));
                for(i=0; i<15; i++) //read fix information
                {
                    if(bp_runtime_info_temp.bp_exist[i]==0)
                    {
pwron_card_retry0:
                        access_result=card_access(i+1,cmd_get_fix_info,send_tmp_buff,sizeof(send_tmp_buff),&(chassis_info.card_fix_info[i]),sizeof(card_fix_Info_t));
                        if(access_result==EXIT_FAILURE)
                        {
                            usleep(220000);
                            goto pwron_card_retry0;
                        }
#ifdef DEBUG_4U
                        printf("\nUnit= %s",chassis_info.card_fix_info[i].Card_parm1);
#endif
                        printf("\nUnit %d= %s|%s|%s|%s",i+1,chassis_info.card_fix_info[i].Card_parm1,chassis_info.card_fix_info[i].Card_parm2,chassis_info.card_fix_info[i].Card_parm3,chassis_info.card_fix_info[i].Card_parm4);
                    }
                }

                for(i=0; i<15; i++) //read runtime information
                {
                    if(bp_runtime_info_temp.bp_exist[i]==0)
                    {
                        dev_type=chassis_info.card_fix_info[i].Card_Type;
                        switch(dev_type)
                        {
                        case UC_OLP:
                            chassis_info.runtime_info[i]=(olp1P1Info_t *)malloc(sizeof(olp1P1Info_t));
 pwron_card_retry1:
                        access_result=card_access(i+1,cmd_get_runtime,send_tmp_buff,sizeof(send_tmp_buff),chassis_info.runtime_info[i],sizeof(olp1P1Info_t));
                        if(access_result==EXIT_FAILURE)
                        {
                            usleep(220000);
                            goto pwron_card_retry1;
                        }
						break;
                        case UC_OTU:
                            break;
                        default:
                            break;

                        }


#ifdef DEBUG_4U
                        // olp1P1Info_t  *p_olp=(olp1P1Info_t  *)chassis_info.runtime_info;
                        // printf("\nmode= %d",p_olp->mode);
#endif
                    }
                }



            }
            else
            { //中断读取变化的板卡信息
loop_bp_retry:
                access_result= card_access(BP_ADDR,cmd_get_runtime,send_tmp_buff,sizeof(send_tmp_buff),&bp_runtime_info_temp,sizeof(backplane_info_t));
                if(access_result==EXIT_FAILURE)
                {
                    usleep(220000);
                    goto loop_bp_retry;
                }


                for(i=0; i<15; i++)
                {
                    if(bp_runtime_info_temp.bp_exist[i]<chassis_info.bp_runtime_info.bp_exist[i])//判断是否有板卡插入
                    {
                        // 板卡插入:读取固定信息
card_fix_retry:
                        access_result=card_access(i+1,cmd_get_fix_info,send_tmp_buff,sizeof(send_tmp_buff),&(chassis_info.card_fix_info[i]),sizeof(card_fix_Info_t));
                        if(access_result==EXIT_FAILURE)
                        {
                            usleep(220000);
                            goto card_fix_retry;
                        }

                        dev_type=chassis_info.card_fix_info[i].Card_Type; 
                        switch(dev_type) //判断板卡类型
                        {
                        case UC_OLP:   //如果是OLP ,开辟OLP动态信息存储空间
                            chassis_info.runtime_info[i]=(olp1P1Info_t *)malloc(sizeof(olp1P1Info_t));
card_runtime_retry:
                            access_result=card_access(i+1,cmd_get_runtime,send_tmp_buff,sizeof(send_tmp_buff),chassis_info.runtime_info[i],sizeof(olp1P1Info_t));
                            if(access_result==EXIT_FAILURE)
                            {
                                usleep(220000);
                                goto card_runtime_retry;
                            }
                            break;
                        case UC_OTU:
                            break;
                        default:
                            break;
                        }


                        printf("\nUnit #%d inserted",i+1);
                        printf("\nUnit %d= %s|%s|%s|%s",i+1,chassis_info.card_fix_info[i].Card_parm1,chassis_info.card_fix_info[i].Card_parm2,chassis_info.card_fix_info[i].Card_parm3,chassis_info.card_fix_info[i].Card_parm4);
                    }
                    else if(bp_runtime_info_temp.bp_exist[i]>chassis_info.bp_runtime_info.bp_exist[i] )
                    {
                        //板卡拔出，清除固定信息和释放内存
                        memset(&(chassis_info.card_fix_info[i]),0,sizeof(card_fix_Info_t));
                        free(chassis_info.runtime_info[i]);
                        printf("\nUnit %d removed",i+1);
                    }
                }

                memcpy(&(chassis_info.bp_runtime_info),&bp_runtime_info_temp,sizeof(backplane_info_t));
            }
            printf("\n");

        }
    }

    gpio_fd_close(int_bp_fd);
    gpio_fd_close(int_setup_respone);

    return 0;
}
*/

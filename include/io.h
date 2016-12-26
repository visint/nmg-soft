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

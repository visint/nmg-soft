/*
 *  unit.h -- Vispace Unit Management public header
 *
 *  Copyright (c) Visint Inc., 2002-2012. All Rights Reserved.
 *
 *  See the file "license.txt" for information on usage and redistribution
 *
 */

#ifndef __UNIT_H
#define __UNIT_H 1

/******************************** Description *********************************/

/*
 *  Vispace Unit header. This defines the Unit Management
 *  public APIs.  Include this header for files that contain access to
 *  unit inquiry or management.
 */

/********************************* Includes ***********************************/

#include  <typedefs.h>
#include  <infoset.h>
#include  <sys/time.h>

/********************************** Defines ***********************************/
//#define MAX_CHASSIS_COUNT 1	/*最大机箱数*/
#define MAX_UNIT_COUNT 16	/*单元盘最大数*/
#define MAX_GROUP_PORT_COUNT 2	/*每组端口最大数*/
#define UNIT_STATUS_INFO_SET_ID 1	/*单元盘状态信息集*/
#define UNIT_BASE_INFO_SET_ID 2

#define MAX_BASE_INFO_COUNT 7

/*单元盘基本信息*/
#define UNIT_PROPERTY_ID 1
#define UNIT_HW_VER_ID 2
#define UNIT_FW_VER_ID 3
#define UNIT_MODEL_ID 4
#define UNIT_SN_ID 5
#define UNIT_CREATION_ID 6
#define UNIT_UPTIME_ID 7

#define SUPPORT_UNIT uclass==3 || 4==uclass

/*单元盘用户信息*/
#define UNIT_NAME_ID 1
#define UNIT_LOC_ID 2
#define UNIT_CONTACT_ID 3
#define UNIT_UP_DEV_ID 4
#define UNIT_USER_DATA_ID 5
#define UNIT_SERVICE_ID 6

#define MAX_ITEM_SIZE 32

#define MAX_GROUP_COUNT 4

/**********************************************
*单元盘属性
*/
typedef struct
{
  uchar_t unit_class;     //单元盘大类
  uchar_t unit_type;      //单元盘小类
  uchar_t hardware_type;  //单元盘硬件类型
  uchar_t firmware_type;  //单元盘固件类型
  uchar_t struct_type;    //单元盘结构类型
}unitProperty_t;

/**********************************************
*单元盘基本信息
*以下信息存放在单元盘,只有特权用户可以修改
*/
typedef struct{
  char property[MAX_ITEM_SIZE];
  char sn[16]; 	              //单元盘序列号
  char model[MAX_ITEM_SIZE];  //单元型号
  char creation[12];          //单元盘创建日期，格式为：YYYY-MM-DD
  char fwver[12];             //单元盘固件版本号
  char hwver[12];             //单元盘硬件版本号
  time_t uptime;              //单元盘运行时间，单位为秒
}unit_base_info_t;

/*以下信息存放在Vispace.db数据库,用户可以修改*/
typedef struct{
  char name[MAX_ITEM_SIZE];       //单元盘名称
  char location[MAX_ITEM_SIZE];   //单元盘位置
  char contact[MAX_ITEM_SIZE];    //单元盘联系人
  char updev[MAX_ITEM_SIZE];      //单元盘上联设备
  char userdata[MAX_ITEM_SIZE];   //单元盘用户自定义信息
  char service[MAX_ITEM_SIZE];    //单元盘开通业务
}unit_user_data_t;

enum UnitClass
{
  UC_UNKOWN=0,
  UC_NMU=1,
  UC_MC_OAM=2,
  UC_OTU=3,
  UC_OLP=4,
  UC_OBP=5,
  UC_OSS=6,
  UC_AWG=7,
  UC_TDM=8
};

enum PortClass
{
  PC_UNKOWN=0,
  PC_eport=1,
  PC_OPORT1X9=2,
  PC_SFP=3,
  PC_SFP_PLUS=4,
  PC_XFP=5,
  PC_ADAPTER=9
};

enum PortType
{
  PT_UNKOWN=0,
};

typedef struct
{
  char group_name[16];
  uchar_t port_count;
  enum PortClass port_class[MAX_GROUP_PORT_COUNT];
  uchar_t port_type[MAX_GROUP_PORT_COUNT];
  uchar_t port_status[MAX_GROUP_PORT_COUNT];
}portGroup_t;

typedef struct
{
  char unit_name[10];
  uchar_t group_count;
  portGroup_t port_group[MAX_GROUP_PORT_COUNT];
}panelInfo_t;

typedef struct
{
  char unit_name[10];
  uchar_t port_count;
  uchar_t port_type[MAX_GROUP_PORT_COUNT];
  uchar_t port_status[MAX_GROUP_PORT_COUNT];
  //uchar_t port_info[MAX_GROUP_PORT_COUNT][20];
}unitInfo_t;

/*
typedef struct
{
 char wave_length[8];
 char distance[6];
 char max_speed[6];
 char current_speed[6];
 uchar_t port_count;
 uchar_t rx_status[2];
 uchar_t tx_status[2];
 uchar_t tx_enable[2];
}channel_t;

typedef struct
{
  uchar_t channel_count;
  channel_t channel[OTU_MAX_CHANNEL_COUNT];
}otuInfo_t;
*/
/*
typedef struct
{
  short rx1_power,rx2_power,tx_power;
  short l1_power,l2_power,tx_alm_power;
}olpPowerInfo_t;

typedef struct
{
  olpPowerInfo_t power;
  short ret_time;
}olp1Data_t;

typedef struct
{
  uchar_t  operating_mode;//工作模式:1--自动 2--手动
  uchar_t  rule;
  uchar_t  line;//1:切换到主路(L1)  2:切换到备路(L2);工作模式为手动时才能设置
  uchar_t  ret_mode;
}olp1StatusInfo_t;

typedef struct
{
  uchar_t  operating_mode;
  uchar_t  line;
}olp2StatusInfo_t;
*/


int isValidUnitClass(uchar_t uclass,uchar_t utype);

boolean_t checkUnitClassType(unit_base_info_t bi,uchar_t *unit_class,uchar_t *unit_type);
/************************************************************************
【功能】获取单元盘属性
【参数说明】
            1、uchar_t slo chassis_number  机箱号（0-MAX_CHASSIS_COUNT）,0为主机箱
            2、uchar_t slot 槽位号（0-MAX_UNIT_COUNT）,0为背板（0-MAX_UNIT_COUNT）
            3、uchar_t index  属性集索引，一般为1
【返回值说明】成功返回单元盘属性指针，失败返回NULL;
【使用方法】
            unitProperty_t *pro;
            pro = getUnitProperty (0,1,1);
***********************************************************************/
unitProperty_t *getUnitProperty(uchar_t chassis,uchar_t slot,uchar_t index);

boolean_t getUnitPanelInfo(uchar_t classis,uchar_t slot,panelInfo_t *info);
status_infoset_ex_t *getMapStatusInfoSet(uchar_t classis,uchar_t slot,uchar_t infoset_id);
//boolean_t getOlp1P1Info(uchar_t chassis,uchar_t slot,olp1StatusInfo_t *info);
//boolean_t getUnitType(uchar_t classis,uchar_t slot,uchar_t *unit_class,uchar_t *unit_type,char *unit_short_name);

#endif /* __UNIT_H */
/******************************************************************************/

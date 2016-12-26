/*
 *  unit.h -- Vispace Unit Management public header
 *
 *  Copyright (c) Visint Inc., 2002-2012. All Rights Reserved.
 *
 *  See the file "license.txt" for information on usage and redistribution
 *
 */

#ifndef __LOG_H
#define __LOG_H 1

/******************************** Description *********************************/

/*
 *  Vispace Unit header. This defines the Unit Management
 *  public APIs.  Include this header for files that contain access to
 *  unit inquiry or management.
 */

/********************************* Includes ***********************************/

//#include  <typedefs.h>

/********************************** Defines ***********************************/
#define MAX_LOG_COUNT 200	/*最大保存日志条数*/

#define LOG_GROUP_SYS_IP_CONF 1
#define LOG_GROUP_SYS_SYS_INFO_CONF 2
#define LOG_GROUP_SYS_USER_CONF 3
#define LOG_GROUP_SYS_USER_ACL_CONF 4
#define LOG_GROUP_SYS_USER_GROUP_CONF 5
#define LOG_GROUP_SYS_PASSWORD_CONF 6

#define LOG_GROUP_SYS_SNMP_COMMINITY_CONF 10
#define LOG_GROUP_SYS_SNMP_DEST_HOST_CONF 11

#define LOG_GROUP_SYS_CURRENT_ALARM_CONF 12
#define LOG_GROUP_SYS_HISTORY_ALARM_CONF 13
#define LOG_GROUP_SYS_SNMP_ALARM_TYPE_CONF 14

#define LOG_GROUP_SYS_OPT_LOG_CONF 15

#define LOG_UNIT_OTU_CONF  31
#define LOG_UNIT_OLP_CONF 32


enum LogOptMode
{
  UnkownMode=0,
  CliMode=1,
  WebMode=2,
  SnmpMode=3
};
enum OperateType
{
  unkown_opt=0,
  mod=1,
  add=2,
  del=3,
  clr=4,
  ack=5,
  reset=6,
  test=7
};
typedef struct
{
  char group_name[16];
}operateLog_t;

int insertOperateLog(int group_id,int object_id,int user_id,int mode,int operate_type);
int deleteOperateLogById(int id);

#endif /* _H_LOG */
/******************************************************************************/

#ifndef __VIS_IPC_H
#define __VIS_IPC_H

/******************************** 说明 *********************************/

/*
 * 信息集是硬件网管信息与上层访问的接口。网管信息可通过ID号来访问。
 * 网管信息ID  由三部分组成:index +InfoSetID +ItemID,相同结构的信息可以放在同一个
 * 信息集里，不同的信息集可以组合成新的信息集。信息集里的信息项可以
 * 绑定到网管信号。也可以不绑定，需要访问网管信号时再具体实现。
 */

/********************************* Includes ***********************************/
#include <typedefs.h>
#include <unit.h>
/********************************** Defines ***********************************/
#define MAX_MESSAGE_SIZE 32
#define MAX_MESSAGE_HEAD 32
//#define MAX_ITEM_COUNT 16
#define MAX_IPC_MSG_BUF 36
#define MAX_IPC_UBI_BUF sizeof(unit_base_info_t)
#define MSGKEY 72
typedef struct
{
  long mtype;
  char mbuf[MAX_IPC_MSG_BUF];
}msgbuf_t;

typedef struct
{
  long mtype;
  char mbuf[MAX_IPC_UBI_BUF];
}ubibuf_t;

// linux IPC number need to greater than 1
enum msgtype{
	IPC_CC_MSG_ID = 1,
	IPC_UNIT_MSG_ID,
	IPC_SNMP_MSG_ID,
	IPC_CLI_MSG_ID,
	IPC_HTTP_MSG_ID,
	IPC_RESTORE_DEFAULT /* Paul add 2012/8/7 */
};

extern int sendSigToCC(int sig,int  sigval);
extern int sendMsgQ(msgbuf_t sndbuf,boolean_t watting,uchar_t timeout);
extern int sendMsgToCC(msgbuf_t sndbuf,boolean_t watting,uchar_t timeout);

extern char getUnitName(uchar_t chassis,uchar_t slot,char *unit_name,char flag);
extern uchar_t getUnitClass(uchar_t chassis,uchar_t slot);
//extern uchar_t getUnitStatusFromShm(uchar_t chassis);
extern boolean_t getUnitClassType(uchar_t chassis,uchar_t slot,uchar_t *uclass,uchar_t *utype);
extern uchar_t *getUnitStatusFromShm(uchar_t chassis);
extern void setUnitStatusToShm(uchar_t chassis,uchar_t slot,uchar_t uclass,uchar_t utype);

extern uchar_t *getUnitStatusMap(uchar_t chassis);
extern uchar_t *getUnitStatusMbusMap(uchar_t chassis);
extern unit_base_info_t *getBaseInfoMap(uchar_t chassis,uchar_t slot);
extern unit_base_info_t *getBaseInfoMbusMap(uchar_t chassis,uchar_t slot);
extern status_infoset_t *getInfoSetMbusMap(uchar_t chassis,uchar_t slot,uchar_t infoset_id,uchar_t index);
extern status_infoset_t *getStatusInfoSetMap(uchar_t chassis,uchar_t slot,uchar_t infoset_id,uchar_t index);
char *getInfoSetValues(uchar_t chassis,uchar_t slot,uchar_t infoset_id,uchar_t index);
#endif /* __VIS_IPC_H */

/******************************************************************************/




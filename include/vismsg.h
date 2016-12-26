#ifndef __VIS_MSG_H
#define __VIS_MSG_H

/******************************** 说明 *********************************/

/*
 * 信息集是硬件网管信息与上层访问的接口。网管信息可通过ID号来访问。
 * 网管信息ID  由三部分组成:index +InfoSetID +ItemID,相同结构的信息可以放在同一个
 * 信息集里，不同的信息集可以组合成新的信息集。信息集里的信息项可以
 * 绑定到网管信号。也可以不绑定，需要访问网管信号时再具体实现。
 */

/********************************* Includes ***********************************/
#include <typedefs.h>
/********************************** Defines ***********************************/
#define MAX_MESSAGE_SIZE 32
#define MAX_MESSAGE_HEAD 32
//#define MAX_ITEM_COUNT 16
#define MAX_IPC_MSG_BUF 32
#define MSGKEY 72
typedef struct
{
  long mtype;
  char mbuf[MAX_IPC_MSG_BUF];
}msgbuf_t;

// linux IPC number need to greater than 1
enum msgtype{
	IPC_CC_MSG_ID = 1,
	IPC_UNIT_MSG_ID,
	IPC_SNMP_MSG_ID,
	IPC_CLI_MSG_ID,
	IPC_HTTP_MSG_ID,
	IPC_RESTORE_DEFAULT /* Paul add 2012/8/7 */
};
/*
extern int CreateMsgQ(char msgkey,uchar_t timeout);
extern int RcvMsgQ(int msgid,msgbuf_t *msgbuf,enum msgtype mtype);
extern bool_t SndMsgQ(int msgid,msgbuf_t msgbuf,bool_t watting,uchar_t timeout);
extern bool_t SndMsgQToCC(msgbuf_t msgbuf,bool_t watting,uchar_t timeout);
extern bool_t DelMsgQ(int msgid);
*/
extern int CreateMsgQ(uchar_t timeout);
extern int RcvMsgQ(msgbuf_t *rcvbuf,bool_t watting,uchar_t timeout);
extern int SndMsgQ(msgbuf_t sndbuf,bool_t watting,uchar_t timeout);
extern int SndMsgQToCC(msgbuf_t sndbuf,bool_t watting,uchar_t timeout);
extern bool_t DelMsgQ();
#endif /* __VIS_MSG_H */

/******************************************************************************/




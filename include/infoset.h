#ifndef __INFOSET_H
#define __INFOSET_H

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
#define MAX_ITEM_COUNT 20
#define MAX_ITEM_SIZE 32
#define MAX_SETTING_ITEM_COUNT 10
#define MAX_MESSAGE_SIZE 32

#define BASE_INFOSET_ID 1
#define USER_DATA_INFOSET_ID 2
#define CHASSIS_INFOSET_ID 3
#define CHASSIS_THRESHOLD_INFOSET_ID 4
#define DDM_INFOSET_ID 11
#define DDM_THRESHOLD_ID 12

#define UNIT_PROPERTY_INFOSET_ID 10

#define OTU_SFP_PROPERTY_INFOSET_ID 50
#define OTU_STATUS_INFOSET_ID 51
#define OTU_SFP_FEC_INFOSET_ID 52
#define SFP_ERROR_CODE_TEST_ID 53

#define OPORT_INFOSET_ID 61
//#define OTU_STATUS_INFOSET_ID 61
#define OLP_STATUS_INFOSET_ID 65
#define OLP_DATA_INFOSET_ID 66


//#pragma pack(2) /* n = 1, 2, 4, 8, 16 */
typedef struct
{
	uchar_t info_set_id;
	//uchar_t item_max_size;
	uchar_t item_id;
	boolean_t  can_set; 
	//char   item_name[32];
}infoitem_t;

typedef struct
{
  uchar_t item_id;
  short  item_value; 
}infoItem_t;

typedef struct
{
    uchar_t chassis;
    uchar_t slot;
    char infoset_type;//'s'-字节型 'S'-双字节
    uchar_t infoset_id;
    uchar_t index;
    infoItem_t infoItems[MAX_SETTING_ITEM_COUNT];
    int useconds;
}setInfoSet_t;

typedef struct
{
  //uchar_t item_id;
  uchar_t value;
  //boolean_t  readonly; 
}status_infoitem_t;

typedef struct
{
   uchar_t infoset_id; /*!< 信息集ID号*/
   uchar_t infoset_type; //1:状态信息;2:双字节信息 3:字符串信息
   uchar_t index;
   uchar_t infoset_size;
   uchar_t item_count;
   uchar_t item_unuse;
   //status_infoitem_t infoitems[MAX_ITEM_COUNT];
   char item_values[MAX_ITEM_SIZE];
}status_infoset_t;

typedef struct
{
   uchar_t infoset_id; /*!< 信息集ID号*/
   uchar_t index;	
   uchar_t item_count;
}message_infoset_t;

typedef struct
{
   message_infoset_t infoset[6];
   uchar_t infoset_type; //1:状态信息;2:双字节信息 3:字符串信息	
   uchar_t infoset_count;
}message_t;

typedef struct
{
   message_t message[6];	
   uchar_t message_count;
   uchar_t command_size;
   uchar_t result_size;
}message_set_t;

typedef struct
{
   status_infoset_t *infoset;
   uchar_t unit_class;
   uchar_t unit_type;
   uchar_t index_count;
   char   infoset_name[16];
   char   item_name[MAX_ITEM_COUNT][16];
   char   status_name[MAX_ITEM_COUNT][16];
}status_infoset_ex_t;

typedef struct
{
   uchar_t info_set_id; /*!< 信息集ID号*/
   uchar_t index;
	
   uchar_t item_count;
   uchar_t *item_ids;
   char  **names[MAX_ITEM_COUNT];
}infoset_t;

typedef struct
{
	infoset_t infoset;
	char values[MAX_ITEM_COUNT];
}byteinfoset_t;
typedef struct
{
	infoset_t infoset;
	short values[MAX_ITEM_COUNT];
}wordinfoset_t;

typedef struct
{
	infoset_t infoset;
	char values[MAX_ITEM_COUNT+1];
}stringinfoset_t;
//#pragma pack(2)
status_infoset_t *getInfoSet(uchar_t chassis,uchar_t slot,uchar_t infoset_id,uchar_t index);
int setUnitInfoSet(setInfoSet_t infoset);
#endif /* __INFOSET_H */

/******************************************************************************/




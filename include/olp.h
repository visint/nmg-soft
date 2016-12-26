#ifndef __OLP_H
#define __OLP_H 1


#define OLP1_L1_SW_POWER 4
#define OLP1_L2_SW_POWER 5
#define OLP1_TX_ALM_POWER 6
#define OLP1_RET_TIME 7

#define OLP1_MODE 8
#define OLP1_LINE 9
#define OLP1_RULE 10
#define OLP1_RET_MODE 11

#define OLP1_R1_WAVE_LEN 15
#define OLP1_R2_WAVE_LEN 16
#define OLP1_TX_WAVE_LEN 17

#define OLP_MIN_RET_TIME 0
#define OLP_MAX_RET_TIME 999

#define OLP_MIN_L1_SW_POWER -500
#define OLP_MAX_L1_SW_POWER 100

#define OLP_MIN_L2_SW_POWER -500
#define OLP_MAX_L2_SW_POWER 100

#define OLP_MIN_TX_ALM_POWER -500
#define OLP_MAX_TX_ALM_POWER 100

#define OLP_MIN_RET_TIME 0
#define OLP_MAX_RET_TIME 999
#pragma pack(2) /* n = 1, 2, 4, 8, 16 */
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



typedef struct
{
  short rx1_power; //R1光功率
  short rx2_power; //R2光功率
  short tx_power;  //Tx光功率
  short l1_power;
  short l2_power;
  short tx_alm_power;
  short ret_time;  //返回时间:0-999

  uchar_t  mode;//工作模式:1--自动 2--手动
  uchar_t  line;//线路选择:1:切换到主路(L1)  2:切换到备路(L2);工作模式为手动时才能设置
  uchar_t  rule; //切换规则
  uchar_t  ret_mode;//返回模式
  //uchar_t  unused;
  uchar_t rx1_led;
  uchar_t rx2_led;
  uchar_t tx_led;
  uchar_t rx1_wave_len;
  uchar_t rx2_wave_len;
  uchar_t tx_wave_len;
  
  uchar_t reserve1;
  uchar_t reserve2;
  uchar_t reserve3;
  uchar_t reserve4;
}__attribute__((packed))olp1P1Info_t;


olp1P1Info_t *getOlp1P1Info(uchar_t chassis,uchar_t slot);
int setOlpInfo(setInfoSet_t infoset);
#pragma pack(2)
#endif /* __OLP_H */

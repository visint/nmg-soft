#ifndef __DDM_H
#define __DDM_H 1

/*OTU单元盘最大通道数*/
//#define OTU_MAX_CHANNEL_COUNT 2
#define MAX_GROUP_PORT_COUNT 2
typedef struct{
	short rxPower;         /*!< 输入光功率单位:0.1dBm*/
	short txPower;         /*!< 输出光功率单位:0.1dBm*/
	short rxVolt;          /*!< SFP 电压单位:毫伏*/
	short rxBiasCurrent;   /*!< SFP 偏置电流单位:毫安*/
        short temperature;     /*!< SFP 温度单位:0.1°C*/
}ddm_t;


typedef struct{
      signed char rxPowerMin;  /*!< 输入最小光功率单位:0.1dBm*/
	signed char rxPowerMax;  /*!< 输入最大光功率单位:0.1dBm*/
      signed char txPowerMin;  /*!< 输出最小光功率单位:0.1dBm*/
	signed char txPowerMax;  /*!< 输出光功率单位:0.1dBm*/
      char voltMin;     /*!< SFP  最小电压单位:毫伏*/
	char voltMax;     /*!< SFP  最大电压单位:毫伏*/
      char rxBiasMin;   /*!< SFP  最小偏置电流单位:毫安*/
	char rxBiasMax;   /*!< SFP  最大偏置电流单位:毫安*/
      signed char tempMin;     /*!< SFP  最小温度单位:0.1°C*/
      char tempMax;     /*!< SFP  最大温度单位:0.1°C*/
}ddmThreshold_t;

typedef struct{
 ddmThreshold_t port_ddm[MAX_GROUP_PORT_COUNT];
}groupDdmThreshold_t;
ddm_t *getPortDdmInfo(uchar_t chassis,uchar_t slot,uchar_t port);
ddmThreshold_t *getPortDdmThreshold(uchar_t chassis,uchar_t slot,uchar_t index);
groupDdmThreshold_t *getGroupDdmThreshold(uchar_t chassis,uchar_t slot,uchar_t group);

#endif /* __DDM_H */

#ifndef __OTU_H
#define __OTU_H 1

/*OTU单元盘最大通道数*/
#define OTU_MAX_CHANNEL_COUNT 2
#define OTU_EVERY_CHANNEL_PORT_COUNT 2

typedef struct
{
  unsigned short  distance;
  unsigned short  max_speed;
  char  wave_length[7];
}otuChannelProperty;

typedef struct
{
  unitProperty_t unit_property;
  //uchar_t otu_type;
  uchar_t channel_count;
  uchar_t channel_port_count;
  uchar_t port_type;
  otuChannelProperty channel_property[OTU_MAX_CHANNEL_COUNT];
}otuProperty_t;

typedef struct
{
 uchar_t rx_status;
 uchar_t tx_status;
 uchar_t tx_enable;
}otuPortStatus_t;

typedef struct
{
 unsigned short current_speed;
 otuPortStatus_t  port_status[OTU_EVERY_CHANNEL_PORT_COUNT];
}otuChannelStatus_t;

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

typedef struct
{
  uchar_t reserve1;
  uchar_t reserve2;
  uchar_t work_mode;
  uchar_t run_status; 
  otuPortStatus_t portStatus[8];
}otuUnitStatus_t;

typedef struct
{
  uchar_t loopback;
  uchar_t fec;
  uchar_t reserve1;
  uchar_t reserve2;
}otuPortProperty_t;

int insertOtuInfoSets(uchar_t chassis,uchar_t slot,otuProperty_t otu);
//bool_t getOtuInfo(uchar_t chassis,uchar_t slot,uchar_t channel,otuInfo_t *info);
otuInfo_t *getOtuInfo(uchar_t chassis,uchar_t slot,uchar_t channel);
otuProperty_t *getOtuProperty(uchar_t chassis,uchar_t slot,uchar_t index);
otuChannelStatus_t *getOtuChannelStatus(uchar_t chassis,uchar_t slot,uchar_t group);
#endif /* __OTU_H */

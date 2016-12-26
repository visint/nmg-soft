/*
 *  unit.h -- Vispace Unit Management public header
 *
 *  Copyright (c) Visint Inc., 2002-2012. All Rights Reserved.
 *
 *  See the file "license.txt" for information on usage and redistribution
 *
 */

#ifndef __SYSINFO_H
#define __SYSINFO_H 1

/******************************** Description *********************************/

/*
 *  Vispace Unit header. This defines the Unit Management
 *  public APIs.  Include this header for files that contain access to
 *  unit inquiry or management.
 */

/********************************* Includes ***********************************/

//#include  <typedefs.h>
/********************************** Defines ***********************************/
#define MAX_POWER_SUPPLY_COUNT 4	/*最大保存日志条数*/
#define MAX_POWER_FAN_COUNT 4
#define NMU_FW_VERSION "01.00.01"
#define NMU_SW_VERSION "01.00.01"

/*
enum PowerSupplyType
{
  unkownType=0,
  psAC=1,
  psDC=2
};

typedef struct
{
  enum PowerSupplyType type;
  short volt;
  short max_vlot;
  short min_vlot;
}powerSupply_t;

typedef struct
{
  short temperature;
  short max_temperature;
  short min_temperature;
}temperature_t;

typedef struct
{
  char status;
}fan_t;

typedef struct
{
  powerSupply_t power_supply[MAX_POWER_SUPPLY_COUNT];
  temperature_t temperature;
  fan_t fan[MAX_POWER_FAN_COUNT];
}sysInfo_t;
*/
struct cpu_usage_t
{
	unsigned long cpu_user;
	unsigned long cpu_sys;
	unsigned long cpu_nice;
	unsigned long cpu_idle;
};

struct mem_usage_t
{
	unsigned long total;
	unsigned long used;
	unsigned long free;
	unsigned long shared;
        unsigned long buffers ;
        unsigned long cached;
};

void restore(int chassis,int slot);
char getCpuUsage(float *sys_usage,float *user_usage);
float  getMemUsage(struct mem_usage_t *usage);

int getMac(char* mac);
int getDeviceIp(char *ip_add,char *mask,char *gateway);
int setDeviceIp(char *ip_addr,char *mask,char *gateway);
pid_t getPidByName ( char *name);
int doSystem(const char *fmt, ...);
//int getSysInfo(sysInfo_t *sys_info);
#endif /* _H_SYSINFO */
/******************************************************************************/

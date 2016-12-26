/*
 *  unit.h -- Vispace Unit Management public header
 *
 *  Copyright (c) Visint Inc., 2002-2012. All Rights Reserved.
 *
 *  See the file "license.txt" for information on usage and redistribution
 *
 */

#ifndef __SNMP_CONF_H
#define __SNMP_CONF_H 1

/******************************** Description *********************************/

/*
 *  Vispace Unit header. This defines the Unit Management
 *  public APIs.  Include this header for files that contain access to
 *  unit inquiry or management.
 */

/********************************* Includes ***********************************/

//#include  <typedefs.h>

/********************************** Defines ***********************************/
#define MAX_DEST_HOST_COUNT 10	/*最大目标主机数*/

int snmpdConf();
int insertCommunity(const char *ipadd,const char *rocommunity,const char *rwcommunity,int port,int read_only,int enable);
int updateCommunity(int id,const char *ipadd,const char *rocommunity,const char *rwcommunity,int port,int read_only,int enable);
int deleteCommunityById(int id);
int deleteCommunityByIp(char * ip);

int insertDestHost(const char *dest_ipadd,int udp_port,int enable);
int deleteDestHost(const char *dest_ipadd,int udp_port);
int restoreSnmpdConf(char isRestart);

#endif /* _H_LOG */
/******************************************************************************/

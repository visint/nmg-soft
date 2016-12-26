#ifndef __ALARM_H
#define __ALARM_H
int setAlarmStatus(int alarm_code,char chassis,char slot,char entity,char status);
int setAlarmStatusById(int alarm_id,char status);
int deleteAlarmLogById(int alarm_id);
int cleanAlarmLog(int alarm_code,char chassis,char slot,char entity);
int ackAlarmLog(int alarm_code,char chassis,char slot,char entity);
int insertAlarmLog(int alarm_code,char chassis,char slot,char entity);
void sendSnmpTrap(int alarm_code,char chassis,char slot,char entity);

#endif /* __ALARM_H */

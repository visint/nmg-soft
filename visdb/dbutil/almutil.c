#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <unit.h>
#include <sqlite3.h>
#include <visdb.h>
#include <dbutil.h>

#define MAX_ALARM_COUNT 300
//gcc -o /usr/local/bin/dbutil.o -lvisipc -lvisdb ../vislib/unit.o ../vislib/visstr.o dbutil.c  -I../include
//gcc -c almutil.c  -I../../include && ar -r libdbutil.a almutil.o
int setAlarmStatus(int alarm_code,char chassis,char slot,char entity,char status)
{
  char sql[200];
  
  if (slot<1 ||slot>MAX_UNIT_COUNT || entity<0 ||entity>8 || status<1 ||status>2)
    return -1;

  sprintf(sql,"UPDATE AlarmLog SET alarm_status=alarm_status|%d WHERE alarm_code=%d AND slot=%d AND entity=%d;",status,alarm_code,slot,entity);
  
 if (SQLITE_OK!=execSql(NULL, sql,NULL,NULL))
       return -1;
  return 0;
}
int setAlarmStatusById(int alarm_id,char status)
{
  char sql[80];
  sprintf(sql,"UPDATE AlarmLog SET alarm_status=alarm_status|%d WHERE alarm_id=%d;",status,alarm_id);
  if (SQLITE_OK!=execSql(NULL, sql,NULL,NULL))
       return -1;
  return 0;
}

int deleteAlarmLogById(int alarm_id)
{
  char sql[80];
  sprintf(sql,"DELETE FROM AlarmLog  WHERE alarm_id=%d;",alarm_id);
  if (SQLITE_OK!=execSql(NULL, sql,NULL,NULL))
       return -1;
  return 0;
}
int cleanAlarmLog(int alarm_code,char chassis,char slot,char entity)
{
  return setAlarmStatus(alarm_code,chassis,slot,entity,1);
}
int ackAlarmLog(int alarm_code,char chassis,char slot,char entity)
{
  return setAlarmStatus(alarm_code,chassis,slot,entity,2);
}

int insertAlarmLog(int alarm_code,char chassis,char slot,char entity)
{
  if (alarm_code %2)
  {
    char sql[220];
    int count;

    if (slot<0 ||slot>MAX_UNIT_COUNT || entity<0 ||entity>8)
     return -1;
    sprintf(sql,"DELETE FROM AlarmLog WHERE ID NOT IN (SELECT ID FROM AlarmLog order by ID LIMIT %d)",MAX_ALARM_COUNT);
    if (SQLITE_OK!=execSql(NULL, sql,NULL,NULL))
        return -1;
   sprintf(sql,"SELECT alarm_code FROM AlarmType WHERE alarm_code=%d AND enable=1 AND mask_slot NOT LIKE '%c%d;%c' LIMIT 1",alarm_code,'%',slot,'%');
  
   if (getRowCount(NULL,sql) >0)
   {
     int status=-1,ret;
     sprintf(sql,"SELECT alarm_status FROM AlarmLog WHERE alarm_code=%d AND slot=%d AND entity=%d ORDER BY alarm_status LIMIT 1;",alarm_code,slot,entity);
     ret=getColIntValue(NULL,sql,&status);
     //printf("status=%d ret=%d\n",status,ret);
     if (ret==-1)
       return -1;
     if (status==-1 || status==3) //记录不存在或为已清除已确认状态
     {
       sprintf(sql,"INSERT INTO \"AlarmLog\" VALUES(NULL,%d,%d,%d,%d,0,datetime('now','localtime'));",alarm_code,chassis,slot,entity);
     }
     else
         sprintf(sql,"UPDATE \"AlarmLog\"  SET alarm_status=0,alarm_time=datetime('now','localtime') WHERE alarm_status!=3 AND alarm_code=%d AND slot=%d AND entity=%d;",alarm_code,slot,entity);
    if (SQLITE_OK!=execSql(NULL, sql,NULL,NULL))
        return -1;
   }
   //printf("sql=%s\n",sql);
  }
  else
      return setAlarmStatus(alarm_code-1,chassis,slot,entity,1);
  return 0;
}
void deleteAlarmLog(char *where)
{
  
}
void sendSnmpTrap(int alarm_code,char chassis,char slot,char entity)
{
  sqlite3_stmt* stmt;
  sqlite3 *db;
  int ret,n=0;
  char sql[256]="";
  
  //char inbuf[]="中文";
  //size_t inlen=strlen(inbuf);
  char ut8buf[160];
  char gbkbuf[160]="";
  char bytes[6]="";
  char entity_name[20]="";
  char alarm_msg[120];

  if (insertAlarmLog(alarm_code,chassis,slot,entity)<0)
      return;
  
  //printf("sendSnmpTrap:\n");
  //return;
  db=openDB(DB_FILE_NAME);
  if (NULL==db)
     return;
 
  sprintf(sql,"SELECT A.dest_ipadd,A.udp_port,B.alarm_name_en,B.entity_name_en FROM DestHost AS A,AlarmType AS B WHERE A.enable=1 AND B.alarm_code=%d",alarm_code);
  //printf("sendSnmpTrap:%s\n",sql);
  //sqlite3_close(db);
  //return;
  ret=sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL);
  if(ret!=SQLITE_OK)
  {
     //sqlite3_finalize(stmt);
     sqlite3_close(db);
     return;
  }
  //sqlite3_finalize(stmt);
  //sqlite3_close(db);
  //return;
  //printf("sendSnmpTrap..:%s\n",sql);
  ret=sqlite3_step(stmt);
 
  while (ret==SQLITE_ROW)
  {
    //strcpy(inbuf,sqlite3_column_text(stmt,2));
    //printf("sendSnmpTrap..:%d\n",n);
    if (n<-1)
    {
      if (entity>0)
          sprintf(entity_name,"%s %d",sqlite3_column_text(stmt,3),entity);
      else
          sprintf(entity_name,"%s",sqlite3_column_text(stmt,3));
      if (slot>0)
         //sprintf(ut8buf,"单元盘 #%d %s %s",slot,entity_name,sqlite3_column_text(stmt,2));
         sprintf(ut8buf,"Unit #%d %s %s",slot,entity_name,sqlite3_column_text(stmt,2));
      else
         sprintf(ut8buf,"%s %s",entity_name,sqlite3_column_text(stmt,3));
      /*ret=u2g(ut8buf, strlen(ut8buf), gbkbuf,60);
      if (ret)
      {
        strcpy(gbkbuf,ut8buf);
      }
      */
    }

    n++;
    //doSystem("snmptrap -m \"\" -v 2c -c public %s:%d ' ' 1.3.6.1.4.1.10215.2.1.9.%d.%d.%d.%d 1.3.6.1.4.1.0 x '%s'",sqlite3_column_text(stmt,0),sqlite3_column_int(stmt,1),alarm_code,chassis,slot,entity,gbkbuf);
    doSystem("snmptrap -m \"\" -v 2c -c public %s:%d ' ' 1.3.6.1.4.1.10215.2.1.9.%d  1.3.6.1.2.1.92.1.3.1.1.8.0 s '%d %d %d %d' 1.3.6.1.2.1.1.6.0 s '%s'",sqlite3_column_text(stmt,0),sqlite3_column_int(stmt,1),alarm_code,chassis,slot,entity,alarm_code,ut8buf);
    printf("snmptrap -m \"\" -v 2c -c public %s:%d ' ' 1.3.6.1.4.1.10215.2.1.9.%d  1.3.6.1.2.1.92.1.3.1.1.8.0 s '%d %d %d %d' 1.3.6.1.2.1.1.6.0 x '%s'",sqlite3_column_text(stmt,0),sqlite3_column_int(stmt,1),alarm_code,chassis,slot,entity,alarm_code,gbkbuf);
    //printf("snmptrap -v 2c -c public %s:%d ' ' 1.3.6.1.4.1.10215.2.1.9.%d.%d.%d.%d 1.3.6.1.4.1.0 s 'unit changed'",sqlite3_column_text(stmt,0),sqlite3_column_int(stmt,1),alarm_code,chassis,slot,entity);
    ret=sqlite3_step(stmt);
  }
  sqlite3_finalize(stmt);
  sqlite3_close(db);
}

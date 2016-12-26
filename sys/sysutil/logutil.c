#include <stdio.h>
#include <string.h>
#include <unit.h>
#include <sqlite3.h>
#include <dbutil.h>
#include <log.h>

int insertOperateLog(int group_id,int object_id,int user_id,int mode,int operate_type)
{
  char sql[160];

  sprintf(sql,"DELETE FROM OptLog WHERE ID NOT IN (SELECT ID FROM OptLog order by ID LIMIT %d)",MAX_LOG_COUNT);
  if (SQLITE_OK!=execSql(NULL, sql,NULL,NULL))
        return -1;

  sprintf(sql,"INSERT INTO \"OptLog\" VALUES(NULL,%d,%d,%d,%d,%d,datetime('now','localtime'));",group_id,object_id,user_id,mode,operate_type);
  if (SQLITE_OK!=execSql(NULL, sql,NULL,NULL))
       return -1;
  else
       return 0;
}


int deleteOperateLogById(int id)
{
  char sql[50]="DELETE FROM OptLog";

  if (id!=0)
      sprintf(sql,"DELETE FROM OptLog WHERE ID=%d;",id);
  //printf("sql=%s\n",sql);
  if (SQLITE_OK!=execSql(NULL, sql,NULL,NULL))
        return -1;
  else
        return 0;
 //printf("sql=%s\n",sql);
}
/*
int deleteOperateLogByIp(char * ip)
{
  char sql[60];
  sprintf(sql,"DELETE FROM \"Community\" WHERE ipadd='%s';",ip);
  if (SQLITE_OK!=execSql(NULL, sql,NULL,NULL))
        return -1;
  else
        return 0;
}
*/


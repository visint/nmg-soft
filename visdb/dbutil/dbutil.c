#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <unit.h>
#include <sqlite3.h>
#include <dbutil.h>
//gcc -o /usr/local/bin/dbutil.o dbutil.c  -I../include
//gcc -c dbutil.c  -I../../include && ar -r libdbutil.a dbutil.o

sqlite3 *openDB(const char * filename)
{
  sqlite3 *db=NULL;
  //printf("db name:%s\n",filename);
  if (sqlite3_open(filename, &db) != SQLITE_OK)
  {
    sqlite3_close(db);
    return NULL;
  }
  sqlite3_busy_timeout(db,300);
  return db;
}

int execSql(
sqlite3 *db,
const char *sql,
int (*callback)(void*,int,char**,char**),  /* Callback function */
void *arg                                     /* 1st argument to callback */
)
{
  //sqlite3 *db=NULL;
  //char *zErrMsg = 0;
  char *errmsg;
  int rc=0;
  boolean_t auto_open=TRUE;
 
  if (db!=NULL)
  {
    auto_open=FALSE;
  }
  else
      db=openDB(DB_FILE_NAME);
  if (db==NULL)
    return -1;
 
  rc=sqlite3_exec( db ,sql,callback , arg , &errmsg);
  if (rc != SQLITE_OK)
  {
    printf("sql:%s\nerror when run sql_exec:%s\n",sql,errmsg);
    rc=-1;
  }
  //printf("sql:%s\nerror when run sql_exec:%s\n",sql,errmsg);
  if (auto_open!=FALSE)
    sqlite3_close(db); //关闭数据库
  return rc;
}

static int onFetchRowCount( void *para, int n_column, char **column_value, char **column_name)
{
  if (para!=NULL)
  {
   int *nrow=(int *)para;
    *nrow=atoi(column_value[0]);
  }
  return 0;
}
int getRowCount(sqlite3 *db,char *sql)
{
  //char *zErrMsg = 0;
  int nrow=0;
  if (strstr(sql,"count") || strstr(sql,"COUNT"))
  {
    execSql(db, sql,onFetchRowCount,&nrow);
  }
  else
  {
    char buf[512]="";
    sprintf(buf,"SELECT COUNT(*) FROM (%s)",sql);
    //printf("sql=:%s\n",sql);
    execSql(db,buf,onFetchRowCount,&nrow);
  }
  return nrow;
}
int getColIntValue(sqlite3 *db,char *sql,int *value)
{
  sqlite3 *conn;
  sqlite3_stmt* stmt;
  int ret;
  //db=openDB(DB_FILE_NAME);
  if (NULL!=db)
     conn=db;
  else
      conn=openDB(DB_FILE_NAME);
  if (conn==NULL)
     return -1;
  ret=sqlite3_prepare_v2(conn,sql,strlen(sql),&stmt,NULL);
  if(ret!=SQLITE_OK)
  {
    // printf("%s\n",sqlite3_errmsg(conn)); 
     sqlite3_close(conn);
     return -1;
  }
  ret=sqlite3_step(stmt);
  if (ret==SQLITE_ROW)
  {
      *value=sqlite3_column_int(stmt,0);
     //printf("value=%d\n",sqlite3_column_int(stmt,0));
  }
  else
  {
     ret=0;
     //*value=0;
     //printf("value not found!\n");
  }
  sqlite3_finalize(stmt);
  if (NULL==db)
     sqlite3_close(conn);
  return ret;
}
static int onColValue(void *para, int n_column, char **column_value, char **column_name)
{
  if (para!=NULL)
  {
    char *value= (char *)para;
    strcpy(value,column_value[0]);
  }
  return 0;
}

boolean_t  getColValue(sqlite3 *db,char *sql,int row,int col, char *value)
{
     execSql(db, sql,onColValue,value);
}
static int onFetchTable( void *para, int n_column, char **column_value, char **column_name)
{
  int n;
  for (n=0;n<n_column;n++)
    printf("%s:%s\n",column_name[n],column_value[n]);
  return 0;
}
void showTable(sqlite3 *db,char * sql)
{
  char buf[100]=""; 
  printf("Show table.sql=%s\n",sql);
  if (strstr(sql,"select")||strstr(sql,"SELECT"))
    execSql(db, sql,onFetchTable,NULL);
  else
   {
     sprintf(buf,"SELECT * FROM %s",sql);
     execSql(db, buf,onFetchTable,NULL);
   }
}

static int onFetchUserData( void *para, int n_column, char **column_value, char **column_name)
{
 
}
/*********************************************************************************
* 函数名称： getUserDataFromDB
* 功能描述： 从数据库中获取单元盘的基本信息。 
* 访问的表： UnitUD
* 修改的表： 无
* 输入参数： uchar_t chassis,uchar_t slot,unit_user_data_t *udata
* 输出参数： unit_user_data_t *udata
* 返 回 值： 成功返回TRUE,错误返回FALSE
* 其它说明： 
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/10/18        V1.0      李文峰        创建
************************************************************************************/
boolean_t getUserDataFromDB(uchar_t chassis,uchar_t slot,unit_user_data_t *udata)
{
  int ret;
  sqlite3_stmt* stmt;
  sqlite3 *db;
  char sql[100];
  
  sprintf(sql,"SELECT unit_name,contact,location,up_device,user_data,service FROM UnitUD WHERE slot=%d Limit 1",slot);

  db=openDB(DB_FILE_NAME);
  if (NULL==db)
     return FALSE;
  ret=sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL);
  if(ret!=SQLITE_OK)
  {
     printf("getUserDataFromDB Error:%s\n",sqlite3_errmsg(db));
     sqlite3_close(db);
     return FALSE;
  }
  ret=sqlite3_step(stmt);
  //int cols=sqlite3_column_count(stmt);
  int count = 0;
  if (ret==SQLITE_ROW)
  {
    memset(udata,0,sizeof(unit_user_data_t));
    strncpy(udata->name,sqlite3_column_text(stmt,0),30);
    strncpy(udata->contact,sqlite3_column_text(stmt,1),30);
    strncpy(udata->location,sqlite3_column_text(stmt,2),30);
    strncpy(udata->updev,sqlite3_column_text(stmt,3),30);
    strncpy(udata->userdata,sqlite3_column_text(stmt,4),30);
    strncpy(udata->service,sqlite3_column_text(stmt,5),30);
    count++;
  }
  sqlite3_finalize(stmt);
  sqlite3_close(db);
  if (count>0)
    return TRUE;
  else
   return FALSE;
}

boolean_t getUserDataByGbk(uchar_t chassis,uchar_t slot,unit_user_data_t *udata)
{
  unit_user_data_t ud;
  if (TRUE!=getUserDataFromDB(chassis,slot,udata))
     return FALSE;
  memcpy(&ud,udata,sizeof(unit_user_data_t));
  u2g(ud.name, strlen(ud.name), udata->name, 30);
  u2g(ud.contact, strlen(ud.contact), udata->contact, 30);
  u2g(ud.location, strlen(ud.location), udata->location, 30);
  u2g(ud.updev, strlen(ud.updev), udata->updev, 30);
  u2g(ud.userdata, strlen(ud.userdata), udata->userdata, 30);
  u2g(ud.service, strlen(ud.service), udata->service, 30);
  return TRUE;
}

/*********************************************************************************
* 函数名称： setUserDataItemToDB
* 功能描述： 把单元盘的基本信息项保存到数据库。 
* 访问的表： UnitUD
* 修改的表： UnitUD
* 输入参数： uchar_t chassis,int item_id,uchar_t slot,char *udata
* 输出参数： unit_user_data_t *udata
* 返 回 值： 成功返回TRUE,错误返回FALSE
* 其它说明： 
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/10/18        V1.0      李文峰        创建
************************************************************************************/
boolean_t setUserDataItemToDB(uchar_t chassis,uchar_t slot,int item_id,const char *udata)
{
  char sql[100]="";
  char field_name[12];
  switch (item_id)
  {
    case UNIT_NAME_ID:
          sprintf(field_name,"unit_name");
         break;
    case UNIT_CONTACT_ID:
         sprintf(field_name,"contact");
         break;
    case UNIT_LOC_ID:
         sprintf(field_name,"location");
         break;
    case UNIT_UP_DEV_ID:
         sprintf(field_name,"up_device");
         break;
    case UNIT_USER_DATA_ID:
         sprintf(field_name,"user_data");
         break;
     case UNIT_SERVICE_ID:
         sprintf(field_name,"service");
         break;
    default:
       return FALSE;
  }
  sprintf(sql,"UPDATE UnitUD SET %s='%s' WHERE slot=%d",field_name,udata,slot);
  if (execSql(NULL , sql,NULL,NULL)!=-1)
     return TRUE;
  else
     return FALSE;
}

boolean_t setGbkUserDataItemToDB(uchar_t chassis,uchar_t slot,int item_id,const char *udata)
{
  char buf[80]="";
  if (!g2u(udata, strlen(udata), buf, 30))
     return setUserDataItemToDB(chassis,slot,item_id,buf);
  else
     return setUserDataItemToDB(chassis,slot,item_id,udata);
}

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <public.h>
#include <unit.h>
#include <sqlite3.h>
#include <dbutil.h>
#include <log.h>

//gcc -o /usr/local/bin/dbm -lvisipc -lvisdb ../vislib/unit.o ../vislib/visstr.o dbman.c  -I../include
static void showUserData(uchar_t chassis,uchar_t slot)
{
  unit_user_data_t udata;
  if (TRUE!=getUserDataFromDB(chassis,slot,&udata))
    return;
  
}
static void usage()
{
  fprintf(stderr,"DB manager ver 1.0.2\n");
  
  printf("Option Specification:\n");
  printf("-b [<1..16>]: show unit base info\n");
  printf("-u [1..16]: show unit user info\n");
  printf("-t table_name: show table\n");
  printf("-m : show info item\n");
  printf("-e <1..16>: show unit status\n");
  printf("-i <infoset id>: show infoset status\n");
  printf("-a : show all\n");
  printf("-C : create DB\n");
  printf("-h for help.\n");
}

static void createUnitBITable(uchar_t classis)
{
  int n;
  sqlite3 *db=NULL;
  char *zErrMsg = 0;
  char sql[] = " CREATE TABLE UnitBI( \
  ID INTEGER PRIMARY KEY, \
  chassis TINYINT, \
  slot TINYINT, \
  property VARCHAR(32), \
  sn VARCHAR(16), \
  model   VARCHAR(32),\
  creation VARCHAR(12),\
  fwver VARCHAR(12),\
  hwver VARCHAR(12)\
  );";

execSql(NULL , "DROP TABLE UnitBI",NULL,NULL);
execSql(NULL , sql,NULL,NULL);


//插入数据
for (n=0;n<=MAX_UNIT_COUNT;n++)
{
 sprintf(sql,"INSERT INTO \"UnitBI\" VALUES(NULL,%d,%d,'OTU111-55-1470-11-%d','SN1--2012-12-%d', 'Model-123-12-%d','2012-12-%d','FV-1.1.1','HV1.1.1');",classis,n,n,n,n,n,n );
 execSql(NULL, sql,NULL,NULL);
}
sprintf(sql,"INSERT INTO \"UnitBI\" VALUES(NULL,%d,16,'NMU111-11-11','SN1--2012-12-%d', 'Model-123-12-%d','2012-12-%d','%s','HV1.1.1');",classis,n,n,n,NMU_FW_VER);
 execSql(NULL, sql,NULL,NULL);
 return;
}
static void createUnitUDTable()
{
  int n;
  char sql[] = " CREATE TABLE UnitUD( \
  ID INTEGER PRIMARY KEY, \
  chassis TINYINT, \
  slot TINYINT, \
  unit_name VARCHAR(32), \
  contact VARCHAR(32), \
  location VARCHAR(32), \
  up_device VARCHAR(32), \
  user_data   VARCHAR(32),\
  service   VARCHAR(32)\
  );";
  execSql(NULL,"DROP TABLE UnitUD;",NULL,NULL);
  execSql(NULL,sql,NULL,NULL);
  //插入数据
 for (n=0;n<=MAX_UNIT_COUNT;n++)
 {
   sprintf(sql,"INSERT INTO \"UnitUD\" VALUES(NULL,%d,%d,'This is unit %d Unit', \
      'This is unit %d contact', 'This is unit %d loc','This is unit %d up device','This is unit %d user data','This is uint #%d open service');",0,n,n,n,n,n,n,n);
   execSql(NULL, sql,NULL,NULL);
 }
 sprintf(sql,"INSERT INTO \"UnitUD\" VALUES(NULL,%d,16,'This is NMU', \
      'This is NMU contact', 'This is NMU loc','This is NMU up device','This is NMU user data','This is NMU open service');",0);
   execSql(NULL, sql,NULL,NULL);
}

static void insertAlarmType(int alarm_code,char *alarm_name,char *alarm_name_en,char *entity_name,char *entity_name_en,uchar_t severity)
{
  char sql[260];
  
  sprintf(sql,"INSERT INTO \"AlarmType\" VALUES(%d,'%s','%s','%s','%s',%d,1,'');",alarm_code,alarm_name,alarm_name_en,entity_name,entity_name_en,severity);
  execSql(NULL, sql,NULL,NULL);
}
/*
void insertAlarmLog(int alarm_code,char chassis,char slot,char entity)
{
  char sql[200];
  int count;
  
  sprintf(sql,"SELECT alarm_code FROM AlarmType WHERE alarm_code=%d AND enable=1 AND mask_slot NOT LIKE '%c%d;%c' LIMIT 1;",alarm_code,'%',slot,'%');
  
  if (getRowCount(NULL,sql) >0 && slot>=0 && slot <=MAX_UNIT_COUNT)
  {
    sprintf(sql,"INSERT INTO \"AlarmLog\" VALUES(NULL,%d,%d,%d,%d,0,datetime('now'));",alarm_code,chassis,slot,entity);
    execSql(NULL, sql,NULL,NULL);
  }
  printf("sql:%s\n",sql,NULL,NULL);
}
*/
static void createAlarmTable()
{
  int n;
  char sql[250] = " CREATE TABLE AlarmType( \
  alarm_code SMALLINT PRIMARY KEY, \
  alarm_name VARCHAR(32), \
  alarm_name_en VARCHAR(32), \
  entity_name VARCHAR(16), \
  entity_name_en VARCHAR(16), \
  severity   TINYINT,\
  enable TINYINT, \
  mask_slot VARCHAR(46) \
  );";
  execSql(NULL,"DROP TABLE AlarmType;",NULL,NULL);
  execSql(NULL,sql,NULL,NULL);
  insertAlarmType(1,"进程重启","Process restart","NMU","NMU",5);
  insertAlarmType(3,"CPU超出阈值","CPU exceed threhold","NMU","NMU",1);
  insertAlarmType(5,"内存超出阈值","Memory exceed threhold","NMU","NMU",1);

  insertAlarmType(21,"电源关闭","Power off","电源","Power supply",1);
  insertAlarmType(22,"电源打开","Power on","电源","Power supply",5);
  insertAlarmType(23,"电压超出阈值","Volt exceed threhold","电源","Power supply",2);
  insertAlarmType(24,"电压恢复正常值","Voltage recovery","电源","Power supply",5);
  insertAlarmType(25,"风扇停转","Fan not working","风扇","Fan",1);
  insertAlarmType(26,"风扇运转","Fan working","风扇","Fan",5);
  insertAlarmType(27,"机箱温度超出阈值","Chassis temp exceed threhold","机箱","Chassis",2);
  insertAlarmType(28,"机箱温度恢复正常值","Chassis temp return to normal","机箱","Chassis",5);
  
  insertAlarmType(31,"接收光功率超出阈值","Rx power exceed threhold","光口","Fiber port",2);
  insertAlarmType(32,"接收光功率恢复正常","Rx power return to normal","光口","Fiber port",5);
  insertAlarmType(33,"发送光功率超出阈值","Tx power exceed threhold","光口","Fiber port",2);
  insertAlarmType(34,"发送光功率恢复正常","Tx power return to normal","光口","Fiber port",5);
  insertAlarmType(35,"偏置电流超出阈值","Bais exceed threhold","光口","Fiber port",3);
  insertAlarmType(36,"偏置电流恢复正常","Bais return to normal","光口","Fiber port",5);
  insertAlarmType(37,"电压超出阈值","Voltage exceed threhold","光口","Fiber port",3);
  insertAlarmType(38,"电压恢复正常","Voltage return to normal","光口","Fiber port",5);
  insertAlarmType(39,"温度超出阈值","Temperature exceed threhold","光口","Fiber port",3);
  insertAlarmType(40,"温度恢复正常","Temperature return to normal","光口","Fiber port",5);

  insertAlarmType(51,"单元盘脱位","Unit removed","单元盘","Unit",1);
  insertAlarmType(52,"单元盘在位","Unit inserted","单元盘","Unit",5);
  
  insertAlarmType(71,"光模块脱位","Fiber module inserted","光口","Chassis",1);
  insertAlarmType(72,"光模块在位","Fiber module removed","光口","Chassis",5);
  insertAlarmType(73,"接收无光","Rx LOS","光口","Fiber port",1);
  insertAlarmType(74,"接收有光","Rx return to normal","光口","Fiber port",5);
  insertAlarmType(75,"发送无光","Tx LOS","光口","Fiber port",1);
  insertAlarmType(76,"发送有光","Tx return to normal","光口","Fiber port",5);
  insertAlarmType(77,"发送关闭","Tx disable","光口","Fiber port",4);
  insertAlarmType(78,"发送开启","Tx enable","光口","Fiber port",5);


  insertAlarmType(401,"OLP工作模式切换为手动","OLP mode switch to manual","OLP","OLP",4);
  insertAlarmType(402,"OLP工作模式切换为自动","OLP mode switch to auto","OLP","OLP",5);
  insertAlarmType(403,"OLP切换到备路(L2)","OLP mode switch to L2","OLP","OLP",4);
  insertAlarmType(404,"OLP切换到主路(L1)","OLP mode switch to L1","OLP","OLP",5);
  insertAlarmType(409,"OLP R1光功率偏低","R1 power relatively low","OLP","OLP",4);
  insertAlarmType(410,"OLP R1光功率恢复正常","R1 power return to normal","OLP","OLP",5);
  insertAlarmType(411,"OLP R2光功率偏低","R2 power relatively low","OLP","OLP",4);
  insertAlarmType(412,"OLP R2光功率恢复正常","R2 power return to normal","OLP","OLP",5);
  insertAlarmType(413,"OLP TX光功率偏低","Tx power relatively low","OLP","OLP",4);
  insertAlarmType(414,"OLP TX光功率恢复正常","Tx power return to normal","OLP","OLP",5);
  
  
  strcpy(sql," CREATE TABLE AlarmLog( \
  ID INTEGER PRIMARY KEY, \
  alarm_code INTEGER,\
  chassis TINYINT, \
  slot TINYINT, \
  entity TINYINT, \
  alarm_status TINYINT, \
  alarm_time  DATETIME DEFAULT (datetime(CURRENT_TIMESTAMP,'localtime')) \
  );");
  execSql(NULL,"DROP TABLE AlarmLog;",NULL,NULL);
  execSql(NULL,sql,NULL,NULL);
  /*
  insertAlarmLog(1,0,0,1);
  insertAlarmLog(3,0,0,2);
  insertAlarmLog(11,0,1,0);
  insertAlarmLog(21,0,1,1);
  insertAlarmLog(23,0,1,2);
  
  insertAlarmLog(61,0,12,0);
  insertAlarmLog(63,0,8,0);
  */
}

static void createSnmpTable()
{
  char sql[250] = " CREATE TABLE Community( \
  ID INTEGER PRIMARY KEY, \
  rocommunity VARCHAR(16), \
  rwcommunity VARCHAR(16), \
  ipadd VARCHAR(24), \
  port INTEGER ,\
  read_only TINYINT,\
  enable TINYINT\
  );";
  execSql(NULL,"DROP TABLE Community;",NULL,NULL);
  execSql(NULL,sql,NULL,NULL);
  //insertCommunity("public","private","",2,1);
  execSql(NULL,"DROP TABLE DestHost;",NULL,NULL);
  execSql(NULL,"CREATE TABLE DestHost( \
  ID INTEGER PRIMARY KEY, \
  dest_ipadd VARCHAR(24), \
  udp_port INTEGER,\
  enable TINYINT);",NULL,NULL);
  execSql(NULL,"DROP TABLE AgentAddr;",NULL,NULL);
  execSql(NULL,"CREATE TABLE AgentAddr( \
  ID INTEGER PRIMARY KEY, \
  agent_ipadd VARCHAR(24), \
  udp_port INTEGER,\
  udp6_port INTEGER,\
  enable TINYINT);",NULL,NULL);
}

static void insertOptObjGroup(int group_id,char *group_name)
{
  char sql[200];
  
  sprintf(sql,"INSERT INTO \"OptObjGroup\" VALUES(%d,'%s',1);",group_id,group_name);
  execSql(NULL, sql,NULL,NULL);
}

static void insertOptObj(int group_id,int object_code,char *object_name)
{
  char sql[100];
  
  sprintf(sql,"INSERT INTO \"OptObj\" VALUES(NULL,%d,%d,'%s');",group_id,object_code,object_name);
  execSql(NULL, sql,NULL,NULL);
}

static void createSysLogTable()
{
  char sql[]=" CREATE TABLE SysLog( \
  ID INTEGER PRIMARY KEY, \
  msg VARCHAR(32), \
  occured_time  DATETIME DEFAULT (datetime(CURRENT_TIMESTAMP,'localtime')) \
  );";
  execSql(NULL,"DROP TABLE SysLog",NULL,NULL);
  execSql(NULL,sql,NULL,NULL);
}
static void createLogTable()
{
  execSql(NULL,"DROP TABLE OptObj;",NULL,NULL);
  execSql(NULL,"DROP TABLE OptLog;",NULL,NULL);
  execSql(NULL,"DROP TABLE OptObjGroup;",NULL,NULL);
  execSql(NULL," CREATE TABLE OptLog( \
  ID INTEGER PRIMARY KEY, \
  group_id INTEGER,\
  object_id INTEGER,\
  user_id TINYINT,\
  mode TINYINT,\
  operate_type TINYINT,\
  operate_time  DATETIME DEFAULT (datetime(CURRENT_TIMESTAMP,'localtime'))\
  );",NULL,NULL);
  execSql(NULL,"CREATE TABLE OptObjGroup( \
  ID INTEGER PRIMARY KEY, \
  group_name VARCHAR(16),\
  enable TINYINT \
  );",NULL,NULL);

  execSql(NULL,"CREATE TABLE OptObj( \
  ID INTEGER PRIMARY KEY, \
  group_id INTEGER,\
  object_code TINYINT,\
  object_name VARCHAR(16));",NULL,NULL);

  insertOptObjGroup(LOG_GROUP_SYS_IP_CONF,"IP地址配置");
  insertOptObjGroup(LOG_GROUP_SYS_SYS_INFO_CONF,"系统信息");
  insertOptObjGroup(LOG_GROUP_SYS_USER_CONF,"用户管理");
  insertOptObjGroup(LOG_GROUP_SYS_PASSWORD_CONF,"用户密码");
  insertOptObjGroup(LOG_GROUP_SYS_USER_ACL_CONF,"用户访问控制");
  insertOptObjGroup(LOG_GROUP_SYS_USER_GROUP_CONF,"用户组管理");
  insertOptObjGroup(LOG_GROUP_SYS_SNMP_COMMINITY_CONF,"SNMP 配置");
  insertOptObjGroup(LOG_GROUP_SYS_SNMP_DEST_HOST_CONF,"告警目标主机");
  insertOptObjGroup(LOG_GROUP_SYS_CURRENT_ALARM_CONF,"当前告警");
  insertOptObjGroup(LOG_GROUP_SYS_HISTORY_ALARM_CONF,"历史告警");
  insertOptObjGroup(LOG_GROUP_SYS_SNMP_ALARM_TYPE_CONF,"告警类型");
  insertOptObjGroup(LOG_GROUP_SYS_OPT_LOG_CONF,"操作日志");

  insertOptObjGroup(LOG_UNIT_OTU_CONF,"OTU信息");
  insertOptObjGroup(LOG_UNIT_OLP_CONF,"OLP信息");
  createSysLogTable();
}


static boolean_t insertInfoSet(uchar_t infoset_id,char infoset_type,const char *iset_name,
                             char *include_class,char *exclude_type,char item_count)
{
  int n;
  char sql[160];
  sprintf(sql,"SELECT ID FROM InfoSet WHERE infoset_id=%d AND include_class like '%c%s%c'",infoset_id,'%',include_class,'%');
  
  n=getRowCount(NULL,sql);
  printf("count=%d sql=%s\n",n,sql);
  if (n>0)
  {
    printf("InfoSet exist\n");
    return FALSE;
  }
  //printf("count=%d sql=%s\n",n,sql);
  //return TRUE;
  sprintf(sql,"INSERT INTO \"InfoSet\" VALUES(NULL,'%d','%d','%s','%s','%s',%d);"
          ,infoset_id,infoset_type,include_class,exclude_type,iset_name,item_count);
  printf("count=%d sql=%s\n",n,sql);
  execSql(NULL,sql,NULL,NULL);
  return TRUE;
}
static boolean_t createInfoSetTable()
{
  char sql[] = " CREATE TABLE InfoSet( \
  ID INTEGER PRIMARY KEY, \
  infoset_id SMALLINT, \
  infoset_type TINYINT, \
  include_class VARCHAR(12), \
  exclude_type  VARCHAR(12), \
  infoset_name VARCHAR(20), \
  item_count INT1 \
  );";
  execSql(NULL,"DROP TABLE InfoSet;",NULL,NULL);
  execSql(NULL,sql,NULL,NULL);
  insertInfoSet(1,3,"单元盘基本信息","0","",3);
  insertInfoSet(2,3,"单元盘用户信息","0","",6);
  insertInfoSet(3,1,"系统信息","1","",6); //电源、风扇状态信息
  insertInfoSet(4,2,"温度、电压告警阈值","1","",8);
  insertInfoSet(11,2,"光口%sDDM诊断信号","1;3;","1:3;",5);/*支持的单元盘:收发器、OTU*,但不支持IP 113收发器*/

  insertInfoSet(OLP_STATUS_INFOSET_ID,1,"OLP(1+1)状态信息","4;","4:2;",4); /*1+1 OLP*/
  insertInfoSet(OLP_STATUS_INFOSET_ID,1,"OLP(1:1)状态信息","4;","4:1;",2); /*1:1 OLP*/
  insertInfoSet(OLP_DATA_INFOSET_ID,2,"OLP(1+1)光功率信息","4;","4:2;",7);  /*1+1 OLP*/
  insertInfoSet(OLP_DATA_INFOSET_ID,2,"OLP(1:1)光功率信息","4;","4:1;",6);  /*1:1 OLP*/

  insertInfoSet(OPORT_INFOSET_ID,1,"光口%s状态","1;3;","",4);      /*支持的单元盘:收发器、OTU*/
  insertInfoSet(22,1,"收发器电口信息","1;","",5);
  return TRUE;
}

static boolean_t insertInfoItem(uchar_t infoset_id,uchar_t unit_class,uchar_t unit_type,char item_id,
                 const char *item_name,const char *status_name,char *exclude_type,char readonly)
{
  int n;
  char sql[120];
 
  char value[16]="";
  /*column_t col;

  col.max_row=2;
  col.col_size=16;
  col.cols=(char *)cols;
  col.row=0;
  */
  //sprintf(sql,"SELECT ID FROM InfoSet WHERE infoset_id=%d AND unit_class=%d AND unit_type=%d",infoset_id,unit_class,unit_type);
  sprintf(sql,"SELECT ID FROM InfoSet WHERE infoset_id=%d AND include_class LIKE '%c%d;%c' AND NOT(exclude_type LIKE '%c%d:c%d;%c') LIMIT 1;",infoset_id,'%',unit_class,'%','%',unit_class,unit_type,'%');
  //n=get_col(NULL,sql,&col);
  getColValue(NULL,value);
  if (strlen(value)<1)
  {
    printf("InfoSet %d Not exist\nsql:%s\n",infoset_id,sql);
    return FALSE;
  }
  sprintf(sql,"INSERT INTO \"InfoItem\" VALUES(NULL,%s,%d,'%s','%s','%s',%d);"
          ,value,item_id,item_name,status_name,exclude_type,readonly);
  execSql(NULL,sql,NULL,NULL);
  return TRUE;
}
static boolean_t createInfoItemTable()
{
  //int n;
  sqlite3 *db=NULL;
  //char *zErrMsg = 0;
  int rc;
  char sql[] = " CREATE TABLE InfoItem( \
  ID INTEGER PRIMARY KEY, \
  PID INTEGER, \
  item_id SMALLINT, \
  item_name VARCHAR(20), \
  status_name VARCHAR(40), \
  exclude_type VARCHAR(8), \
  readonly INT1 \
  );";
  execSql(NULL,"DROP TABLE InfoItem;",NULL,NULL);
  execSql(NULL,sql,NULL,NULL);
  insertInfoItem(21,3,1,1,"接收","1=有光;2=无光","0",0);
  insertInfoItem(21,3,1,2,"发送","1=有光;2=无光","0",0);
  insertInfoItem(21,3,1,2,"发送关断","1=启用;2=禁用","0",0);
  insertInfoItem(21,3,1,3,"LOS","1=有信号;2=无信号","0",0);

  /*insertInfoItem(11,3,1,2,"接收光功率","1=无光;2=有光","0",0);
  insertInfoItem(11,3,1,2,"发送光功率","1=无光;2=有光","0",0);
  insertInfoItem(11,3,1,2,"电压","1=无光;2=有光","0",0);
  insertInfoItem(11,3,1,2,"电压","1=无光;2=有光","0",0);
  */
 return TRUE;
}
static int onFetchColValue( void *para, int n_column, char **column_value, char **column_name)
{
  if (strcmp(column_value[0],"0"))
    printf("Unit #%s",column_value[0]);
    printf("\t");
  if (strcmp(column_value[1],"0"))
    printf(" %s %s",column_value[5],column_value[1]);
    printf("\t");
  printf("%s\t",column_value[4]);
  if (strcmp(column_value[3],"1"))
     printf("unclean unack\t");
  else if (strcmp(column_value[3],"2"))
     printf("unclean ack\t");
  printf("%s\n",column_value[2]);
  return 0;
}
void showAlarm()
{
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
  char sql[]="SELECT B.slot,B.entity,B.alarm_time, B.alarm_status,A.alarm_name,A.entity_name FROM AlarmType AS A,AlarmLog AS B WHERE A.alarm_code=B.alarm_code";
  rc = sqlite3_open(DB_FILE_NAME, &db);  
  
  rc = sqlite3_exec(db, sql, onFetchColValue, NULL, &zErrMsg); 
  printf("zErrMsg:%s\n",zErrMsg);
  sqlite3_close(db);
}
static int onFetchTable( void *para, int n_column, char **column_value, char **column_name)
{
  int n;
  for (n=0;n<n_column;n++)
  printf("%s:%s\n",column_name[n],column_value[n]);
  return 0;
}
/*
void showTable(char * table_name)
{
  sqlite3 *db=NULL;
  char *zErrMsg = 0;
  int rc;
  int i,j;
  int index=0;
  char cols_width[12]={5,5,12,16,16,12,12,12,5,10,10,10};
  int nrow = 0, ncol = 0;
  char **dbResult;
  char sql[100];
  rc = sqlite3_open(DB_FILE_NAME, &db); 
  if( rc )
  {
   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
   sqlite3_close(db);
   return;
  }
  sprintf(sql,"SELECT * FROM %s",table_name);
  rc = sqlite3_exec(db, sql,  onFetchTable, NULL, &zErrMsg); 
  if (rc != SQLITE_OK)
  {
    printf("zErrMsg:%s\n",zErrMsg);
    //printTable("满足条件的信息项","table_name",dbResult,dbResult+ncol,nrow,ncol,cols_width);
  }
 sqlite3_close(db);
}
*/
static int onFetchInfoset( void *para, int n_column, char **column_value, char **column_name)
{
  int n;
  for (n=0;n<n_column;n++)
    printf("%s:%s\n",column_name[n],column_value[n]);
  return 0;
}
void showInfoset(int uclass,int utype)
{
  sqlite3 *db=NULL;
  char *zErrMsg = 0;
  int rc;

  char sql[100];
  rc = sqlite3_open(DB_FILE_NAME, &db); 
  if( rc )
  {
   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
   sqlite3_close(db);
   return;
  }
  sprintf(sql,"SELECT * FROM InfoSet WHERE include_class like '%c%d;%c' AND exclude_type NOT LIKE '%c%d:%d;%c';",'%',uclass,'%','%',uclass,utype,'%');
  printf("sql:%s\n",sql);
  rc = sqlite3_exec(db, sql,  onFetchInfoset, NULL, &zErrMsg); 
  //printf("zErrMsg:%s\n",zErrMsg);
  sqlite3_close(db);
}

void createDB()
{
  createUnitBITable(0);
  createUnitUDTable();
  createInfoSetTable();
  createInfoItemTable();
  createAlarmTable();
  createSnmpTable();
  createLogTable();
}

int dbman (int argc, char **argv)
{
    int oc,chassis=0,slot=0,infoset_id=0,item_id=0,item_value=0;                    
    char opt=0,rw=-1,field_value[32]="",*b_opt_arg;           

    while((oc = getopt(argc, argv, "abc:f:i:n:s:tuACD:I:GS")) != -1)
    {
        switch(oc)
        {
            case 'a':
                opt='a';
                break;
            case 'c':
                chassis=atoi(optarg);
                printf("Chassis is %s.\n",optarg);
                break;
            case 'n':
                b_opt_arg = optarg;
                strncpy(field_value, optarg,30);
                printf("Unit name is %s\n", optarg);
                break;
            case 's':
                b_opt_arg = optarg;
                slot=atoi(optarg);
                printf("Unit is #%s\n", optarg);
                break;
            case 'f':
                b_opt_arg = optarg;
                infoset_id=atoi(optarg);
                printf("infoset id is %s\n", optarg);
                break;
            case 'i':
                b_opt_arg = optarg;
                item_id=atoi(optarg);
                //printf("Unit data item id is %s\n", optarg);
                break;
          case 'u':
                printf("Set user data.\n");
                opt='u';
                break;
          case 'G':
                printf("Get user data.\n");
                rw='G';
                break;
          case 'S':
                printf("Set user data.\n");
                rw='S';
                break;
          case 'C':
                printf("Create DB.\n");
                createDB();
                return 0;
          case 'D':
                printf("Show table.\n");
                showTable(NULL,optarg);
                return 0;
          case 'A':
                printf("Show Alarm.\n");
                showAlarm();
                return 0;
         case 'I':
                printf("Show Infoset.\n");
                showInfoset(atoi(optarg),1);
                return 0;
          case '?':
                //ec = (char)optopt;
                printf("无效的选项字符 \' %c \'!\n", (char)optopt);
                usage();
                return 0;
          case ':':
                printf("缺少选项参数！\n");
                usage();
                return 0;

        }
    }
  printf("opt is %c rw=%c\n", opt,rw);
  switch(opt)
  {
     case 'u':
         if (rw=='G')
         {
           printf("Get user data.\n");
         }
         else if (rw=='S')
         {
            if (FALSE!=setUserDataItemToDB(chassis,slot,item_id,field_value))
              printf("Set user data OK.\n");
         }
         else
             usage();
     break;
     case 'a':
              //dbm -a -s 4 -f 21 -i 1
             insertAlarmLog(infoset_id,chassis,slot,item_id);
           break;
     default:
             usage();
             return 0;
  }
   // printf("opterr=%d\n",opterr);
   return 0;

}

int main (int argc, char **argv)
{
  /*int n,ac;
  char *av[10];
  char **pav;
  char *delim=" ";
  char opt_str[]="dbm -i 1=2;2=1 -G -s 1 -u";
  char opt[10][32];
  
  for (n=0;n<10;n++)
   av[n]=opt[n];
  
  pav=av;
  ac=optsep(opt_str,delim,pav,10);
  
  printf("argc=%d %s %s\n",ac,pav[0],pav[1]);
  dbman(ac,pav);
*/
  dbman(argc,argv);
}

/*
int main1(int argc, char **argv)
{
  result_t result;
  char   col_name[10][16]={ "","", "","", "","","", "",""};
  uchar_t cols_width[12]={5,22,22,16,16,16,12,6};
  char   dataset[FIELD_MAX_COUNT][FIELD_MAX_SIZE]={ "","", "","", "","","", "",""};
  char sql[200];
  int n;

  char key[10][10];
  char value[10][20];
  char kv[]="1=link;2=down";
  getKeyValue(kv,";","=",(char*)key,10,(char*)value,20,5);
 
  result.col_name=col_name;
  result.fields=FIELD_MAX_COUNT;
  result.results=dataset;
  result.max_row=16;
  result.row=0;
  result.col=0;

  
  if (argc<2)
  {
    usage();
    return 0;
  }
  if (!strcmp(argv[1],"-t"))
  {
     sql_query(NULL,"SELECT * FROM InfoSet",&result);
      printArrTable("信息集列表",(char*)col_name,16,(char*)dataset,FIELD_MAX_SIZE,result.row,result.col,cols_width);
 return 1;
  }
  else if (!strcmp(argv[1],"-c"))
  {
    createDB();
    return;
  }
  else if (!strcmp(argv[1],"-i"))
  {
    status_infoset_t infoset;
    infoset.infoset_id=21;

    infoset.infoitems[1].item_id=1;
    infoset.infoitems[2].item_id=2;
    infoset.infoitems[1].value=1;
    infoset.infoitems[2].value=2;
    status_infoset_ex_t iset_ex;
    iset_ex.unit_class=3;
    iset_ex.unit_type=1;
    iset_ex.infoset=&infoset;

    mapStatusInfoSet(0,1,&iset_ex);
    printf("%s %s\n",iset_ex.item_name[1],iset_ex.status_name[1]);
    return;
  }
  else if (!strcmp(argv[1],"-m"))
  {
     sql_query(NULL,"SELECT * FROM InfoItem",&result);
      printArrTable("信息项列表",(char*)col_name,16,(char*)dataset,FIELD_MAX_SIZE,result.row,result.col,cols_width);
 return 1;
  }
  else if (!strcmp(argv[1],"-e"))
  {
    if (argc<3)
    {
      usage();
      return 0;
    }
    status_infoset_t infoset;
    infoset.infoitems[0].item_id=1;
    infoset.infoitems[1].item_id=2;
    infoset.infoitems[0].value=1;
    infoset.infoitems[1].value=2;
    infoset.infoset_id=21;

    // sql_query(NULL,"SELECT * FROM ItemStatus",&result);
    //  printArrTable("信息状态表",(char*)col_name,16,(char*)dataset,FIELD_MAX_SIZE,result.row,result.col,cols_width);
   showStatusInfoSet(0,atoi(argv[2]),infoset);
   return 1;
  }
  else if (!strcmp(argv[1],"-u"))
  {
     sql_query(NULL,"SELECT * FROM UnitUD",&result);
      printArrTable("信用户数据表",(char*)col_name,16,(char*)dataset,FIELD_MAX_SIZE,result.row,result.col,cols_width);
   return 1;
  }
  else if (!strcmp(argv[1],"-b"))
  {
    if (argc<3)
    {
      sql_query(NULL,"SELECT slot,property,sn,model,creation,fwver,hwver FROM UnitBI Limit 16",&result);
      printArrTable("单元盘基本信息列表",(char*)col_name,16,(char*)dataset,FIELD_MAX_SIZE,result.row,result.col,cols_width);
 return 1;
    }
    else
    {
      n=atoi(argv[1]);
      sprintf(sql,"SELECT slot,property,sn,model,creation,fwver,hwver FROM UnitBI WHERE slot=%s ",argv[2]);
      sql_query(NULL,sql,&result);
      //printf("result.row=%d col=%d\n",result.row,result.col);
      printArray("单元盘基本信息",(char*)col_name,16,(char*)dataset,FIELD_MAX_SIZE,result.row,result.col);
      return 1;
    }
  }
}
*/

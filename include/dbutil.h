#ifndef __DB_UTIL_H
#define __DB_UTIL_H

//#define DB_FILE_NAME "/usr/local/share/vispace/vispace.db"
#define DB_FILE_NAME "/vispace/visdb/vispace.db"
/*********************************************************************************
* 函数名称： dbOpen
* 功能描述： 打开数据库。 
* 访问的表： 
* 修改的表： 无
* 输入参数： const char * filename:数据库文件名
* 输出参数： 无
* 返 回 值： 成功返回sqlite3 指针,错误返回NULL
* 其它说明： 
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/10/18        V1.0      李文峰        创建
************************************************************************************/
sqlite3 *openDB(const char * filename);
/*
*********************************************************************************
* 函数名称： execSql
* 功能描述： execSql 是sqlite3_exec的封装
execSql接口执行0或多个UTF-8编码的，分号分割的sql语句，传到第二个参数中。
如果execSql的第三个参数回调函数指针不为空，那么它会为每个来自执行的SQL语句的结果行调用（也就是说回调函数会调用多次，上面例子中会返回2个结果行，因而会被执行2次），第4个参数是传给回调函数的第一个参数，如果回调函数指针为空，那么回调不会发生同时结果行被忽略。
如果在执行sql语句中有错误发生，那么当前的语句的执行被停止，后续的语句也被跳过。第五个参数不为空的时候，它被分配内存并写入了错误信息，所以在sqlite3_exec后面需要调用sqlite3_free去释放这个对象以防止内存泄露。

* 访问的表： 
* 修改的表： 无
* 输入参数： const char * filename:数据库文件名
* 输出参数： 无
* 返 回 值： 成功返回 ,错误返回-1
* 其它说明： 
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/10/18        V1.0      李文峰        创建
************************************************************************************/
int execSql(
sqlite3 *db,
const char *sql,
int (*callback)(void*,int,char**,char**),  /* Callback function */
void *arg                                     /* 1st argument to callback */
);

int getRowCount(sqlite3 *db,char *sql);

/*********************************************************************************
* 函数名称： getUserDataFromDB
* 功能描述： 从数据库中获取单元盘的用户信息。 
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

boolean_t getUserDataFromDB(uchar_t chassis,uchar_t slot,unit_user_data_t *udata);
boolean_t getUserDataByGbk(uchar_t chassis,uchar_t slot,unit_user_data_t *udata);
/*********************************************************************************
* 函数名称： setUserDataItemToDB
* 功能描述： 把单元盘的用户信息项保存到数据库。 
* 访问的表： UnitUD
* 修改的表： UnitUD
* 输入参数： uchar_t chassis,uchar_t slot,int item_id,char *udata:最大长度为30个字节
* 输出参数： unit_user_data_t *udata
* 返 回 值： 成功返回TRUE,错误返回FALSE
* 其它说明： 
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/10/18        V1.0      李文峰        创建
*例子:设置单元盘#3名称为:This is Unit #3
*setUserDataItemToDB(0,3,UNIT_NAME_ID,"This is Unit #3");
************************************************************************************/
boolean_t setUserDataItemToDB(uchar_t chassis,uchar_t slot,int item_id,const char *udata);

/*********************************************************************************
* 函数名称： setAlarmStatus
* 功能描述： 设置告警状态。 
* 访问的表： AlarmLog
* 修改的表： AlarmLog
* 输入参数： int alarm_code,char chassis,char slot,char entity,char status:告警状态 1--清除 2--确认
* 输出参数： 无
* 返 回 值： 成功返回TRUE,错误返回FALSE
* 其它说明： 
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/10/18        V1.0      李文峰        创建
*例子:设置单元盘#3告警状态为:清除
*setAlarmStatus(61,3,1,1);
************************************************************************************/
int setAlarmStatus(int alarm_code,char chassis,char slot,char entity,char status);
/*********************************************************************************
* 函数名称： setAlarmStatusById
* 功能描述： 设置告警状态。 
* 访问的表： AlarmLog
* 修改的表： AlarmLog
* 输入参数： int alarm_id,char status:告警状态 1--清除 2--确认
* 输出参数： 无
* 返 回 值： 成功返回TRUE,错误返回FALSE
* 其它说明： 
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/10/18        V1.0      李文峰        创建
*例子:设置alarm_id=10 告警状态为:清除
*setAlarmStatusById(10,1);
************************************************************************************/
int setAlarmStatusById(int alarm_id,char status);
int cleanAlarmLog(int alarm_code,char chassis,char slot,char entity);
/*********************************************************************************
* 函数名称： insertAlarmLog
* 功能描述： 插入告警记录。 
* 访问的表： AlarmLog
* 修改的表： AlarmLog
* 输入参数： int alarm_code,char chassis,char slot,char entity:物理实体，如光口1、光口2
* 输出参数： 无
* 返 回 值： 成功返回TRUE,错误返回FALSE
* 其它说明： 
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/10/18        V1.0      李文峰        创建
*例子:设置单元盘#3告警状态为:清除
*setAlarmStatus(61,3,1,1);
************************************************************************************/
int insertAlarmLog(int alarm_code,char chassis,char slot,char entity);
int deleteAlarmLogById(int alarm_id);
void sendSnmpTrap(int alarm_code,char chassis,char slot,char entity);
#endif //__DB_UTIL_H

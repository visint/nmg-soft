/*********************************************************************
* 版权所有 (C)2012, 汇信特通信技术有限公司。
*
* 文件名称： iniconf.c
* 文件标识： 
* 其它说明： INI文件的操作
* 当前版本： V1.0
* 作    者： 李文峰
* 完成日期： 
*
* 修改记录1：
*    修改日期：2012年6月23日
*    版 本 号：V1.0
*    修 改 人：李文峰
*    修改内容：创建
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <visstr.h>
/*#undef  VOS_WINNT*/
#define  VOS_WINNT

#ifdef VOS_WINNT
#define INIFileTstmain main1
#endif

#define SuccessRet 1;
#define FailedRet  0;

#define MAX_CFG_BUF                              512 

#define CFG_OK                                   0 
#define CFG_SECTION_NOT_FOUND                    -1 
#define CFG_KEY_NOT_FOUND                        -2 
#define CFG_ERR                                  -10 
#define CFG_ERR_FILE                             -10 
#define CFG_ERR_OPEN_FILE                        -10 
#define CFG_ERR_CREATE_FILE                      -11 
#define CFG_ERR_READ_FILE                        -12 
#define CFG_ERR_WRITE_FILE                       -13 
#define CFG_ERR_FILE_FORMAT                      -14 
#define CFG_ERR_SYSTEM                           -20 
#define CFG_ERR_SYSTEM_CALL                      -20 
#define CFG_ERR_INTERNAL                         -21 
#define CFG_ERR_EXCEED_BUF_SIZE                  -22 

#define COPYF_OK                                 0 
#define COPYF_ERR_OPEN_FILE                      -10 
#define COPYF_ERR_CREATE_FILE                    -11 
#define COPYF_ERR_READ_FILE                      -12 
#define COPYF_ERR_WRITE_FILE                     -13 

#define TXTF_OK                                  0 
#define TXTF_ERR_OPEN_FILE                       -1 
#define TXTF_ERR_READ_FILE                       -2 
#define TXTF_ERR_WRITE_FILE                      -3 
#define TXTF_ERR_DELETE_FILE                     -4 
#define TXTF_ERR_NOT_FOUND                       -5 

#define DEFALUT_CONF_FILE "/etc/vispace.conf"
char CFG_ssl = '[', CFG_ssr = ']';  /* 项标志符Section Symbol --可根据特殊需要进行定义更改，如 { }等*/
char CFG_nis = ':';                 /*name 与 index 之间的分隔符 */
char CFG_nts = '#';                 /*注释符*/

int  CFG_section_line_no, CFG_key_line_no, CFG_key_lines; 
/*
static char * strtrimr(char * buf);
static char * strtriml(char * buf);
*/
static int  FileGetLine(FILE *fp, char *buffer, int maxlen); 
static int  SplitKeyValue(char *buf, char **key, char **val); 

extern int  fileCopy(void *source_file, void *dest_file); 
extern int  splitSectionToNameIndex(char *section, char **name, char **index);
extern int  joinNameIndexToSection(char **section, char *name, char *index);
extern int  configGetKey(void *CFG_file, void *section, void *key, void *buf); 
extern int  configSetKey(void *CFG_file, void *section, void *key, void *buf); 
extern int  configGetSections(void *CFG_file, char *sections[]); 
extern int  configGetKeys(void *CFG_file, void *section, char *keys[]);
extern void  configDeleteSection(void *CFG_file, const char *section);
extern void INIFileTstmain(void);

/*缺省配置文件/etc/vispace.conf*/
extern int  visConfGetKey(void *section, void *key, void *buf); 
extern int  visConfSetKey(void *section, void *key, void *buf); 
extern int  visConfGetSections(char *sections[]); 
extern int  visConfGetKeys(void *section, char *keys[]);
extern void visConfDeleteSection(const char *section);

/**********************************************************************
* 函数名称： strtrimr
* 功能描述： 去除字符串右边的空字符
* 访问的表： 无
* 修改的表： 无
* 输入参数： char * buf 字符串指针
* 输出参数： 无
* 返 回 值： 字符串指针
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/6/23        V1.0      李文峰        创建
* 
***********************************************************************/
/*char * strtrimr(char * buf)
{
	int len,i;
	char * tmp = NULL;
	len = strlen(buf);
	tmp = (char*)malloc(len);
	
	memset(tmp,0x00,len);
	for(i = 0;i < len;i++)
	{
		if (buf[i] !=' ')
			break;
	}
	if (i < len) {
		strncpy(tmp,(buf+i),(len-i));
	}
	strncpy(buf,tmp,len);
	free(tmp);
	return buf;
}
*/
/**********************************************************************
* 函数名称： strtriml
* 功能描述： 去除字符串左边的空字符
* 访问的表： 无
* 修改的表： 无
* 输入参数： char * buf 字符串指针
* 输出参数： 无
* 返 回 值： 字符串指针
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/6/23        V1.0      李文峰        创建
* 
***********************************************************************/
/*char * strtriml(char * buf)
{
	int len,i;	
	char * tmp=NULL;
	len = strlen(buf);
	tmp = (char*)malloc(len);
	memset(tmp,0x00,len);
	for(i = 0;i < len;i++)
	{
		if (buf[len-i-1] !=' ')
			break;
	}
	if (i < len) {
		strncpy(tmp,buf,len-i);
	}
	strncpy(buf,tmp,len);
	free(tmp);
	return buf;
}
*/
/**********************************************************************
* 函数名称： FileGetLine
* 功能描述： 从文件中读取一行
* 访问的表： 无
* 修改的表： 无
* 输入参数： FILE *fp 文件句柄；int maxlen 缓冲区最大长度
* 输出参数： char *buffer 一行字符串
* 返 回 值： 实际读的长度
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/6/23        V1.0      李文峰        创建
* 
***********************************************************************/
int  FileGetLine(FILE *fp, char *buffer, int maxlen)
{
	int  i, j; 
	char ch1; 
	
	for(i = 0, j = 0; i < maxlen; j++) 
	{ 
		if(fread(&ch1, sizeof(char), 1, fp) != 1) 
		{ 
			if(feof(fp) != 0) 
			{ 
				if(j == 0) return -1;               /* 文件结束 */ 
				else break; 
			} 
			if(ferror(fp) != 0) return -2;        /* 读文件出错 */ 
			return -2; 
		} 
		else 
		{ 
			if(ch1 == '\n' || ch1 == 0x00) break; /* 换行 */ 
			if(ch1 == '\f' || ch1 == 0x1A)        /* '\f':换页符也算有效字符 */ 
			{ 
				buffer[i++] = ch1; 
				break; 
			} 
			if(ch1 != '\r') buffer[i++] = ch1;    /* 忽略回车符 */ 
		} 
	} 
	buffer[i] = '\0'; 
	return i; 
} 

/**********************************************************************
* 函数名称： fileCopy
* 功能描述： 文件拷贝
* 访问的表： 无
* 修改的表： 无
* 输入参数： void *source_file　源文件　void *dest_file　目标文件
* 输出参数： 无
* 返 回 值： 0 -- OK,非0－－失败
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/6/23        V1.0      李文峰        创建
* 
***********************************************************************/
int  fileCopy(void *source_file, void *dest_file)
{ 
	FILE *fp1, *fp2; 
	char buf[1024+1]; 
	int  ret; 
	
	if((fp1 = fopen((char *)source_file, "r")) == NULL) 
		return COPYF_ERR_OPEN_FILE; 
	ret = COPYF_ERR_CREATE_FILE; 
	
	if((fp2 = fopen((char *)dest_file, "w")) == NULL) goto copy_end; 
	
	while(1) 
	{ 
		ret = COPYF_ERR_READ_FILE; 
		memset(buf, 0x00, 1024+1); 
		if(fgets((char *)buf, 1024, fp1) == NULL) 
		{ 
			if(strlen(buf) == 0) 
			{ 
				if(ferror(fp1) != 0) goto copy_end; 
				break;                                   /* 文件尾 */ 
			} 
		} 
		ret = COPYF_ERR_WRITE_FILE; 
		if(fputs((char *)buf, fp2) == EOF) goto copy_end; 
	} 
	ret = COPYF_OK; 
copy_end: 
	if(fp2 != NULL) fclose(fp2); 
	if(fp1 != NULL) fclose(fp1); 
	return ret; 
}

/**********************************************************************
* 函数名称： splitSectionToNameIndex
* 功能描述： 分离section为name和index
*            [section]
*              /   \
*            name:index
*            jack  :   12 
*   	     |     |   | 
*            k1    k2  i 
* 访问的表： 无
* 修改的表： 无
* 输入参数： char *section  
* 输出参数： char **name, char **index
* 返 回 值： 1 --- ok 
*			 0 --- blank line 
*			-1 --- no name, ":index" 
*			-2 --- only name, no ':' 
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/6/23        V1.0      李文峰        创建
* 
***********************************************************************/
int  splitSectionToNameIndex(char *section, char **name, char **index)
{ 
	int  i, k1, k2, n; 
	
	if((n = strlen((char *)section)) < 1) return 0; 
	for(i = 0; i < n; i++) 
		if(section[i] != ' ' && section[i] != '\t') break; 
		if(i >= n) return 0; 
		if(section[i] == CFG_nis) return -1; 
		k1 = i; 
		for(i++; i < n; i++) 
			if(section[i] == CFG_nis) break; 
			if(i >= n) return -2; 
			k2 = i; 
			for(i++; i < n; i++) 
				if(section[i] != ' ' && section[i] != '\t') break; 
				section[k2] = '\0'; 
				*name = section + k1; 
				*index = section + i; 
				return 1; 
}  

/**********************************************************************
* 函数名称： joinNameIndexToSection
* 功能描述： 合成name和indexsection为section
*            jack  :   12   :1
*            name:index:unit
*              \    /    /
*              [section]
* 访问的表： 无
* 修改的表： 无
* 输入参数： char *name, char *index, char *unit
* 输出参数： char **section  
* 返 回 值： 1 --- ok 
*			 0 --- blank line 
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/6/23        V1.0      李文峰        创建
* 
***********************************************************************/
int  joinNameIndexToSection(char **section, char *name, char *index)
{ 
	int n1,n2; 
	
	if((n1 = strlen((char *)name)) < 1) return 0; 
	if((n2 = strlen((char *)index)) < 1) return 0;
	
	strcat(*section,name);
	strcat(*section+n1,":");
	strcat(*section+n1+1,index);
	*(*section+n1+1+n2) = '\0';
	return 1; 
} 
/**********************************************************************
* 函数名称： SplitKeyValue
* 功能描述： 分离key和value
*　　　　　　key=val
*			jack   =   liwf 
*			|      |   | 
*			k1     k2  i 
* 访问的表： 无
* 修改的表： 无
* 输入参数： char *buf  
* 输出参数： char **key;char **val
* 返 回 值： 1 --- ok 
*			 0 --- blank line 
*			-1 --- no key, "= val" 
*			-2 --- only key, no '=' 
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/6/23        V1.0      李文峰        创建
* 
***********************************************************************/
int  SplitKeyValue(char *buf, char **key, char **val)
{
	int  i, k1, k2, n; 
	
	if((n = strlen((char *)buf)) < 1) return 0; 
	for(i = 0; i < n; i++) 
		if(buf[i] != ' ' && buf[i] != '\t') break; 
		if(i >= n) return 0; 
		if(buf[i] == '=') return -1; 
		k1 = i; 
		for(i++; i < n; i++) 
			if(buf[i] == '=') break; 
			if(i >= n) return -2; 
			k2 = i; 
			for(i++; i < n; i++) 
				if(buf[i] != ' ' && buf[i] != '\t') break; 
				buf[k2] = '\0'; 
				*key = buf + k1; 
				*val = buf + i; 
				return 1; 
} 

/**********************************************************************
* 函数名称： configGetKey
* 功能描述： 获得key的值
* 访问的表： 无
* 修改的表： 无
* 输入参数： void *CFG_file　文件；void *section　项值；void *key　键值
* 输出参数： void *buf　key的值
* 返 回 值： 0 --- ok 非0 --- error 
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/6/23        V1.0      李文峰        创建
* 
***********************************************************************/
int  configGetKey(void *CFG_file, void *section, void *key, void *buf)
{ 
	FILE *fp; 
	char buf1[MAX_CFG_BUF + 1], buf2[MAX_CFG_BUF + 1]; 
	char *key_ptr, *val_ptr; 
	int  line_no, n, ret; 
	
	line_no = 0; 
	CFG_section_line_no = 0; 
	CFG_key_line_no = 0; 
	CFG_key_lines = 0; 
	
	if((fp = fopen((char *)CFG_file, "rb")) == NULL) return CFG_ERR_OPEN_FILE; 
	
	while(1)                                       /* 搜找项section */ 
	{ 
		ret = CFG_ERR_READ_FILE; 
		n = FileGetLine(fp, buf1, MAX_CFG_BUF); 
		if(n < -1) goto r_cfg_end; 
		ret = CFG_SECTION_NOT_FOUND; 
		if(n < 0) goto r_cfg_end;                    /* 文件尾，未发现 */ 
		line_no++; 
		n = strlen(strtriml(strtrimr(buf1))); 
		if(n == 0 || buf1[0] == CFG_nts) continue;       /* 空行 或 注释行 */ 
		ret = CFG_ERR_FILE_FORMAT; 
		if(n > 2 && ((buf1[0] == CFG_ssl && buf1[n-1] != CFG_ssr))) 
			goto r_cfg_end; 
		if(buf1[0] == CFG_ssl) 
		{ 
			buf1[n-1] = 0x00; 
			if(strcmp(buf1+1, section) == 0) 
				break;                                   /* 找到项section */ 
		} 
	} 
	CFG_section_line_no = line_no; 
	while(1)                                       /* 搜找key */ 
	{ 
		ret = CFG_ERR_READ_FILE; 
		n = FileGetLine(fp, buf1, MAX_CFG_BUF); 
		if(n < -1) goto r_cfg_end; 
		ret = CFG_KEY_NOT_FOUND; 
		if(n < 0) goto r_cfg_end;                    /* 文件尾，未发现key */ 
		line_no++; 
		CFG_key_line_no = line_no; 
		CFG_key_lines = 1; 
		n = strlen(strtriml(strtrimr(buf1))); 
		if(n == 0 || buf1[0] == CFG_nts) continue;       /* 空行 或 注释行 */ 
		ret = CFG_KEY_NOT_FOUND; 
		if(buf1[0] == CFG_ssl) goto r_cfg_end; 
		if(buf1[n-1] == '+')                         /* 遇+号表示下一行继续  */ 
		{ 
			buf1[n-1] = 0x00; 
			while(1) 
			{ 
				ret = CFG_ERR_READ_FILE; 
				n = FileGetLine(fp, buf2, MAX_CFG_BUF); 
				if(n < -1) goto r_cfg_end; 
				if(n < 0) break;                         /* 文件结束 */ 
				line_no++; 
				CFG_key_lines++; 
				n = strlen(strtrimr(buf2)); 
				ret = CFG_ERR_EXCEED_BUF_SIZE; 
				if(n > 0 && buf2[n-1] == '+')            /* 遇+号表示下一行继续 */ 
				{ 
					buf2[n-1] = 0x00; 
					if(strlen(buf1) + strlen(buf2) > MAX_CFG_BUF) 
						goto r_cfg_end; 
					strcat(buf1, buf2); 
					continue; 
				} 
				if(strlen(buf1) + strlen(buf2) > MAX_CFG_BUF) 
					goto r_cfg_end; 
				strcat(buf1, buf2); 
				break; 
			} 
		} 
		ret = CFG_ERR_FILE_FORMAT; 
		if(SplitKeyValue(buf1, &key_ptr, &val_ptr) != 1) 
			goto r_cfg_end; 
		strtriml(strtrimr(key_ptr)); 
		if(strcmp(key_ptr, key) != 0) 
			continue;                                  /* 和key值不匹配 */ 
		strcpy(buf, val_ptr); 
		break; 
	} 
	ret = CFG_OK; 
r_cfg_end: 
	if(fp != NULL) fclose(fp); 
	return ret; 
} 
int visConfGetKey(void *section, void *key, void *buf)
{
	return configGetKey(DEFALUT_CONF_FILE,section,key,buf);
}
/**********************************************************************
* 函数名称： configSetKey
* 功能描述： 设置key的值
* 访问的表： 无
* 修改的表： 无
* 输入参数： void *CFG_file　文件；void *section　项值；
* 			 void *key　键值；void *buf　key的值
* 输出参数： 无
* 返 回 值：   0 --- ok 非0 --- error 
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/6/23        V1.0      李文峰        创建
* 
***********************************************************************/
int  configSetKey(void *CFG_file, void *section, void *key, void *buf)
{ 
	FILE *fp1, *fp2; 
	char buf1[MAX_CFG_BUF + 1]; 
	int  line_no, line_no1, n, ret, ret2; 
	//char *tmpfname; 
	char template[]="tmp-XXXXXX";

	ret = configGetKey(CFG_file, section, key, buf1); 
	if(ret <= CFG_ERR && ret != CFG_ERR_OPEN_FILE) return ret; 
	if(ret == CFG_ERR_OPEN_FILE || ret == CFG_SECTION_NOT_FOUND) 
	{ 
		
		if((fp1 = fopen((char *)CFG_file, "a")) == NULL) 
			
			return CFG_ERR_CREATE_FILE; 
		
		if(fprintf(fp1, "%c%s%c\n", CFG_ssl, section, CFG_ssr) == EOF) 
		{ 
			fclose(fp1); 
			return CFG_ERR_WRITE_FILE; 
		} 
		if(fprintf(fp1, "%s=%s\n", key, buf) == EOF) 
		{ 
			fclose(fp1); 
			return CFG_ERR_WRITE_FILE; 
		} 
		fclose(fp1); 
		return CFG_OK; 
	} 
       /*
	if((tmpfname = tmpnam(NULL)) == NULL) 
		return CFG_ERR_CREATE_FILE; 
	
	if((fp2 = fopen(tmpfname, "w")) == NULL) 
		
		return CFG_ERR_CREATE_FILE; 
        */
        if(mkstemp(template) == -1) 
           return CFG_ERR_CREATE_FILE;
        if((fp2 = fopen(template, "w")) == NULL) 
	   return CFG_ERR_CREATE_FILE; 
 
        //= mkstemp(template);

	ret2 = CFG_ERR_OPEN_FILE; 
	
	if((fp1 = fopen((char *)CFG_file, "rb")) == NULL) goto w_cfg_end; 
	
	if(ret == CFG_KEY_NOT_FOUND) 
		line_no1 = CFG_section_line_no; 
	else /* ret = CFG_OK */ 
		line_no1 = CFG_key_line_no - 1; 
	for(line_no = 0; line_no < line_no1; line_no++) 
	{ 
		ret2 = CFG_ERR_READ_FILE; 
		n = FileGetLine(fp1, buf1, MAX_CFG_BUF); 
		if(n < 0) goto w_cfg_end; 
		ret2 = CFG_ERR_WRITE_FILE; 
		if(fprintf(fp2, "%s\n", buf1) == EOF) goto w_cfg_end; 
	} 
	if(ret != CFG_KEY_NOT_FOUND) 
		for( ; line_no < line_no1+CFG_key_lines; line_no++) 
		{ 
			ret2 = CFG_ERR_READ_FILE; 
			n = FileGetLine(fp1, buf1, MAX_CFG_BUF); 
			if(n < 0) goto w_cfg_end; 
		} 
		ret2 = CFG_ERR_WRITE_FILE; 
		if(fprintf(fp2, "%s=%s\n", key, buf) == EOF) goto w_cfg_end; 
		while(1) 
		{ 
			ret2 = CFG_ERR_READ_FILE; 
			n = FileGetLine(fp1, buf1, MAX_CFG_BUF); 
			if(n < -1) goto w_cfg_end; 
			if(n < 0) break; 
			ret2 = CFG_ERR_WRITE_FILE; 
			if(fprintf(fp2, "%s\n", buf1) == EOF) goto w_cfg_end; 
		} 
		ret2 = CFG_OK; 
w_cfg_end: 
		if(fp1 != NULL) fclose(fp1); 
		if(fp2 != NULL) fclose(fp2); 
		if(ret2 == CFG_OK) 
		{ 
			ret = fileCopy(template, CFG_file); 
			if(ret != 0) return CFG_ERR_CREATE_FILE; 
		} 
		remove(template); 
		return ret2; 
} 
int visConfSetKey(void *section, void *key, void *buf)
{
	 return configSetKey(DEFALUT_CONF_FILE, section, key,buf);
}
/**********************************************************************
* 函数名称： configGetSections
* 功能描述： 获得所有section
* 访问的表： 无
* 修改的表： 无
* 输入参数： void *CFG_file　文件
* 输出参数： char *sections[]　存放section名字
* 返 回 值： 返回section个数。若出错，返回负数。
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/6/23        V1.0      李文峰        创建
* 
***********************************************************************/
int  configGetSections(void *CFG_file, char *sections[])
{ 
	FILE *fp; 
	char buf1[MAX_CFG_BUF + 1]; 
	int  n, n_sections = 0, ret; 
	
	
	if((fp = fopen(CFG_file, "rb")) == NULL) return CFG_ERR_OPEN_FILE; 
	
	while(1)                                       /*搜找项section */ 
	{ 
		ret = CFG_ERR_READ_FILE; 
		n = FileGetLine(fp, buf1, MAX_CFG_BUF); 
		if(n < -1) goto cfg_scts_end; 
		if(n < 0) break;                             /* 文件尾 */ 
		n = strlen(strtriml(strtrimr(buf1))); 
		if(n == 0 || buf1[0] == CFG_nts) continue;       /* 空行 或 注释行 */ 
		ret = CFG_ERR_FILE_FORMAT; 
		if(n > 2 && ((buf1[0] == CFG_ssl && buf1[n-1] != CFG_ssr))) 
			goto cfg_scts_end; 
		if(buf1[0] == CFG_ssl) 
		{ 
			buf1[n-1] = 0x00; 
			strcpy(sections[n_sections], buf1+1); 
			n_sections++; 
		} 
	} 
	ret = n_sections; 
cfg_scts_end: 
	if(fp != NULL) fclose(fp); 
	return ret; 
} 
int  visConfGetSections(char *sections[])
{
	return configGetSections(DEFALUT_CONF_FILE,sections);
}
/**********************************************************************
* 函数名称： configGetKeys
* 功能描述： 获得所有key的名字（key=value形式, value可用加号表示续行）
* 访问的表： 无
* 修改的表： 无
* 输入参数： void *CFG_file　文件 void *section 项值
* 输出参数： char *keys[]　存放key名字
* 返 回 值： 返回key个数。若出错，返回负数。
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/6/23        V1.0      李文峰        创建
* 
***********************************************************************/
int  configGetKeys(void *CFG_file, void *section, char *keys[])
{ 
	FILE *fp; 
	char buf1[MAX_CFG_BUF + 1], buf2[MAX_CFG_BUF + 1]; 
	char *key_ptr, *val_ptr; 
	int  n, n_keys = 0, ret; 
	
	
	if((fp = fopen(CFG_file, "rb")) == NULL) return CFG_ERR_OPEN_FILE; 
	
	while(1)                                       /* 搜找项section */ 
	{ 
		ret = CFG_ERR_READ_FILE; 
		n = FileGetLine(fp, buf1, MAX_CFG_BUF); 
		if(n < -1) goto cfg_keys_end; 
		ret = CFG_SECTION_NOT_FOUND; 
		if(n < 0) goto cfg_keys_end;                 /* 文件尾 */ 
		n = strlen(strtriml(strtrimr(buf1))); 
		if(n == 0 || buf1[0] == CFG_nts) continue;       /* 空行 或 注释行 */ 
		ret = CFG_ERR_FILE_FORMAT; 
		if(n > 2 && ((buf1[0] == CFG_ssl && buf1[n-1] != CFG_ssr))) 
			goto cfg_keys_end; 
		if(buf1[0] == CFG_ssl) 
		{ 
			buf1[n-1] = 0x00; 
			if(strcmp(buf1+1, section) == 0) 
				break;                                   /* 找到项section */ 
		} 
	} 
	while(1) 
	{ 
		ret = CFG_ERR_READ_FILE; 
		n = FileGetLine(fp, buf1, MAX_CFG_BUF); 
		if(n < -1) goto cfg_keys_end; 
		if(n < 0) break;                             /* 文件尾 */ 
		n = strlen(strtriml(strtrimr(buf1))); 
		if(n == 0 || buf1[0] == CFG_nts) continue;       /* 空行 或 注释行 */ 
		ret = CFG_KEY_NOT_FOUND; 
		if(buf1[0] == CFG_ssl) 
			break;                                     /* 另一个 section */ 
		if(buf1[n-1] == '+')                         /* 遇+号表示下一行继续 */ 
		{ 
			buf1[n-1] = 0x00; 
			while(1) 
			{ 
				ret = CFG_ERR_READ_FILE; 
				n = FileGetLine(fp, buf2, MAX_CFG_BUF); 
				if(n < -1) goto cfg_keys_end; 
				if(n < 0) break;                         /* 文件尾 */ 
				n = strlen(strtrimr(buf2)); 
				ret = CFG_ERR_EXCEED_BUF_SIZE; 
				if(n > 0 && buf2[n-1] == '+')            /* 遇+号表示下一行继续 */ 
				{ 
					buf2[n-1] = 0x00; 
					if(strlen(buf1) + strlen(buf2) > MAX_CFG_BUF) 
						goto cfg_keys_end; 
					strcat(buf1, buf2); 
					continue; 
				} 
				if(strlen(buf1) + strlen(buf2) > MAX_CFG_BUF) 
					goto cfg_keys_end; 
				strcat(buf1, buf2); 
				break; 
			} 
		} 
		ret = CFG_ERR_FILE_FORMAT; 
		if(SplitKeyValue(buf1, &key_ptr, &val_ptr) != 1) 
			goto cfg_keys_end; 
		strtriml(strtrimr(key_ptr));
		strcpy(keys[n_keys], key_ptr); 
		n_keys++; 
	} 
	ret = n_keys; 
cfg_keys_end: 
	if(fp != NULL) fclose(fp); 
	return ret; 
}
int visConfGetKeys(void *section, char *keys[])
{
	return configGetKeys(DEFALUT_CONF_FILE, section, keys);
}
/**********************************************************************
* 函数名称： configDeleteSection
* 功能描述： 删除Section下的所有key的名字（key=value形式）
* 访问的表： 无
* 修改的表： 无
* 输入参数： void *CFG_file　文件 void *section 项值
* 输出参数： char *keys[]　存放key名字
* 返 回 值： 无。
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/10/1        V1.0      李文峰        创建
* 
***********************************************************************/
void configDeleteSection(void *CFG_file, const char *section)
{
	FILE *fp;
	char config_linebuf[MAX_CFG_BUF]; 
	char buf[MAX_CFG_BUF + 1]; 
	int  line_no, n, ret; 
	char *tmpfname;
	char begin=0,end=0;
	long congig_lenth;
	char sum_buf[0x8000];

	fp = fopen(CFG_file,"r"); 
	if(fp == NULL) 
    return; 
   fseek(fp,0,SEEK_END); 
   congig_lenth = ftell(fp); 
  
  memset(sum_buf,0,sizeof(sum_buf)); 
  fseek(fp,0,SEEK_SET);  
  while(fgets(config_linebuf,256,fp) != NULL) 
  {
   	 if(strlen(config_linebuf) < 3 ||config_linebuf[0]=='#') //判断是否是空行 
     { 
       strcat(sum_buf,config_linebuf); 
       continue; 
     }
     if (begin!=1)
     {
     	 if (NULL!=strstr(config_linebuf,section))
     	   begin=1;
     	 strcat(sum_buf,config_linebuf);
     }
     else
     {
     	if (end==0)
      {
     	 if (config_linebuf[0]==CFG_ssl)
     	 {
     		end=1;
     		strcat(sum_buf,config_linebuf);
       }
      }
      else
     	  strcat(sum_buf,config_linebuf);
     }   
  }
  fclose(fp);
  fp = fopen(CFG_file,"w"); 
  if(fp == NULL) 
    return;
  fputs(sum_buf,fp); 
  fclose(fp);
}
void visConfDeleteSection(const char *section)
{
	configDeleteSection(DEFALUT_CONF_FILE,section);
}
/**********************************************************************
* 函数名称： INIFileTstmain
* 功能描述： 测试函数入口
* 访问的表： 无
* 修改的表： 无
* 输入参数： 无
* 输出参数： 无
* 返 回 值： 无
* 其它说明： 无
* 修改日期        版本号     修改人       修改内容
* -----------------------------------------------
* 2012/6/23        V1.0      李文峰        创建
* 
***********************************************************************/
void INIFileTstmain(void)
{
	char buf[20]="";
	char buf1[20]="";
	char buf2[20]="";
	char buf3[20]="";
	int  ret;
	long abc;
	int i;
	
	char * section;
	char * key;
	char * val;
	char * name;
	char * index;
	section = buf1;
	key = buf2;
	val = buf3;
	
	//ret = configSetKey("/etc/vissnmp.conf", "agentAddressy", "Unit", "-12321");
	
        ret = visConfSetKey( "DevIp", "ipAdd", "192.168.1.228");
        ret = visConfSetKey( "DevIp", "mask", "255.255.255.0");
        ret = visConfSetKey( "DevIp", "gateway", "192.168.1.1");
	for(i = 0;i < 1; i++)
	{
		memset(buf,0x00,20);
		memset(buf1,0x00,20);
		memset(buf2,0x00,20);
		memset(buf3,0x00,20);
		sprintf(buf,"%d",i);
		sprintf(buf2,"NMU%d",i);
		sprintf(buf3,"%d",i);
		joinNameIndexToSection(&section, "Unit", buf);
		configSetKey("/etc/vissnmp.conf", section, key, val);
	}
	
	name = buf2;
	index = buf3;
	memset(buf1,0x00,20);
	memset(buf2,0x00,20);
	memset(buf3,0x00,20);
	strcpy(buf1,"Unit:NMU");
	splitSectionToNameIndex(section, &name, &index);
	printf("\n name=%s,index=%s\n",name,index);
	
	ret = configGetKey("/etc/vissnmp.conf", "Unit:NMU", "Unit", buf);
	if (strcmp(buf,"")!=0) {
		abc = atol(buf);
	}
	
	printf("\n buf=%s\n",buf);	
	printf("\n abc=%ld\n",abc);
	
        ret = visConfGetKey("DevIp", "ipAdd", buf);
	if (strcmp(buf,"")!=0) {
           printf("IpAdd:%s\n",buf);
        }
	configSetKey("/etc/vissnmp.conf","Unit:NMU", "Email", "liwf@163.com");

	fileCopy("/etc/vissnmp.conf", "Configbak.dat");

	printf("\nFile %s line%d\n",__FILE__,__LINE__);

	printf("\n******** This test is created by Unit NMU. Email:liwf@163.com********\n\n");
	
}

/* 
 *从配置文件中读取相应的值 
 *输入参数：1，配置文件路径 2，匹配标记 3，输出存储空间 
 *并且排除了空行，“=”前后无内容，无“=”的情况 
 */ 
int getConfValue(char *conf_path,char *conf_name,char *config_buff,char *config_sign) 
{ 
    char config_linebuf[512]; 
    char line_name[40]; 
    char exchange_buf[256]; 
    //char *config_sign = "="; 
    char *leave_line; 
    FILE *f; 
    // printf("Here!%s\n",conf_path);
    f = fopen(conf_path,"r"); 
    if(f == NULL) 
    { 
        printf("OPEN CONFIG FALID\n"); 
        return -1; 
    } 
    // printf("Here...!\n");
    fseek(f,0,SEEK_SET); 
     //fgets((char *)config_linebuf,250,f);
     //printf("Here...2!\n");
    while(1) 
    {    //printf("Here!!!!!!!\n");
        if (fgets(config_linebuf,250,f) == NULL)
               break;
     //     printf("Here3\n");
        if(strlen(config_linebuf) < 3) //判断是否是空行 
        { 
            continue; 
        } 
       //  printf("Here!!!!!!!\n");
        if (config_linebuf[strlen(config_linebuf)-1] == 10) //去除最后一位是\n的情况 
        { 
             
            memset(exchange_buf,0,sizeof(exchange_buf)); 
            strncpy(exchange_buf,config_linebuf,strlen(config_linebuf)-1); 
            memset(config_linebuf,0,sizeof(config_linebuf)); 
            strcpy(config_linebuf,exchange_buf); 
        } 
       //     printf("Here4\n");
        memset(line_name,0,sizeof(line_name));
        //leave_line = strstr(config_linebuf,"#");
       // if(leave_line != NULL)
        //	 *leave_line='\0';
        //leave_line=strtrim(leave_line);
           leave_line = strstr(config_linebuf,"#");
       if(leave_line)                            //去除无"="的情况 
        { 
            continue; 
        } 
        leave_line = strstr(config_linebuf,config_sign);
           // printf("Here5\n");
        //leave_line = strstr(leave_line,config_sign);
         //  printf("Here5\n");
        if(leave_line == NULL)                            //去除无"="的情况 
        { 
            continue; 
        } 
      
        int leave_num = leave_line - config_linebuf; 
        strncpy(line_name,config_linebuf,leave_num); 
        if(strcmp(line_name,conf_name) ==0) 
        { 
            strncpy(config_buff,config_linebuf+(leave_num+1),strlen(config_linebuf)-leave_num-1);
            config_buff[strlen(config_linebuf)-leave_num-1]='\0';
            //printf("value=%s\n",config_buff);
         break; 
        } 
        if(fgetc(f)==EOF) 
        { 
            break;   
        } 
        fseek(f,-1,SEEK_CUR); 
        memset(config_linebuf,0,sizeof(config_linebuf)); 
    } 
    fclose(f); 
  return 0;
} 


/* 
 *添加修改文件（当配置文件中存在标记字段，则进行修改，若不存在则进行添加） 
 * 
 *输入参数：1，配置文件路径 2，匹配标记 3，替换或添加的内容 
 * 
 */ 

int setConfValue(char *conf_path,char *conf_name,char *config_buff,char *config_sign) 
{ 
     
    char config_linebuf[512]; 
    char line_name[40]; 
    //char *config_sign = "="; 
    char *leave_line; 
    int  alter_sign = 0; 
     
    FILE *f; 
    f = fopen(conf_path,"r+"); 
    if(f == NULL) 
    { 
        printf("OPEN CONFIG FALID\n"); 
        return -1; 
    } 
    fseek(f,0,SEEK_END); 
    long congig_lenth = ftell(f); 
    int configbuf_lenth = strlen(config_buff); 
    configbuf_lenth = configbuf_lenth + 5; 
    char sum_buf[congig_lenth+configbuf_lenth]; 
    memset(sum_buf,0,sizeof(sum_buf)); 
    fseek(f,0,SEEK_SET);  
    while(fgets((char *)config_linebuf,256,f) != NULL) 
    {    
      if(strlen(config_linebuf) < 3) //判断是否是空行 
        { 
                //strcat(sum_buf,conf_name);
                 //strcat(sum_buf,"=");
            strcat(sum_buf,config_linebuf); 
            continue; 
        } 
        leave_line = NULL; 
        leave_line = strstr(config_linebuf,config_sign); 
        if(leave_line == NULL)                            //去除无"="的情况 
        { 
               //strcat(sum_buf,conf_name);
                 //strcat(sum_buf,"=");
            strcat(sum_buf,config_linebuf); 
            continue; 
        } 
        int leave_num = leave_line - config_linebuf; 
        memset(line_name,0,sizeof(line_name)); 
        strncpy(line_name,config_linebuf,leave_num); 
        if(strcmp(line_name,conf_name) ==0) 
        { 
                strcat(sum_buf,conf_name);
                 strcat(sum_buf,config_sign);
            strcat(sum_buf,config_buff); 
            strcat(sum_buf,"\n"); 
            alter_sign = 1; 
        } 
        else 
        { 
            //strcat(sum_buf,config_linebuf); 
             //strcat(sum_buf,conf_name);
                // strcat(sum_buf,"=");
                 strcat(sum_buf,config_linebuf);
        } 
        if(fgetc(f)==EOF) 
        { 
            break;   
        } 
        fseek(f,-1,SEEK_CUR); 
        memset(config_linebuf,0,sizeof(config_linebuf)); 
    } 
    if(alter_sign == 0) 
    { 
           strcat(sum_buf,conf_name);
           strcat(sum_buf,config_sign);
        strcat(sum_buf,config_buff); 
        strcat(sum_buf,"\n"); 
    } 
    printf("---sum_buf---->%s<----------\n",sum_buf); 
    remove(conf_path); 
    fclose(f); 
    FILE *fp; 
    fp = fopen(conf_path,"w+"); 
    if(fp == NULL) 
    { 
        printf("OPEN CONFIG FALID\n"); 
        return 2; 
    } 
    fseek(fp,0,SEEK_SET); 
    fputs(sum_buf,fp); 
    fclose(fp);
     return 0;
} 



/* 
 *删除配置文件内容（ 
 * 
 *输入参数：1，配置文件路径 2，匹配标记  
 * 
 */ 
deleteConfItem(char *conf_path,char *conf_name) 
{ 
     
    char config_linebuf[256]; 
    char line_name[40]; 
    char *config_sign = "="; 
    char *leave_line; 
     
    FILE *f; 
    f = fopen(conf_path,"r+"); 
    if(f == NULL) 
    { 
        printf("OPEN CONFIG FALID\n"); 
        return 0; 
    } 
    fseek(f,0,SEEK_END); 
    long congig_lenth = ftell(f); 
    char sum_buf[congig_lenth+2]; 
    memset(sum_buf,0,sizeof(sum_buf)); 
    fseek(f,0,SEEK_SET);  
    while(fgets(config_linebuf,256,f) != NULL) 
    {    
        if(strlen(config_linebuf) < 3) //判断是否是空行 
        { 
            strcat(sum_buf,config_linebuf); 
            continue; 
        } 
        leave_line = NULL; 
        leave_line = strstr(config_linebuf,config_sign); 
        if(leave_line == NULL)                            //去除无"="的情况 
        { 
            strcat(sum_buf,config_linebuf); 
            continue; 
        } 
        int leave_num = leave_line - config_linebuf; 
        memset(line_name,0,sizeof(line_name)); 
        strncpy(line_name,config_linebuf,leave_num); 
        if(strcmp(line_name,conf_name) ==0) 
        { 
             
        } 
        else 
        { 
            strcat(sum_buf,config_linebuf); 
        } 
         
        if(fgetc(f)==EOF) 
        { 
            break;   
        } 
        fseek(f,-1,SEEK_CUR); 
        memset(config_linebuf,0,sizeof(config_linebuf)); 
    } 
    printf("---sum_buf---->%s<----------\n",sum_buf); 
    remove(conf_path); 
    fclose(f); 
    FILE *fp; 
    fp = fopen(conf_path,"w+"); 
    if(fp == NULL) 
    { 
        printf("OPEN CONFIG FALID\n"); 
        return 2; 
    } 
    fseek(fp,0,SEEK_SET); 
    fputs(sum_buf,fp); 
    fclose(fp); 
} 


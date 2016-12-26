#ifndef __VISSTR_H
#define __VISSTR_H
#include <ctype.h>
#include <string.h>
/*
extern void strdel(char *source,const char *begin,const char *end);
extern void strins(char source[],size_t len,const char *begin1,const char *begin2,const char *insert);
extern char *bufins(char *bytes,size_t buf_len,const char *str);
extern char *bufstr(const char *bytes,size_t len,const char *str);
extern void vis_str_test();
*/
char * strtriml(char * buf);
char * strtrimr(char * buf);
/*
*去除字符串两端空格
*/
char *strtrim(char *pstr);

char *strTolower(char *s);
extern int keyval(char *buf, char *sep,char **key, char **val);
#endif

#include <stdio.h> 
#include <stdlib.h> 
#include <stdarg.h>  
#include <errno.h>  
#include <fcntl.h>  
#include <limits.h>  
#include <unistd.h>  
#include <signal.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <sys/wait.h>  
#include <sys/ioctl.h>  
#include <ctype.h> 

#include <dirent.h>
#include <iconv.h>
#include <sysinfo.h>
/*   *  * description: parse va and do system   *   */ 
int doSystem(const char *fmt, ...)  
{         
  char cmd_buf[512];          
  va_list pargv;
  va_start(pargv, fmt);
  vsnprintf(cmd_buf, sizeof(cmd_buf), fmt, pargv);
  va_end(pargv);
  return system(cmd_buf);
}

pid_t getPidByName ( char *name)
{  
    FILE *fp;
    char n=0;
    pid_t           pid = -1;
    char buf[10]="";
    fp = popen(name,"r");
    if (fp!=NULL)
    {
      if ((fgets (buf, 6, fp)) == NULL)
      {
        pclose(fp);
        return(pid );
      }
     //buf[n]='\0';
     pclose(fp);
     pid=atoi(buf);
    }
    //printf("%s pid=%d\n",buf,pid);
    return( pid );
}  /* end of getpidbyname */

static int code_convert(char* from_charset, char* to_charset,
        char* inbuf, size_t inlen, char* outbuf, size_t outlen)
{
    iconv_t cd;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset, from_charset);
    if (cd ==  (iconv_t)-1)
    {
        perror("iconv_open:");
        return -1;
    }

    memset(outbuf,0,outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
    {
        //printf("%s: call iconv failed!\n", __FUNCTION__);
        printf("errno=%d\n", errno);
        perror("iconv failed:\n");
        iconv_close(cd);
        return -1;
    }
    iconv_close(cd);
    return 0;
}

int g2u(char* inbuf, size_t inlen, char* outbuf, size_t outlen)
{
    return code_convert("gb2312", "utf-8", inbuf, inlen, outbuf, outlen);
}

int u2g(char* inbuf, size_t inlen, char* outbuf, size_t outlen)
{
    return code_convert("utf-8", "gb2312",inbuf, inlen, outbuf, outlen);
}

char getCpuUsage(float *sys_usage,float *user_usage)
{
 #define  CPU_FILE_PROC_STAT "/proc/stat"
 FILE *fp = NULL;
 char tmp[10];
 unsigned long user,sys,nice,idle,total;
 //unsigned long cpu_rate;
 
 fp = fopen(CPU_FILE_PROC_STAT,"r");
 if(fp == NULL)
 {
   return -1;
 }
 fscanf(fp,"%s %lu %lu %lu %lu",tmp,&user,&nice,&sys,&idle);
/*
 user = usage->cpu_user;
 sys = usage->cpu_sys;
 nice = usage->cpu_nice;
 idle = usage->cpu_idle;
*/
 fclose(fp);
 total = user + sys + nice + idle;
 if (total>0)
 {
  *sys_usage=sys*100.0/total;
  *user_usage=user*100.0/total;
 }
 else
 {
  *sys_usage=0;
  *user_usage=0;
  return -1;
 }
 //cpu_rate = (1-idle/total)*100;
 return 0;
}

float getMemUsage(struct mem_usage_t *usage)
{
  /*FILE *fp = NULL;
  
  fp = popen("free -m| grep -n '^'  |grep  '^2:'|grep -o '[^4:].*'|cut -d : -f 3","r");
  if (NULL==fp)
     return -1;
  usage->cached=0;
  fscanf(fp,"%lu %lu %lu %lu %lu %lu",&(usage->total),
	&(usage->used),&(usage->free),&(usage->shared),&(usage->buffers),&(usage->cached));
  pclose(fp);
  //total=usage->used+usage->shared+usage->buffers+usage->cached;
  if (usage->total>0)
   return (usage->used-usage->buffers-usage->cached)*100.0/usage->total;
  return -1.0;
*/
  //long unsigned  total=0,used=0,free_mem=0,shrd=0,buff=0,cached=0;
  FILE *fp = NULL;
 
  fp = popen("top -n 1|grep Mem","r");
  if (NULL==fp)
    return -1;
  usage->used=0;
  usage->free=0;
  usage->shared=0;
  usage->buffers=0;
  usage->cached=0;
  fscanf(fp,"%lu %lu %lu %lu %lu",&(usage->total),
	&(usage->used),&(usage->free),&(usage->shared),&(usage->buffers),&(usage->cached));
  //printf("%lu %lu %lu %lu %lu\n",used,free_mem,shrd,buff,cached);
  //sscanf("KiB Mem: 1026624 total,   828928 used,   197696 free,   113768 buffers","%*s%lu %lu %lu %lu %lu %lu",&used,&free_mem,&shrd,&buff,&cached);
 #ifdef X86_LINUX
  fscanf(fp,"%*s%*s%lu %*s%lu %*s%lu %*s%lu %*s%lu %*s%lu",&(usage->total),
	&(usage->used),&(usage->free),&(usage->shared),&(usage->buffers),&(usage->cached));
//printf("buf:%s total=%lu used=%lu free_mem=%lu shrd=%lu,buff=%lu,cached=%lu\n",buf,total,used,free_mem,shrd,&(usage->buffers),&(usage->cached));
 #else
   fscanf(fp,"%*s%lu %*s%*s%lu %*s%*s%lu %*s%*s%lu %*s%*s%lu",&(usage->used),&(usage->free),&(usage->shared),&(usage->buffers),&(usage->cached));
usage->total=usage->used+usage->free+usage->cached;
if (usage->total>0)
{
//printf("total=%lu used=%lu free_mem=%lu shrd=%lu,buff=%lu,cached=%lu Mem usage=%.1f%%\n",usage->total,usage->used,usage->free,usage->shared,usage->buffers,usage->cached,usage->used*100./usage->total);
pclose(fp);
return (usage->used)*100.0/usage->total;
}
#endif
pclose(fp);
return -1;
}




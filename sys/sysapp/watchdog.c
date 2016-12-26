#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sysinfo.h>

//#define WEBS_SERVICE "/vispace/webs/webs&"
//#define WEBS_PATH "/home/visint/VisProj/src/webs/LINUX"
#define WEBS_PATH "/visipace/webs"
#define SNMPD_SERVICE "snmpd -c /vispace/snmp/snmpd.conf"
#define CLID_SERVICE "/vispace/webs/webs&"
//gcc -o watchdogd watchdog.c -I./../../include -L./../../linuxlib -lvispace -liconv -lsqlite3


//arm-linux-gcc -o watchdogd watchdog.c -I./../../include -L/home/visint/src/arm/lib -lvispace -liconv -lsqlite3
static unsigned short start_count=0;
static unsigned short webs_start_count=0;
static unsigned short snmp_start_count=0;
static void startService(int proc)
{
  pid_t pid;
  if (3==proc)
  {
     pid=getPidByName("pidof webs");
     if (pid<1)
     {
        start_count++;
        doSystem("cd %s && ./webs&",WEBS_PATH);
        //system(WEBS_SERVICE);
        printf("Starting webs\n");
      }
  }
  else if (2==proc)
  {
      pid=getPidByName("pidof snmpd");
      if (pid<1)
      {
         system(SNMPD_SERVICE);
         start_count++;
         printf("Starting snmpd\n");
      }
     // else
     //    printf("snmpd running pid=%d\n",pid);
 }
 if (start_count>10)
 {   
   snmp_start_count=0;
   sendSnmpTrap(1,0,16,0);
   system("reboot");
 }
} 
static void sigHandle(int sig,struct siginfo *siginfo,void *myact)
{

  if (sig == SIGSEGV || sig==SIGUSR1)
  {
    sleep(1);
    startService(siginfo->si_int);
  }
  else if (sig == SIGALRM)
  {
    float sys_usage, user_usage,mem_usage;
    struct mem_usage_t memery;
    if (!getCpuUsage(&sys_usage,&user_usage))
    {
      //printf("CPU usage user:%.1f%%\tsys:%.1f%%\n",user_usage,sys_usage);
      
      if ((sys_usage+user_usage)>80)
      {
       printf("CPU usage user:%.1f%%\tsys:%.1f%% Over!\n",user_usage,sys_usage);
       sendSnmpTrap(3,0,16,0);
       system("reboot");
      }
    }
    mem_usage=getMemUsage(&memery);
    //printf("%lu %lu %lu %lu %lu %lu\n",memery.total,memery.used,memery.free,memery.shared,memery.buffers,memery.cached);
    //printf("Mem usage:%.1f%%\n",mem_usage);
    if (mem_usage>0)
    {
      if (mem_usage>90)
      {
       printf("%lu %lu %lu %lu %lu %lu\n",memery.total,
	memery.used,memery.free,memery.shared,memery.buffers,memery.cached);
       printf("Mem usage:%.1f%% over!\nreboot...\n",mem_usage);
       sendSnmpTrap(5,0,16,0);
       system("reboot");
      }
    }
    startService(2);
    startService(3);
    alarm(20);
  }
}

static void sigInit()
{
  int i;
  struct sigaction act;
  sigemptyset(&act.sa_mask);  
  act.sa_flags=SA_SIGINFO;
  act.sa_sigaction=sigHandle;

  sigaction(SIGALRM,&act,NULL);
  /*
  sigaction(SIGINT,&act,NULL);
  sigaction(SIGQUIT,&act,NULL);
  sigaction(SIGTSTP,&act,NULL);
  sigaction(SIGTTIN,&act,NULL);
  sigaction(SIGTERM,&act,NULL);
  sigaction(SIGKILL,&act,NULL);
  sigaction(SIGUSR1,&act,NULL);
  sigaction(SIGUSR2,&act,NULL);
  sigaction(SIGRTMIN,&act,NULL);
  */
  sigaction(SIGSEGV,&act,NULL);
}

int main(int argc, char **argv)
{
  sigInit();
  alarm(30);
  /* Most of time it goes to sleep */
  while (1)
  {	
    pause();
  }
  return 1;
}

/******************************** 说明 *********************************/

/*
 * 信息集是硬件网管信息与上层访问的接口。网管信息可通过ID号来访问。
 * 网管信息ID  由三部分组成:index +InfoSetID +ItemID,相同结构的信息可以放在同一个
 * 信息集里，不同的信息集可以组合成新的信息集。信息集里的信息项可以
 * 绑定到网管信号。也可以不绑定，需要访问网管信号时再具体实现。
 */

/********************************* Includes ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <typedefs.h>
#include <visipc.h>
/********************************** Defines ***********************************/
static pid_t cpid;
#define CCALL_NAME "ccenter"
#define MOD_BUS_NAME "mbus"

int findPid(char *name)
{
     char buf[100];
     sprintf(buf, "ps -A | grep %s", name);
     FILE *fp = popen(buf, "r");
     if (fgets(buf, sizeof(buf), fp) != NULL)
      {
          pclose(fp);
          return atoi(buf); /* 转换成PID*/
      }
     pclose(fp);
     return 0; /*发送0作为没找结束标志*/
}

pid_t findCCPid()
{
  return findPid(CCALL_NAME);
}
pid_t findModBusPid()
{
  return findPid(MOD_BUS_NAME);
}
int CreateMsgQ(uchar_t timeout)
{
 int msqid;
 /* 消息队列并非私有，因此此键值的消息队列很可能在其他进程已经被创建 
 所以这里尝试打开已经被创建的消息队列 
 */  
 //msqid = msgget(MSGKEY,0);  
 //if(msqid < 0) /*打开不成功，表明未被创建 现在可以按照标准方式创建消息队列*/  
  msqid = msgget(MSGKEY,IPC_CREAT|0666);  
 return msqid; 
}
int sendSig(pid_t pid,int sig,int  sigval)
{
  union sigval sig_val;

  if (pid<1)
  {
    printf("Process not run!pid=%d\n",pid);
    return -1;
  }
  sig_val.sival_int=sigval;
  if(sigqueue(pid,sig,sig_val)==-1)
  {
     perror("sendSig error\n");
     return -1;
  }
  return 0;
}
int sendSigToModBus(int sig,int  sigval)
{
  return sendSig(findPid(MOD_BUS_NAME),sig,sigval);
}
int sendSigToCC(int sig,int  sigval)
{
  return sendSig(findPid(CCALL_NAME),sig,sigval);
}

int getMsgQ(msgbuf_t *rcvbuf,boolean_t watting,uchar_t timeout)
{
    int infolen;
    long mtype=rcvbuf->mtype;
    int msgflg=0;
    int msgqid;
    msgbuf_t buf;
    //buf.mtype=12;    
    if (watting!=TRUE)
        msgflg= IPC_NOWAIT;
    msgqid= msgget(MSGKEY,0666);
    if (msgqid<0)
      return  msgqid;
    //printf("msgqid=%d\n",msgqid);
    //strcpy(buf.mbuf,"www"); 
    if((infolen=msgrcv(msgqid,rcvbuf,MAX_IPC_MSG_BUF,0,msgflg))<0)
    {
        if (errno == EINTR) //如果消息队列已满或其他情况无法送入消息，则立即返回 EAGIN
        {
          return -1;
        }
        else
        {
          return -2;
        }
    }
    
    if (mtype == rcvbuf->mtype)
      return infolen;
    else
      return 0;
}

int sendMsgQ(msgbuf_t sndbuf,boolean_t watting,uchar_t timeout)
{
 int msgflg=0;
 int msgqid;
 if (watting!=TRUE)
     msgflg= IPC_NOWAIT;
 msgqid= msgget(MSGKEY,IPC_CREAT|0666);
 if (msgqid<0)
 {
    perror("sendMsgQ msgget error");
    return  msgqid;
 }
 int mlen;
 mlen=msgsnd(msgqid,&sndbuf,MAX_IPC_MSG_BUF,msgflg);
 if (mlen==-1)
 {
   perror("sendMsgQ msgsnd error");
   //sleep(1);
   //SendSigToCCenter(10);
}
 //printf("sendMsgQ OK\n");
 return mlen;
}

struct itimerval itv;
static int result=0;
static void
alarmtimer(unsigned long sec, unsigned long usec)
{
  itv.it_value.tv_sec  = sec;
  itv.it_value.tv_usec = usec;
  itv.it_interval = itv.it_value;
  setitimer(ITIMER_REAL, &itv, NULL);
}

static void catch_sig(int sig)
{
  if (sig == SIGUSR1)
  { 
    printf("Require from unit complete!\n");
     sleep(1); 
    kill(cpid,SIGUSR2); 
    printf("Require from unit complete!!\n");
     result=1;
 }
 else if (sig == SIGUSR2)
 {
  printf("Require from unit complete2!\n");
     sleep(1);
  kill(cpid,SIGUSR2); 
  result=2;
  printf("Recieved SIGUSR2\n");
 }
  printf("SIG=%d\n",sig);
}

static void sig_handle(int sig,struct siginfo *siginfo,void *myact)
{
  if (sig == SIGUSR1)
  {
      printf("Rev SIGUSR1\n");
      result=siginfo->si_int;
      printf("Recieved from CC SIGUSR1 %d\n",result);
      result=1;
  }
  else if (sig == SIGALRM)
  {
  	printf("Timeout\n");
        result=2;
  }
  else
  {
      printf("SigNo:%d\n",sig);
      result=3;
  }
}
void watchdog(int sig)
{
  printf("watchdog\n");
}
static void sig_init()
{
  int i;
  struct sigaction act;
  sigemptyset(&act.sa_mask);  
  act.sa_flags=SA_SIGINFO;
  act.sa_sigaction=sig_handle;
  //act.sa_handler=handler;

  //sigaction(SIGUSR1,&act,NULL);
  sigaction(SIGALRM,&act,NULL); 
  //signal(SIGALRM, watchdog);
  sigaction(SIGINT,&act,NULL);
    sigaction(SIGQUIT,&act,NULL);
    sigaction(SIGTSTP,&act,NULL);
    sigaction(SIGTTIN,&act,NULL);
    sigaction(SIGTERM,&act,NULL);
    sigaction(SIGKILL,&act,NULL);
    sigaction(SIGUSR1,&act,NULL);
    sigaction(SIGUSR2,&act,NULL);
    sigaction(SIGRTMIN,&act,NULL);
}

int sendMsgToCC(msgbuf_t msgbuf,boolean_t watting,uchar_t timeout)
{
 // pid_t pid;
  int signum;
  union sigval sig_val;
 
  printf("Startting sending:%s\n",msgbuf.mbuf); 
  //cpid=GetPidFromFile("/var/run/ccenter.pid");
  cpid=findCCPid();
  if (cpid<1)
  {
   printf("pid=%d\n",cpid);
   return cpid;
  }
  signum=SIGUSR1;
  sig_val.sival_int=8;//不代表具体含义，只用于说明问题
  printf("Run here 1\n");
  if(sigqueue(cpid,signum,sig_val)==-1)
  {
	printf("send error!\n");
	return -1;
  }
  //usleep(300);
  printf("Startting sending\n");
 if (watting && timeout>0)
 {
    sig_init();
    alarmtimer(timeout,0);
  if (result==1)
   pause();
   if (result>0)
    {
      printf("Recieved from CC SIGUSR1 %d\n",result);
    }
    else
      printf("Not recieved from CC %d\n",result);
   return 1;
}
  printf("Completed\n");
  return 0;
}


/******************************************************************************/




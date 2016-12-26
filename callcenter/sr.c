#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <bits/siginfo.h>
void new_op(int signum,siginfo_t *info,void *myact)
{
    printf("receive signal %d", signum);
    //char *msg=(char*)info->si_value.sival_ptr;
    //printf(" receive infomation is:%s\n",msg);
    int msg=info->si_value.sival_int;
    printf("receive information is %d",msg);
    sleep(5);
}
int main()
{    
    pid_t pid;
    pid=getpid();
    printf("my pid is %d",pid);    
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags=SA_SIGINFO;
    act.sa_sigaction=new_op;
    int result;
    result=sigaction(SIGUSR2,&act,NULL);
    if(result < 0)
    {
    printf("install sigal error\n");
    }    
    while(1)
    {
    sleep(2);
    //printf("wait for the signal\n");
    }
}



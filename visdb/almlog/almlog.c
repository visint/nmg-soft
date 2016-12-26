#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <unit.h>
#include <sqlite3.h>
#include <dbutil.h>
//gcc -o /usr/local/bin/almlog -lvisipc -lvisdb ../vislib/visstr.o almlog.c  -I../include

//gcc -o /usr/local/bin/almlog libdbutil.a  almlog.c  -I../include

static void usage()
{
  fprintf(stderr,"alarm log ver %s 1.0.3 lang:%s\n",VERSION,DEFAULT_LANG);
  
  printf("Option Specification:\n");
  printf("-i [id|from..to]: alarm id\n");
  printf("-a code: alarm code\n");
  printf("-c [<0..31>]: chassis number\n");
  printf("-s [<0..15>]: slot number\n");
  printf("-e [<0..4>]: entity number\n");
  printf("[-A|-C|-D|-I]: -A: ack -C: clean -I: insert alarm ,-D: delete\n");
  printf("-h for help.\n");
}

int main (int argc, char **argv)
{
    int oc,chassis=0,alarm_code=0,alarm_id=0,slot=0,entity=0;                    
    char opt=0;           

    while((oc = getopt(argc, argv, "a:c:e:i:hs:t:ACDIS")) != -1)
    {
        switch(oc)
        {
            case 'a':
                alarm_code=atoi(optarg);
                break;
            case 'c':
                chassis=atoi(optarg);
                //printf("Chassis is %s.\n",optarg);
                break;
            case 'i':
                alarm_id=atoi(optarg);
                break;
            case 's':
                //b_opt_arg = optarg;
                slot=atoi(optarg);
                //printf("Unit is #%s\n", optarg);
                break;
            case 'e':
                //b_opt_arg = optarg;
                entity=atoi(optarg);
                printf("infoset id is %s\n", optarg);
                break;
          case 'A':
          case 'C':
          case 'D':
          case 'I':
                if (opt!=0)
                {
                  usage();
                  return 0;
                }
                opt=oc;
                break;
          case 'S':
                  showTable(NULL,"SELECT * FROM AlarmType");
                  printf("row count=%d\n",getRowCount(NULL,"SELECT count(*) FROM AlarmType"));
                 return;
          case 'h':
                  usage();
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

  if (opt==0)
  {
      usage();
      return 0;
   }
  switch(opt)
  {
     case 'A':
             //almlog -s 13 -A -a 61 -e 0
            //printf("ACK.\n");
            if (ackAlarmLog(alarm_code,chassis,slot,entity))
             {
              usage();
             }
              return 0;
     break;
     case 'C':
              //almlog -s 13 -C -a 61 -e 0
             //printf("Clean.\n");
             if (cleanAlarmLog(alarm_code,chassis,slot,entity))
             {
              usage();
             }
              return 0;
     case 'I':
              //almlog -s 13 -I -a 61 -e 0
             //printf("Insert.\n");
             if (insertAlarmLog(alarm_code,chassis,slot,entity))
             {
              usage();
             }
              return 0;
     case 'D':
              //dbm -a -s 4 -f 21 -i 1
             //printf("Delete.\n");
           break;
     default:
             usage();
             return 0;
  }
   // printf("opterr=%d\n",opterr);
   return 0;

}


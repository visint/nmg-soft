#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <unit.h>
#include <sqlite3.h>
#include <dbutil.h>
//gcc -o /usr/local/bin/dbm -lvisipc -lvisdb ../vislib/unit.o ../vislib/visstr.o dbman.c  -I../include
static void showUserData(uchar_t chassis,uchar_t slot)
{
  unit_user_data_t udata;
  if (TRUE!=getUserDataFromDB(chassis,slot,&udata))
    return;
  printf("%s",udata.name);
}
static void usage()
{
  fprintf(stderr,"DB application ver 1.0.1\n");
  
  printf("Option Specification:\n");
  printf("-b [<1..16>]: show unit base info\n");
  printf("-u [1..16]: show unit user info\n");
  printf("-t table_name: show table\n");
  printf("-m : show info item\n");
  printf("-e <1..16>: show unit status\n");
  printf("-i <infoset id>: show infoset status\n");
  //printf("-a : show all\n");
  printf("-h for help.\n");
}

int main (int argc, char **argv)
{
    int oc,chassis=0,slot=0,infoset_id=0,item_id=0,item_value=0;                    
    char opt=0,rw=-1,field_value[32]="",*b_opt_arg;           

    while((oc = getopt(argc, argv, "abc:f:i:n:s:t:uAD:I:GS")) != -1)
    {
        switch(oc)
        {
            case 'a':
                opt='a';
                break;
            case 'c':
                chassis=atoi(optarg);
                //printf("Chassis is %s.\n",optarg);
                break;
            case 'n':
                b_opt_arg = optarg;
                strncpy(field_value, optarg,30);
                //printf("Unit name is %s\n", optarg);
                break;
            case 's':
                b_opt_arg = optarg;
                slot=atoi(optarg);
                printf("Unit is #%s\n", optarg);
                break;
            case 't':
                printf("Show table.\n");
                showTable(NULL,optarg);
                return 0;
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
          case 'D':
                printf("Show table.\n");
                showTable(NULL,optarg);
                return 0;
          case 'A':
                printf("Show Alarm.\n");
                //showAlarm();
                return 0;
         case 'I':
                printf("Show Infoset.\n");
                //showInfoset(atoi(optarg),1);
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
  //printf("opt is %c rw=%c\n", opt,rw);
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
              //dbapp -a -s 4 -f 21 -i 1
             insertAlarmLog(infoset_id,chassis,slot,item_id);
           break;
     default:
             usage();
             return 0;
  }
   // printf("opterr=%d\n",opterr);
   return 0;

}

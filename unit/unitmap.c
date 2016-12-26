#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <unit.h>
char GetBaseInfoSet(uchar_t chassis,uchar_t slot,unit_base_info_t *bi)
{
  int fd,i;
  unit_base_info_t *p_map;
  fd=open(BASE_INFO_PATH,O_CREAT|O_RDWR,00777 );
  p_map = (unit_base_info_t*)mmap(NULL,sizeof(unit_base_info_t)*(MAX_UNIT_COUNT+1),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
  for(i = 0;i<MAX_UNIT_COUNT;i++)
  {
   printf("Unit:%d name %s\n",i+1,(*(p_map+i)).name);
  }
  munmap(p_map,sizeof(unit_base_info_t)*MAX_UNIT_COUNT);
}
void SetBaseInfo(uchar_t chassis,uchar_t slot,uchar_t item_id,const char *item_value)
{
 unit_base_info_t unit_base_info;
}
void SetBaseInfoSet(uchar_t chassis,uchar_t slot,unit_base_info_t bi)
{
  int fd;
  unit_base_info_t *p_map;
  char item[32]="";
  
  fd=open(BASE_INFO_PATH,O_CREAT|O_RDWR,00777);
  lseek(fd,sizeof(unit_base_info_t)*MAX_UNIT_COUNT,SEEK_SET);
  write(fd,"",1);
  printf("Run %d\n",1); 
  p_map = (unit_base_info_t*) mmap(NULL,sizeof(unit_base_info_t)*(MAX_UNIT_COUNT+1),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
  printf("Run %d\n",2); 
  memcpy(( *(p_map+slot-1) ).name, bi.name,strlen(bi.name));
  printf("Run %d\n",1);
  memcpy(( *(p_map+slot-1) ).model, bi.model,strlen(bi.model));
  memcpy(( *(p_map+slot-1) ).location,bi.location,strlen(bi.location)); 
  close(fd);
  munmap(p_map, sizeof(unit_base_info_t)*(MAX_UNIT_COUNT+1) );
}

void main(int argc, char **argv)
{
   int slot=0;
   char *name, *value, buf[60];
	unit_base_info_t bi;
      /* 
	 * Skip program name 
	 */
	--argc;
	++argv;

	if (!*argv)
            return;	
        /* 
	 * Process the remaining arguments. 
	 */
	for (; *argv; argv++) {
          if (!strncmp(*argv, "get", 3)) {
		if (*++argv) {
	         GetBaseInfoSet(0,1,&bi);	
                }
   	  } else if (!strncmp(*argv, "set", 3)) {
       	     if (*++argv) {
                value=buf;	
                printf("argv:%s\n",*argv);	
                strncpy(buf, *argv, sizeof(buf));
	        printf("value:%s\n",value); 
                slot= 2; //atoi(strsep(&value, " "));	
                printf("slot=%d\n",slot); 
                name = strsep(&value, " ");
                strcpy(bi.name,"This is model");
                strcpy(bi.model,"This is model");
                strcpy(bi.location,"This is location");
                printf("slot=%d\n",slot);
                if(slot>0 && slot<17)
                SetBaseInfoSet(1,slot,bi);
             }
       	  } 
	if (!*argv)
		break;
	} 
}

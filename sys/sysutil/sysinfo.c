#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include  <sysinfo.h>
#include  <iniconf.h>
#include  <unit.h>
#define DEFAULT_IP "192.168.2.100"
#define DEFAULT_SUBNET_MASK "255.255.255.0"
#define DEFAULT_GATEWAY "192.168.2.1"
/*
int getSysInfo(uchar_t chassis,sysInfo_t *sys_info)
{
  //status_infoset_t *getStatusInfoSetMap(chassis,0,uchar_t infoset_id,1);
}
*/

void restore(int chassis,int slot)
{
  if (slot<0 || slot>MAX_UNIT_COUNT)
     return;
  if (MAX_UNIT_COUNT==slot)
  {
   setDeviceIp(DEFAULT_IP,DEFAULT_SUBNET_MASK,DEFAULT_GATEWAY);
   system("cp /vispace/webs/umconfig.def /vispace/webs/umconfig.txt");
   restoreSnmpdConf(0);
   printf("Reboot ...\n");
   system("reboot");
  }
}
int getDeviceIp(char *ip_add,char *mask,char *gateway)
{
  
  if (NULL!=ip_add)
  {
       if (getConfValue("/etc/network/interfaces","address",ip_add," "))
     //if (visConfGetKey("DevIp", "ipAdd",ip_add))
          return -1;
  }
  if (NULL!=mask)
  {
     //if (visConfGetKey("DevIp", "mask",mask))
        if (getConfValue("/etc/network/interfaces","netmask",mask," "))
          return -1;
  }
  if (NULL!=gateway)
  {
     //if (visConfGetKey("DevIp", "gateway",gateway))
       if (getConfValue("/etc/network/interfaces","gateway",gateway," "))
          return -1;
  }

  return 0;
}
int setDeviceIp(char *ip_addr,char *mask,char *gateway)
{
  char modified=0;
  if (NULL!=ip_addr)
  {
     //doSystem("ifconfig eth0 %s",ip_addr);
     //doSystem(
     //if (visConfSetKey( "DevIp", "ipAdd",ip_addr))
      if (setConfValue("/etc/network/interfaces","address",ip_addr," "))
          return -1;
        modified=1;
  }
  if (NULL!=mask)
  {
     //if (visConfSetKey("DevIp", "mask",mask))
     if (setConfValue("/etc/network/interfaces","netmask",mask," "))
          return -1;
        modified=1;
  }
  if (modified)
      doSystem("ifconfig eth0 %s netmask %s up",ip_addr,mask);
  if (NULL!=gateway)
  {
     //if (visConfSetKey("DevIp", "gateway",gateway))
       if (setConfValue("/etc/network/interfaces","gateway",gateway," "))
          return -1;
        doSystem("route add default gw %s",gateway);
  }
  return 0;
}

int getMac(char* mac)

{
    struct ifreq tmp;
    int sock_mac;
    char mac_addr[30];
    sock_mac = socket(AF_INET, SOCK_STREAM, 0);
    if( sock_mac == -1){
        perror("create socket fail\n");
        return -1;
    }
    memset(&tmp,0,sizeof(tmp));
    strncpy(tmp.ifr_name,"eth0",sizeof(tmp.ifr_name)-1 );
    if( (ioctl( sock_mac, SIOCGIFHWADDR, &tmp)) < 0 ){
        printf("mac ioctl error\n");
        return -1;
    }
    
    sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)tmp.ifr_hwaddr.sa_data[0],
            (unsigned char)tmp.ifr_hwaddr.sa_data[1],
            (unsigned char)tmp.ifr_hwaddr.sa_data[2],
            (unsigned char)tmp.ifr_hwaddr.sa_data[3],
            (unsigned char)tmp.ifr_hwaddr.sa_data[4],
            (unsigned char)tmp.ifr_hwaddr.sa_data[5]
            );
    //printf("local mac:%s\n", mac_addr);
    close(sock_mac);
    memcpy(mac,mac_addr,strlen(mac_addr));
    return 0;
}


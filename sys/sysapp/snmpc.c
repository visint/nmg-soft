#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <unit.h>
#include <sqlite3.h>
#include <dbutil.h>

int main (int argc, char **argv)
{
  insertCommunity("192.168.1.238","public","private",2,1);
  insertDestHost("192.168.1.238",9162,1);
}

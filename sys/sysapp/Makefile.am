AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = ipconf snmpc watchdogd
LIBS=-lvispace -lsqlite3 -liconv
ipconf_SOURCES =  devip.c
ipconf_LDADD = ../sysutil/libsysutil.a -lsqlite3
snmpc_SOURCES =  snmpc.c
snmpc_LDADD = ../sysutil/libsysutil.a ../../visdb/dbutil/libdbutil.a -lsqlite3
INCLUDES=-I$(CURRENTPATH)./../../include



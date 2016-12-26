AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = dbman dbapp

dbman_SOURCES =  dbman.c
LIBS=-lvispace -lsqlite3 -liconv
#dbman_LDADD = ../../lib/libvispace.a -lsqlite3
dbman_LDADD = -L../../lib
dbapp_SOURCES =  dbapp.c
#dbapp_LDADD = ../dbutil/libdbutil.a -lsqlite3
dbapp_LDADD = -L../../lib
INCLUDES=-I$(CURRENTPATH)./../../include


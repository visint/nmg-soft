AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS =  uset ushow

uset_SOURCES =  unitset.c
ushow_SOURCES =  unitshow.c

ushow_LDADD =../../visstr/libvisstr.a ../unitutil/libunit.a ../../visipc/ipcutil/libipc.a ../../visdb/dbutil/libdbutil.a -lsqlite3
uset_LDADD =../../visstr/libvisstr.a ../unitutil/libunit.a ../../visipc/ipcutil/libipc.a ../../visdb/dbutil/libdbutil.a -lsqlite3
INCLUDES=-I$(CURRENTPATH)./../../include


AUTOMAKE_OPTIONS = foreign
lib_LIBRARIES = libvispace.a

libvispace_a_SOURCES = ../unit/unitutil/unit.c ../unit/otu/otu.c ../unit/olp/olp.c ../unit/ddm/ddm.c ../unit/chassis/chassis.c\
../visipc/ipcutil/sigmsg.c ../visipc/ipcutil/shmutil.c ../visipc/ipcutil/mbusshm.c \
../sys/sysutil/sysinfo.c ../sys/sysutil/snmputil.c ../sys/sysutil/logutil.c \
../visdb/dbutil/dbutil.c ../visdb/dbutil/almutil.c ../visstr/visstr.c \
../utils/iniconf.c ../utils/shutils.c ../infoset/infoset.c ../io/ioutil.c
#LIBS= ../../visstr/libvisstr.a
#libutils_a_LDFLAGS = -version-info 0:0:0
INCLUDES=-I$(CURRENTPATH)./../include

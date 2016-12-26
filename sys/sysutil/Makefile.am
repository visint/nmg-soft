AUTOMAKE_OPTIONS = foreign
lib_LIBRARIES = libsysutil.a

libsysutil_a_SOURCES = sysinfo.c snmputil.c logutil.c
#LIBS=../../utils/libutils.a
libsysutil_LDADD=../utils/libutils.a
libsysutil_a_LDFLAGS = -version-info 0:0:0
INCLUDES=-I$(CURRENTPATH)./../../include

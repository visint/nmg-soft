AUTOMAKE_OPTS = -Wno-portability
bin_PROGRAMS = almlog

almlog_SOURCES = almlog.c
#almlog_LDADD = ../dbutil/libdbutil.a -lsqlite3
almlog_LDADD = ../../lib/libvispace.a -lsqlite3
INCLUDES=-I$(CURRENTPATH)./../../include
almlog_CFLAGS  = -DVERSION=\"$(VERSION)\" -DDEFAULT_LANG=\"$(DEFAULT_LANG)\"

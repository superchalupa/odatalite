# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

sbin_PROGRAMS += out/phitd

#EXTRA_DIST += src/phitd-include
phitd_includes = src/server src src/odata

PHITD_LIBLIST = \
	out/libodataplugin.la \
	out/libodata.la \
	out/libbase.la

out_phitd_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,PHITD) \
	$(PAM_LIBS)

out_phitd_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(phitd_includes))

out_phitd_LDADD = $(PHITD_LIBLIST)

out_phitd_DEPENDENCIES=$(PHITD_LIBLIST)

out_phitd_SOURCES = \
	src/server/listener.c \
	src/server/defaultplugin.c \
	src/server/plugin.c \
	src/server/help.c \
	src/server/server.c \
	src/server/disp.c \
	src/server/connection.c \
	src/server/context.c \
	src/server/webpage.c \
	\
	src/server/connection.h \
	src/server/context.h \
	src/server/contextoptions.h \
	src/server/defaultplugin.h \
	src/server/disp.h \
	src/server/help.h \
	src/server/listener.h \
	src/server/options.h \
	src/server/plugin.h \
	src/server/webpage.h

# TODO: phitd will write files to /var:
#       /var/log/phitrecv.log
#       /var/log/phitsend.log
#       /var/run/phitd.pid

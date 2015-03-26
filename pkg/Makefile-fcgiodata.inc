# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

bin_PROGRAMS += out/fcgiodata
out_fcgiodata_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes)) $(FASTCGI_CFLAGS)
out_fcgiodata_LDADD = out/libbase.la out/libclient.la $(FASTCGI_LIBS) out/libodataplugin.la
out_fcgiodata_SOURCES = \
	src/server/context.c	\
	src/server/connection.c	\
	src/fcgiodata/fcgi_connection.c	\
	src/fcgiodata/fcgi_connection.h	\
	src/fcgiodata/http.c	\
	src/fcgiodata/fcgiodata.c

# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

bin_PROGRAMS += out/fcgiodata-async
out_fcgiodata_async_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes)) $(FASTCGI_CFLAGS) $(CZMQ_CFLAGS)
out_fcgiodata_async_LDADD = out/libbase.la out/libclient.la $(FASTCGI_LIBS) out/libodataplugin.la $(CZMQ_LIBS)
out_fcgiodata_async_SOURCES = \
	src/fcgiodata/context.c	\
	src/fcgiodata/context.h	\
	src/fcgiodata/connection.c	\
	src/fcgiodata/connection.h	\
	src/fcgiodata/http.c	\
	src/fcgiodata/fcgiodata-async.c


# Apache2 conf file for fastcgi
dist_apache_conf_DATA += share/apache/fcgiodata-async.conf

# odata service install
dist_systemdsystemunit_DATA += share/apache/fcgiodata-async.service

# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

bin_PROGRAMS += out/fcgiodata
out_fcgiodata_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes)) \
	$(FASTCGI_CFLAGS) $(CZMQ_CFLAGS) $(SYSTEMD_CFLAGS)
out_fcgiodata_LDADD = out/libbase.la out/libclient.la $(FASTCGI_LIBS) out/libodataplugin.la $(CZMQ_LIBS) $(SYSTEMD_LIBS)
out_fcgiodata_SOURCES = \
	src/fcgiodata/context.c	\
	src/fcgiodata/context.h	\
	src/fcgiodata/connection.c	\
	src/fcgiodata/connection.h	\
	src/fcgiodata/http.c	\
	src/fcgiodata/fcgiodata-async.c


# Apache2 conf file for fastcgi
dist_apache_conf_DATA += share/apache/fcgiodata.conf share/apache/01-load-modules.conf

# odata service install
dist_systemdsystemunit_DATA += share/apache/fcgiodata.service share/apache/fcgiodata.socket

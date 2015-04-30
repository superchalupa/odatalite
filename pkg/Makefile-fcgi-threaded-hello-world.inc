# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

bin_PROGRAMS += out/fcgi-threaded-hello-world
out_fcgi_threaded_hello_world_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes)) \
	$(FASTCGI_CFLAGS) $(CZMQ_CFLAGS) $(SYSTEMD_CFLAGS)

out_fcgi_threaded_hello_world_LDADD = $(FASTCGI_LIBS) $(CZMQ_LIBS) $(SYSTEMD_LIBS)
out_fcgi_threaded_hello_world_SOURCES = src/benchmark/fcgi-threaded-hello-world.c

# Apache2 conf file for fastcgi
dist_apache_conf_DATA += share/apache/fcgi-threaded-hello-world.conf

# odata service install
dist_systemdsystemunit_DATA += share/apache/fcgi-threaded-hello-world.service share/apache/fcgi-threaded-hello-world.socket



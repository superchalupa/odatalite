# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

bin_PROGRAMS += out/fcgiauth
out_fcgiauth_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes)) \
	$(FASTCGI_CFLAGS) 
out_fcgiauth_LDADD = out/libbase.la out/libclient.la $(FASTCGI_LIBS) -lpam
out_fcgiauth_SOURCES = \
	src/fcgiauth/fcgiauth.c

# Apache2 conf file for fastcgi
dist_apache_conf_DATA += share/apache/fcgiauth.conf

# odata service install
dist_systemdsystemunit_DATA += share/apache/fcgiauth.service share/apache/fcgiauth.socket

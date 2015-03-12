# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

bin_PROGRAMS += out/fcgitest
out_fcgitest_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(bin_includes)) $(FASTCGI_CFLAGS)
out_fcgitest_LDADD = out/libbase.la out/libclient.la $(FASTCGI_LIBS)
out_fcgitest_SOURCES = \
	src/fcgitest/fcgitest.c

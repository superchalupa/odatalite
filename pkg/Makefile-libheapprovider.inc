# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

provider_LTLIBRARIES += out/libheapprovider.la
out_libheapprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))
out_libheapprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,HEAPPROVIDER) -avoid-version -fvisibility=hidden
out_libheapprovider_la_LIBADD =

out_libheapprovider_la_SOURCES = \
	src/odata/providers/heap/HeapProvider.cpp \
	src/odata/providers/heap/HeapProvider.h

INSTALL_COMMANDS += ;echo  'heapprovider:/GetHeapUsage:' > $(DESTDIR)/$(phit_confdir)/providers.d/heapprovider.conf

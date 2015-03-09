# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

provider_LTLIBRARIES += out/librootprovider.la
out_librootprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,ROOTPROVIDER) -avoid-version
out_librootprovider_la_LIBADD = 
out_librootprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))

out_librootprovider_la_SOURCES = \
	./src/odata/providers/root/RootProvider.h \
	./src/odata/providers/root/RootProvider.cpp

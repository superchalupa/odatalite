# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

provider_LTLIBRARIES += out/librootprovider.la
out_librootprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))
out_librootprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,ROOTPROVIDER) -avoid-version -fvisibility=hidden
out_librootprovider_la_LIBADD = out/libbase.la

out_librootprovider_la_SOURCES = \
	src/odata/providers/root/rootprovider.c

INSTALL_COMMANDS += ;echo  'rootprovider:/:' >> $(DESTDIR)/$(phit_confdir)/providers.conf

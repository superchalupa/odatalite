# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

provider_LTLIBRARIES += out/libcheckprovider.la
out_libcheckprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,CHECKPROVIDER) -avoid-version
out_libcheckprovider_la_LIBADD = 
out_libcheckprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))

out_libcheckprovider_la_SOURCES = \
	./src/odata/providers/check/checkprovider.c

INSTALL_COMMANDS += ;echo  'checkprovider:/Check:' >> $(DESTDIR)/$(phit_confdir)/providers.conf

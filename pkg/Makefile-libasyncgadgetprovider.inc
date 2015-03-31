# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

provider_LTLIBRARIES += out/libasyncgadgetprovider.la
out_libasyncgadgetprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))
out_libasyncgadgetprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,WIDGETPROVIDER) -avoid-version -fvisibility=hidden
out_libasyncgadgetprovider_la_LIBADD = out/libbase.la

out_libasyncgadgetprovider_la_SOURCES = \
	src/odata/providers/gadget/async_gadgetprovider.c

INSTALL_COMMANDS += ;echo  'asyncgadgetprovider:/AGadgets,/AGadgets/Widgets:count,top,skip' >> $(DESTDIR)/$(phit_confdir)/providers.conf

# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

provider_LTLIBRARIES += out/libwidgetprovider.la
out_libwidgetprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))
out_libwidgetprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,WIDGETPROVIDER) -avoid-version -fvisibility=hidden
out_libwidgetprovider_la_LIBADD = out/libbase.la

out_libwidgetprovider_la_SOURCES = \
	src/odata/providers/widget/widgetprovider.c

INSTALL_COMMANDS += ;echo  'widgetprovider:/Widgets:' > $(DESTDIR)/$(phit_confdir)/providers.d/widgetprovider.conf

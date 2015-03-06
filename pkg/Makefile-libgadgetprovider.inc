# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

lib_LTLIBRARIES += out/libgadgetprovider.la
out_libgadgetprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,WIDGETPROVIDER)
out_libgadgetprovider_la_LIBADD = out/libbase.la
out_libgadgetprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))

out_libgadgetprovider_la_SOURCES = \
	./src/odata/providers/gadget/gadgetprovider.c


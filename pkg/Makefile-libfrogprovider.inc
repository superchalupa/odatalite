# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

lib_LTLIBRARIES += out/libfrogprovider.la
out_libfrogprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,FROGPROVIDER)
out_libfrogprovider_la_LIBADD = 
out_libfrogprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))

out_libfrogprovider_la_SOURCES = \
	./src/odata/providers/frog/FrogProvider.cpp \
	./src/odata/providers/frog/FrogProvider.h \
	./src/odata/providers/frog/strings.h


# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

lib_LTLIBRARIES += out/libmetadataprovider.la
out_libmetadataprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,METADATAPROVIDER)
out_libmetadataprovider_la_LIBADD = out/libbase.la
out_libmetadataprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))

out_libmetadataprovider_la_SOURCES = \
	./src/odata/providers/metadata/metadataprovider.c \
	./src/odata/providers/metadata/ServiceRoot.c

#gen: TODO: need to build csdl2c-native
#	../../../csdl2c/csdl2c ../../../RedfishSchema/ServiceRoot.0.9.2.metadata


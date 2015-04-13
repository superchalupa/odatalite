# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

provider_LTLIBRARIES += out/libmetadataprovider.la
out_libmetadataprovider_la_CPPFLAGS = $(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(provider_includes))
out_libmetadataprovider_la_LDFLAGS = $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,METADATAPROVIDER) -avoid-version -fvisibility=hidden
# for Calloc
out_libmetadataprovider_la_LIBADD = out/libbase.la

out_libmetadataprovider_la_SOURCES = \
	src/odata/providers/metadata/metadataprovider.c

nodist_out_libmetadataprovider_la_SOURCES = \
	src/RedfishSchema/ServiceRoot.c

DISTCLEANFILES += $(nodist_out_libmetadataprovider_la_SOURCES)

EXTRA_DIST += src/RedfishSchema/ServiceRoot.0.94.0.metadata

INSTALL_COMMANDS += ;echo  'metadataprovider:/$$metadata:' >> $(DESTDIR)/$(phit_confdir)/providers.conf

# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

pingplugin_includes = src/plugins/pingplugin src/odata src

# this is a convenience library so we dont recompile
# pingplugin .c files repeatedly for each binary
lib_LTLIBRARIES += out/libpingplugin.la
out_libpingplugin_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(pingplugin_includes))

out_libpingplugin_la_LDFLAGS = -avoid-version $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,PINGPLUGIN)
out_libpingplugin_la_LIBADD = out/libbase.la

out_libpingplugin_la_SOURCES = \
	src/plugins/ping/pingplugin.c

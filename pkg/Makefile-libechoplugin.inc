# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

echoplugin_includes = src/plugins/echoplugin src/odata src

# this is a convenience library so we dont recompile
# echoplugin .c files repeatedly for each binary
lib_LTLIBRARIES += out/libechoplugin.la
out_libechoplugin_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(echoplugin_includes))

out_libechoplugin_la_LDFLAGS = -static

out_libechoplugin_la_SOURCES = \
	src/plugins/echo/echoplugin.c 

# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

helloplugin_includes = src/plugins/helloplugin src/odata src

# this is a convenience library so we dont recompile
# helloplugin .c files repeatedly for each binary
lib_LTLIBRARIES += out/libhelloplugin.la
out_libhelloplugin_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(helloplugin_includes))

out_libhelloplugin_la_LDFLAGS = -static

out_libhelloplugin_la_SOURCES = \
	src/plugins/hello/helloplugin.c

INSTALL_COMMANDS += ;echo  'Hello:GET:/hello:helloplugin:hello' >> $(DESTDIR)/$(phit_confdir)/plugins.conf

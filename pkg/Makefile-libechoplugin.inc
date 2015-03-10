# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

echoplugin_includes = src/plugins/echoplugin src/odata src

# this is a convenience library so we dont recompile
# echoplugin .c files repeatedly for each binary
lib_LTLIBRARIES += out/libechoplugin.la
out_libechoplugin_la_CPPFLAGS = \
	$(AM_CPPFLAGS) $(addprefix -I$(top_srcdir)/,$(echoplugin_includes))

out_libechoplugin_la_LDFLAGS = -avoid-version $(AM_LDFLAGS) $(call STD_LDFLAGS_LIB,ECHOPLUGIN)
out_libechoplugin_la_LIBADD = out/libbase.la

out_libechoplugin_la_SOURCES = \
	src/plugins/echo/echoplugin.c 

INSTALL_COMMANDS += ;echo 'Echo:M-POST:/echo:echoplugin:echo/webpage' >> $(DESTDIR)/$(phit_confdir)/plugins.conf
INSTALL_COMMANDS += ;echo 'Echo:POST:/echo:echoplugin:echo/webpage' >> $(DESTDIR)/$(phit_confdir)/plugins.conf
INSTALL_COMMANDS += ;echo 'Echo:GET:/echo/webpage:echoplugin:echo/webpage' >> $(DESTDIR)/$(phit_confdir)/plugins.conf

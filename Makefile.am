# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

# standard location to put our stuff out of the way
ACLOCAL_AMFLAGS = -I pkg/m4

DISTCHECK_CONFIGURE_FLAGS = \
	--build=$(build_alias) \
	--host=$(host_alias) \
	--target=$(target_alias) \
	--with-libtool-sysroot=$(SYSROOT_PATH)	\
        --with-systemdsystemunitdir=$$dc_install_base/$(systemdsystemunitdir)

# normally empty
SUBDIRS =

include pkg/Makefile-compileopts.inc
include pkg/Makefile-functions.inc

# directory to install providers in. This is only for documentation purposes, it will always be pkglibexecdir.
providerdir=$(pkglibexecdir)
provider_LTLIBRARIES=

# most providers use same includes, so define it once
provider_includes = src
bin_includes = src

# generate .c files from .metadata files
# NOTE: version encoded in files, unfortunately...
%.c: %.0.9.2.metadata out/csdl2c
	PATH=$$PATH:$(top_builddir)/out csdl2c $<
	mv $(notdir $@) $@

INSTALL_COMMANDS=:

include pkg/Makefile-libbase.inc
include pkg/Makefile-libodata.inc
include pkg/Makefile-libclient.inc

include pkg/Makefile-tools.inc
include pkg/Makefile-json.inc
include pkg/Makefile-phitcli.inc
include pkg/Makefile-olcli.inc
include pkg/Makefile-csdl2c.inc
include pkg/Makefile-testcsdl.inc

if BUILD_PHITD
# can't build phitd without the odata plugin due to static linking
include pkg/Makefile-libodataplugin.inc
include pkg/Makefile-phitd.inc
endif
if BUILD_ECHO_PLUGIN
include pkg/Makefile-libechoplugin.inc
endif
if BUILD_PING_PLUGIN
include pkg/Makefile-libpingplugin.inc
endif
if BUILD_HELLO_PLUGIN
include pkg/Makefile-libhelloplugin.inc
endif

if BUILD_METADATA_PROVIDER
include pkg/Makefile-libmetadataprovider.inc
endif

if BUILD_ROOT_PROVIDER
include pkg/Makefile-librootprovider.inc
endif
if BUILD_WIDGET_PROVIDER
include pkg/Makefile-libwidgetprovider.inc
endif
if BUILD_FROG_PROVIDER
include pkg/Makefile-libfrogprovider.inc
endif
if BUILD_GADGET_PROVIDER
include pkg/Makefile-libgadgetprovider.inc
endif
if BUILD_HEAP_PROVIDER
include pkg/Makefile-libheapprovider.inc
endif
if BUILD_CHECK_PROVIDER
include pkg/Makefile-libcheckprovider.inc
endif

# config files
phit_confdir=$(sysconfdir)/phit
dist_phit_conf_DATA = etc/phit/phitd.conf  etc/phit/plugins.conf  etc/phit/providers.conf  etc/phit/roles.conf

# PAM config
pam_confdir=$(sysconfdir)/pam.d
dist_pam_conf_DATA=etc/pam.d/phit

# tmpfiles
tmpfiledir=$(libdir)/tmpfiles.d
dist_tmpfile_DATA = etc/tmpfiles.d/phit.conf

# HTML stuff
phit_datadir=$(datadir)/phit
dist_phit_data_DATA = share/phit/background.gif share/phit/postamble.html share/phit/odata.html share/phit/preamble.html

install-data-hook:
	mkdir -p $(DESTDIR)/$(pkgrundir)
	$(INSTALL_COMMANDS)

sdk_HEADERS += \
src/odata/odatacxx.h	\
src/odata/odatacxxinlines.h	\
src/odata/odata.h	\
src/phit.h

EXTRA_DIST += 	\
src/server/help.h	\
src/server/context.h	\
src/server/webpage.h	\
src/server/contextoptions.h	\
src/server/defaultplugin.h	\
src/server/listener.h	\
src/server/disp.h	\
src/server/connection.h	\
src/server/plugin.h	\
src/server/options.h	\
src/common.h	\
src/plugins/odata/scope.h	\
src/plugins/odata/post.h	\
src/plugins/odata/webpage.h	\
src/plugins/odata/providers.h	\
src/plugins/odata/options.h	\
src/plugins/odata/odataplugin.h	\
src/winconfig.h	\
src/json2schema/objects.h	\
src/phitfeatures.h	\
src/phitcli/options.h	\
src/tests/test.h	\
src/base/log.h	\
src/base/shlib.h	\
src/base/sock.h	\
src/base/base64.h	\
src/base/role.h	\
src/base/path.h	\
src/base/httpstatuscodes.h	\
src/base/buf.h	\
src/base/array.h	\
src/base/env.h	\
src/base/dump.h	\
src/base/plugininfo.h	\
src/base/html.h	\
src/base/user.h	\
src/base/alloc.h	\
src/base/conffile.h	\
src/base/format.h	\
src/base/ptrarray.h	\
src/base/addr.h	\
src/base/roundpow2.h	\
src/base/cleanup.h	\
src/base/auth.h	\
src/base/find.h	\
src/base/pam.h	\
src/base/datetime.h	\
src/base/process.h	\
src/base/str.h	\
src/base/err.h	\
src/base/iprintf.h	\
src/base/getopt.h	\
src/base/pidfile.h	\
src/base/parse.h	\
src/base/file.h	\
src/base/selector.h	\
src/base/http.h	\
src/base/chars.h	\
src/client/client.h	\
src/odata/providerentry.h	\
src/odata/uriparser.h	\
src/odata/jsonobject.h	\
src/odata/name.h	\
src/odata/types.h	\
src/odata/scope.h	\
src/odata/odatamacros.h	\
src/odata/providers/heap/HeapProvider.h	\
src/odata/providers/frog/FrogProvider.h	\
src/odata/providers/frog/strings.h	\
src/odata/providers/root/RootProvider.h	\
src/odata/queryoption.h	\
src/odata/scratchbuf.h	\
src/odata/alloc.h	\
src/odata/format.h	\
src/odata/jsonparser.h	\
src/odata/uri.h	\
src/odata/outstream.h	\
src/odata/getters.h	\
src/odata/pair.h	\
src/odata/compare.h	\
src/odata/return.h	\
src/odata/csdl.h	\
src/odata/result.h	\
src/odata/destructors.h	\
src/odata/setters.h	\
src/odata/serialize.h	\
src/odata/jsonarray.h	\
src/odata/json.h	\
src/odata/chars.h	\
src/csdl2c/xml.h	\
src/olcli/options.h	\
src/config.h

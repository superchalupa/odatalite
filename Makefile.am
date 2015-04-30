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

# password files
dist_sysconf_DATA = etc/redfish_passwd

# Apache2 conf file for fastcgi
apache_confdir=$(sysconfdir)/apache2/conf.d
dist_apache_conf_DATA =

# odata service install
dist_systemdsystemunit_DATA =

# config files
phit_confdir=$(sysconfdir)/phit
dist_phit_conf_DATA = \
        etc/phit/phitd.conf \
        etc/phit/plugins.conf \
        etc/phit/roles.conf \
        etc/phit/ServiceRoot.json \
        etc/phit/OdataService.json

phit_providers_confdir=$(phit_confdir)/providers.d
dist_phit_providers_conf_DATA = etc/phit/providers.d/01-providers-header.conf

# PAM config
pam_confdir=$(sysconfdir)/pam.d
dist_pam_conf_DATA=etc/pam.d/phit etc/pam.d/redfish

# tmpfiles
tmpfiledir=$(libdir)/tmpfiles.d
dist_tmpfile_DATA = etc/tmpfiles.d/phit.conf

# HTML stuff
phit_datadir=$(datadir)/phit
dist_phit_data_DATA = share/phit/background.gif share/phit/postamble.html share/phit/odata.html share/phit/preamble.html

# generate .c files from .metadata files
# NOTE: version encoded in files, unfortunately...
# ServiceRoot.0.96.0.metadata
%.c: %.0.96.0.metadata out/csdl2c
	PATH=$$PATH:$(top_builddir)/out csdl2c $<
	mv $(notdir $@) $@

INSTALL_COMMANDS = mkdir -p $(DESTDIR)/$(phit_confdir)/providers.d

include pkg/Makefile-libbase.inc
include pkg/Makefile-libodata.inc
include pkg/Makefile-libclient.inc

include pkg/Makefile-tools.inc
include pkg/Makefile-json.inc
include pkg/Makefile-phitcli.inc
include pkg/Makefile-olcli.inc
include pkg/Makefile-csdl2c.inc
include pkg/Makefile-testcsdl.inc

include pkg/Makefile-libodataplugin.inc

if BUILD_PHITD
# can't build phitd without the odata plugin due to static linking
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

if BUILD_ODATA_FASTCGI
include pkg/Makefile-fcgiodata.inc
include pkg/Makefile-fcgiauth.inc
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

install-data-hook:
	mkdir -p $(DESTDIR)/$(pkgrundir) ||:
	$(INSTALL_COMMANDS)

pkgconf_list+=odata-provider-sdk
ODATA_PROVIDER_SDK_LIBS =
ODATA_PROVIDER_SDK_CFLAGS=-I$(SDK_DEFAULT_INC_PATH)/odata-provider-sdk $(PKGCONFIG_CPPFLAGS)
ODATA_PROVIDER_SDK_ADD_PKGCONFIG := odata_provider_dir=$(pkglibexecdir)\nodata_provider_conf_dir=$(phit_confdir)
odata_provider_sdkdir=$(sdkdir)/odata-provider-sdk
odata_provider_sdk_HEADERS = \
	src/odata/odatacxx.h	\
	src/odata/odatacxxinlines.h	\
	src/odata/odatamacros.h	\
	src/odata/odata.h	\
	src/phit.h

# most of these don't belong here. They should be in _HEADERS or _SOURCES.
# Remove them from here once we have verified that they are in the appropriate
# _SOURCES or _HEADERS.
EXTRA_DIST += 	\
src/plugins/odata/scope.h	\
src/plugins/odata/post.h	\
src/plugins/odata/webpage.h	\
src/plugins/odata/providers.h	\
src/plugins/odata/options.h	\
src/plugins/odata/odataplugin.h	\
src/winconfig.h	\
src/json2schema/objects.h	\
src/phitcli/options.h	\
src/tests/test.h	\
src/client/client.h	\
src/odata/providerentry.h	\
src/odata/uriparser.h	\
src/odata/jsonobject.h	\
src/odata/name.h	\
src/odata/types.h	\
src/odata/scope.h	\
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

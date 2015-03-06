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

# most providers use same includes, so define it once
provider_includes = src 
bin_includes = src

include pkg/Makefile-libodataplugin.inc
include pkg/Makefile-libodata.inc
include pkg/Makefile-libbase.inc
include pkg/Makefile-phitd.inc
include pkg/Makefile-libechoplugin.inc
include pkg/Makefile-libpingplugin.inc
include pkg/Makefile-libhelloplugin.inc
include pkg/Makefile-tools.inc
include pkg/Makefile-libmetadataprovider.inc
include pkg/Makefile-librootprovider.inc
include pkg/Makefile-libwidgetprovider.inc
include pkg/Makefile-libfrogprovider.inc
include pkg/Makefile-libgadgetprovider.inc
include pkg/Makefile-libheapprovider.inc
include pkg/Makefile-libcheckprovider.inc
include pkg/Makefile-json.inc
include pkg/Makefile-libclient.inc
include pkg/Makefile-phitcli.inc
include pkg/Makefile-olcli.inc
include pkg/Makefile-csdl2c.inc

# vim:noexpandtab:autoindent:tabstop=8:shiftwidth=8:filetype=make:nocindent:tw=0:

###############################################################################
###############################################################################
###
### STOP   STOP   STOP   STOP   STOP   STOP   STOP   STOP   STOP   STOP   STOP
###
### This is a common makefile that is copied between multiple projects and 
### must be the same. Do not make modifications to this makefile unless you
### sync the same change across every copy in the build tree
###
### STOP   STOP   STOP   STOP   STOP   STOP   STOP   STOP   STOP   STOP   STOP
###
###############################################################################
###############################################################################

# set up variables that define the SDK
# make sure that the pkgconfig -I path matches this
sdkdir = $(includedir)/$(PACKAGE_NAME)/
EXTRA_HEADERS =
sdk_HEADERS = $(SONAME_HEADER_LIST)
SDK_DEFAULT_INC_PATH = $${includedir}/$(PACKAGE_NAME)

# define some standard vars that are used in just about every build
# if you need a directory not on this list, add it to your Makefile.am
EXTRA_DIST =
check_PROGRAMS =
check_LTLIBRARIES =
noinst_PROGRAMS =
noinst_LTLIBRARIES =
lib_LTLIBRARIES =
bin_PROGRAMS =
sbin_PROGRAMS =

# standard stuff to clean up
CLEANFILES = $(PACKAGE)-*.{tar.gz,tar.bz2,tar.lzma,rpm} $(PKG_CONFIG_FILES)
DISTCLEANFILES = *~ out $(LIBTOOL) *_soname.h

.PHONY: clean-local distclean-local
clean-local:
	-chmod -R u+w $(CLEANFILES) 2>/dev/null 2>&1
	-test -z "$(CLEANFILES)" || rm -rf $(CLEANFILES)

distclean-local:
	-chmod -R u+w $(DISTCLEANFILES) 2>/dev/null 2>&1
	-test -z "$(DISTCLEANFILES)" || rm -rf $(DISTCLEANFILES)

# compat: older automake (rhel5) doesnt set this
abs_top_builddir ?= $(shell pwd)

# use this function for all versioned shared libraries. It will add appropriate
# versioning to the shared lib.
STD_LDFLAGS_LIB = $(if $($(1)_LIBRARY_VERSION),$($(1)_LIBRARY_VERSION),$(DEFAULT_LIBRARY_VERSION))

# use this function to statically link a single library
STATIC_LINK_FN =   -Wl,-Bstatic -Xlinker -l$1 -Wl,-Bdynamic

# use this function to create a shared library out of a single .a static lib,
# or to link the entire contents of a .a file into a .so file.
STATIC_TO_SHARED = -Wl,-Bstatic -Wl,-whole-archive -Xlinker -l$1 -Wl,-no-whole-archive -Wl,-Bdynamic

# force shared library only
FORCE_SHARED = -avoid-version -rpath $(abs_top_builddir)/out/.libs/ -module

#######
# automatic rootfs population from toplevel rootfs/ dir
define rootfs_install
install-data-local::
	$(mkinstalldirs) $(DESTDIR)/$($(1))
	cd $(top_srcdir)/rootfs/$(1); find . -depth -print | cpio -pamVd $(DESTDIR)/$($(1))/.
	
uninstall-local::
	set -x	;\
	cd $(top_srcdir)/rootfs/$(1); \
	find . -type d -print | while read dr; \
	do \
		dest=$(DESTDIR)/$($(1))/$$$$dr	;\
		echo "chmod: $$$$dest"	;\
		chmod +w $$$$dest ; \
	done;\
	find . -depth -type f -print | while read fn; \
	do \
		dest=$(DESTDIR)/$($(1))/$$$$fn	;\
		echo "Removing: $$$$fn"	;\
		echo "    from: $$$$dest"	;\
		chmod +w $$$$(dirname $$$$dest ); \
		rm -f $$$$dest	;\
	done
endef

# for each toplevel in rootfs/, look up variable by that name and install stuff there
$(foreach rootfs_dir,$(notdir $(wildcard $(top_srcdir)/rootfs/*)),$(eval $(call rootfs_install,$(rootfs_dir))))
$(foreach rootfs_dir,$(notdir $(wildcard $(top_srcdir)/rootfs/*)),$(eval EXTRA_DIST+=rootfs/$(rootfs_dir)))

#######
#
# Usage:  add "pkgconf_list" entries for each pkgconfig file you want to create
# 	pkgconf_list += mypcfile
#
#  Then set these variables as needed:
#	MYPCFILE_LIBS := 
#	MYPCFILE_CFLAGS := 
#	MYPCFILE_ADD_PKGCONFIG := 
#
#  Other reqs:
#    pc_template
#	- these files are the template files used. Should likely be generated in configure.ac by:
#	AC_CONFIG_FILES([pkg/pc_template])
#	- this implies that you must have pkg/pc_template.in and in your source tree
#
###
#  Build package-config files
###
pkgconf_list =
PKG_CONFIG_FILES=$(patsubst %,out/pc/%.pc,$(pkgconf_list))

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(PKG_CONFIG_FILES)

BUILD_PC_FILE = \
	cat $< > $@	;\
	echo -e 'Libs: $${libdirsearchpath} $($(1)_LIBS) $($(1)_LDFLAGS)' >> $@	;\
	echo -e 'Cflags: $($(1)_CFLAGS)' >> $@	;\
	echo -e '$($(1)_ADD_PKGCONFIG)' >> $@

%.pc: pkg/pc_template $(top_builddir)/config.status
	test -e $(dir $@) || mkdir -p $(dir $@)	|| :
	$(call BUILD_PC_FILE,$(shell echo -n $(basename $(notdir $@)) |  tr [:lower:] [:upper:] | tr -c [:alnum:] _))


#############
# End pkgconfig build
#############

# DIRECTORIES:
# the only safe way to pass directory names into the build is via the
# makefiles. In configure.ac, they are not full expanded yet
initdir       = $(sysconfdir)/init.d
pkgsysconfdir = $(sysconfdir)/$(PACKAGE)
pkglibexecdir = $(libexecdir)/$(PACKAGE_NAME)
pkglogdir     = $(localstatedir)/log/$(PACKAGE_NAME)
pkglocalstatedir = $(localstatedir)/lib/$(PACKAGE_NAME)
pkgrundir     = $(localstatedir)/run/$(PACKAGE_NAME)
pkgdatadir    = $(datadir)/$(PACKAGE)

# these are autoconf std variables
FIXUP_VARS ?= PACKAGE_NAME PACKAGE_VERSION RELEASE_MAJOR \
        prefix bindir sbindir libexecdir sysconfdir sharedstatedir      \
        localstatedir libdir includedir oldincludedir datarootdir datadir \
        infodir localedir mandir docdir htmldir dvidir pdfdir psdir	\
	initdir pkgsysconfdir pkglibexecdir pkglogdir \
	pkglocalstatedir pkgrundir pkgdatadir

export $(FIXUP_VARS)

ADD_DEF = -D$(shell echo $(1) | tr [:lower:] [:upper:])="\"$($(1))\""
AM_CPPFLAGS += $(foreach def,$(FIXUP_VARS),$(call ADD_DEF,$(def)))

# a simple function to generate a file from another file, replacing "variables"
# of the form of either %{variable}, or lines that begin with "variable=..."
define REPLACE_VARS
	$(QUIET_GEN)
	mkdir -p $$(dirname $@) 2>/dev/null ||:
	cp $< $@
	for var in $(FIXUP_VARS)	;\
	do	\
	        perl -p -i -e "s|%{$$var}|$${!var}|g"  $@  ;\
	        perl -p -i -e "s|^$$var=.*|$$var=$${!var}|g"  $@  ;\
	done
endef

define REPLACE_RULE
$(1): $(1)$(if $(2),$(2),.in)
	$(value REPLACE_VARS)
endef

# have to use eval construct for CLEANFILES/EXTRA_DIST to get around automake silliness
# use $(if test to only generate REPLACE_RULE if a .in file exists
define REPLACE_TEMPLATE
$(eval EXTRA_DIST += $(shell test ! -e $(top_srcdir)/$(1) || echo $(1)))
$(eval EXTRA_DIST += $(shell test ! -e $(top_srcdir)/$(1)$(if $(2),$(2),.in) || echo $(1)$(if $(2),$(2),.in)))
$(eval CLEANFILES += $(shell test ! -e $(top_srcdir)/$(1)$(if $(2),$(2),.in) || echo $(1)))
$(if $(shell test ! -e $(top_srcdir)/$(1)$(if $(2),$(2),.in) || echo $(1)$(if $(2),$(2),.in)),$(eval $(REPLACE_RULE)))
endef

do_replace = $(foreach file,$($(1)),$(eval $(call REPLACE_TEMPLATE,$(file),$(2))))


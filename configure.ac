#                                               -*- Autoconf -*-
# vim:tw=0:et:ts=4:sw=4
# Process this file with autoconf to produce a configure script.

##############################################################################
#  VERSION VARIABLES
##############################################################################
AC_INIT([odatalite],[0.0.3])

# define library versions
m4_define([LIB_LIST], [DEFAULT])

#anything not listed above will be a cain library and needs to be fixed
VER_DEFAULT="0:0:3"
##############################################################################
# END RELEASE VARIABLES
##############################################################################

AC_PREREQ(2.59)
AC_CONFIG_AUX_DIR([pkg])
AC_CONFIG_MACRO_DIR([pkg/m4])
AC_PREFIX_DEFAULT([/opt/odatalite])

# Generate header files with soname #defines
AX_GENERATE_SONAME_HEADERS([LIB_LIST])
AX_SETUP_VERSION_VARIABLES

AM_INIT_AUTOMAKE([subdir-objects foreign tar-ustar dist-bzip2 no-define])
# enable pretty compile output, only works on automake 1.11, but this line
# makes it not break for older.
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_CONFIG_HEADERS([src/config.h])
AC_CANONICAL_HOST

PKG_PROG_PKG_CONFIG
AC_ARG_WITH([systemdsystemunitdir],
        AS_HELP_STRING([--with-systemdsystemunitdir=DIR], [Directory for systemd service files]),
        [], [with_systemdsystemunitdir=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)])
AC_SUBST([systemdsystemunitdir], [$with_systemdsystemunitdir])
AM_CONDITIONAL(HAVE_SYSTEMD, [test -n "$with_systemdsystemunitdir" ])

AC_ARG_ENABLE(debug,
    AS_HELP_STRING(--enable-debug,Enable debug build. default: disabled),
    [wantdebug=$enableval], [wantdebug=no])
AM_CONDITIONAL(DEBUG, [test "$wantdebug" = "yes"])
if test "$wantdebug" = "yes"; then
    AC_DEFINE_UNQUOTED([ENABLE_DEBUG], [$debug], [debug enabled])
fi

AC_ARG_ENABLE(phitd,
    AS_HELP_STRING(--enable-phitd,Enable phitd build/install. default: disabled),
    [wantphitd=$enableval], [wantphitd=yes])
AM_CONDITIONAL(BUILD_PHITD, [test "$wantphitd" = "yes"])

AC_ARG_ENABLE(hello-plugin,
    AS_HELP_STRING(--enable-hello-plugin,Enable hello-plugin build/install. default: disabled),
    [wanthello=$enableval], [wanthello=no])
AM_CONDITIONAL(BUILD_HELLO_PLUGIN, [test "$wanthello" = "yes"])

AC_ARG_ENABLE(ping-plugin,
    AS_HELP_STRING(--enable-ping-plugin,Enable ping-plugin build/install. default: disabled),
    [wantping=$enableval], [wantping=no])
AM_CONDITIONAL(BUILD_PING_PLUGIN, [test "$wantping" = "yes"])

AC_ARG_ENABLE(echo-plugin,
    AS_HELP_STRING(--enable-echo-plugin,Enable echo-plugin build/install. default: disabled),
    [wantecho=$enableval], [wantecho=no])
AM_CONDITIONAL(BUILD_ECHO_PLUGIN, [test "$wantecho" = "yes"])


AC_ARG_ENABLE(odata-fastcgi,
    AS_HELP_STRING(--enable-odata-fastcgi,Enable odata-fastcgi build/install. default: disabled),
    [wantodatafastcgi=$enableval], [wantodatafastcgi=yes])
AM_CONDITIONAL(BUILD_ODATA_FASTCGI, [test "$wantodatafastcgi" = "yes"])


AC_ARG_ENABLE(metadata-provider,
    AS_HELP_STRING(--enable-metadata-provider,Enable metadata-provider build/install. default: disabled),
    [wantmetadata=$enableval], [wantmetadata=yes])
AM_CONDITIONAL(BUILD_METADATA_PROVIDER, [test "$wantmetadata" = "yes"])

AC_ARG_ENABLE(root-provider,
    AS_HELP_STRING(--enable-root-provider,Enable root-provider build/install. default: disabled),
    [wantroot=$enableval], [wantroot=yes])
AM_CONDITIONAL(BUILD_ROOT_PROVIDER, [test "$wantroot" = "yes"])

AC_ARG_ENABLE(widget-provider,
    AS_HELP_STRING(--enable-widget-provider,Enable widget-provider build/install. default: disabled),
    [wantwidget=$enableval], [wantwidget=no])
AM_CONDITIONAL(BUILD_WIDGET_PROVIDER, [test "$wantwidget" = "yes"])

AC_ARG_ENABLE(frog-provider,
    AS_HELP_STRING(--enable-frog-provider,Enable frog-provider build/install. default: disabled),
    [wantfrog=$enableval], [wantfrog=yes])
AM_CONDITIONAL(BUILD_FROG_PROVIDER, [test "$wantfrog" = "yes"])

AC_ARG_ENABLE(gadget-provider,
    AS_HELP_STRING(--enable-gadget-provider,Enable gadget-provider build/install. default: disabled),
    [wantgadget=$enableval], [wantgadget=no])
AM_CONDITIONAL(BUILD_GADGET_PROVIDER, [test "$wantgadget" = "yes"])

AC_ARG_ENABLE(heap-provider,
    AS_HELP_STRING(--enable-heap-provider,Enable heap-provider build/install. default: disabled),
    [wantheap=$enableval], [wantheap=no])
AM_CONDITIONAL(BUILD_HEAP_PROVIDER, [test "$wantheap" = "yes"])

AC_ARG_ENABLE(check-provider,
    AS_HELP_STRING(--enable-check-provider,Enable check-provider build/install. default: disabled),
    [wantcheck=$enableval], [wantcheck=no])
AM_CONDITIONAL(BUILD_CHECK_PROVIDER, [test "$wantcheck" = "yes"])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# PKG CONFIG STUFF
AC_ARG_VAR([PKG_CONFIG_PATH], [Additional pkg-config paths to search])
AC_ARG_VAR([SYSROOT_PATH], [if we are doing a cross build, the path to the sysroot])
AC_SUBST([SYSROOT_PATH])

# Checks for libraries.

# put pkgconfig tests here
if test "$BREAK_PKG_DEPS" != "1"; then
    PKG_CHECK_MODULES([SYSTEMD], [libsystemd],
    [
AC_DEFINE([HAVE_SYSTEMD], [1], [compile time define if we have systemd])
    ],
[
:
])
    PKG_CHECK_MODULES([PAM], [libpam],,
[
# fallback for native compile where libpam.pc may not exist
AC_CHECK_LIB([pam], [pam_start], [PAM_LIBS="$PAM_LIBS -lpam"],
    AC_MSG_ERROR(
            [Unable to find PAM. Please install the PAM development libraries])
)
AC_SUBST([PAM_LIBS])
])
    # yocto calls it expat.pc
    PKG_CHECK_MODULES([EXPAT], [expat],,
        # RHEL 6 can call it expat21.pc, so look for that as an alternative.
        # note the m4 quote [] around this protects it from yocto autodeps expansion, don't change this
        [PKG_CHECK_MODULES([EXPAT], [expat21])]
        )

if test "$wantodatafastcgi" = "yes"
then
    PKG_CHECK_MODULES([CZMQ], [libczmq])
    # activate dell yocto autodeps
    PKG_CHECK_MODULES([FASTCGI], [libfcgi],,
[ # fallback for native compile where libfcgi.pc probably does not exist
AC_CHECK_LIB([fcgi], [FCGI_Accept], [FASTCGI_LIBS="$FASTCGI_LIBS -lfcgi"],
    AC_MSG_ERROR(
            [Unable to find the fastcgi library. Please install the fastcgi development libraries])
)
AC_SUBST([FASTCGI_LIBS])
])
fi

fi


# the build system and spec files
AC_CONFIG_FILES([Makefile])

AC_CONFIG_FILES([pkg/pc_template])

AC_CHECK_FUNCS([vsnprintf])

# odata defines
AC_DEFINE([RUNSTATEDIR], [PKGRUNDIR], [Where to keep socket and pid files.])
AC_DEFINE([HAVE_POSIX], [], [Compiling on a posix system.])

# generate files and exit
AC_OUTPUT

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
PKG_CHECK_MODULES([SYSTEMD], [systemd])
AC_ARG_WITH([systemdsystemunitdir],
        AS_HELP_STRING([--with-systemdsystemunitdir=DIR], [Directory for systemd service files]),
        [], [with_systemdsystemunitdir=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)])
if test "x$with_systemdsystemunitdir" != xno; then
        AC_SUBST([systemdsystemunitdir], [$with_systemdsystemunitdir])
fi
AM_CONDITIONAL(HAVE_SYSTEMD, [test -n "$with_systemdsystemunitdir" -a "x$with_systemdsystemunitdir" != xno ])

AC_ARG_ENABLE(debug,
    AS_HELP_STRING(--enable-debug,Enable debug build. default: disabled),
    [wantdebug=$enableval], [wantdebug=no])
AM_CONDITIONAL(DEBUG, [test "$wantdebug" = "yes"])
if test "$wantdebug" = "yes"; then
    AC_DEFINE_UNQUOTED([ENABLE_DEBUG], [$debug], [debug enabled])
fi

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

# put pkgconfig tests here
if test "$BREAK_PKG_DEPS" != "1"; then
    PKG_CHECK_MODULES([PAM], [libpam])
    PKG_CHECK_MODULES([EXPAT], [expat])
fi


# the build system and spec files
AC_CONFIG_FILES([Makefile])

AC_CONFIG_FILES([pkg/pc_template])

AC_CHECK_FUNCS([vsnprintf])

# odata defines
AC_DEFINE([RUNSTATEDIR], [PKGRUNDIR], [Where to keep socket and pid files.])

# generate files and exit
AC_OUTPUT

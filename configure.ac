AC_PREREQ(2.52)

AC_INIT([avsystem], [1.0])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_HEADERS([config.h:config.hin])

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_C_CONST
dnl AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_HEADER_STDBOOL
AC_HEADER_STDC
AC_HEADER_TIME
AC_PROG_GCC_TRADITIONAL
AM_PROG_CC_C_O
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_LIBTOOL

AC_ARG_ENABLE(sdk, AC_HELP_STRING([--enable-sdk], [sdk build]),
	[
	 case "${enableval}" in
	  yes) IS_SDK=yes ;;
	  no)  IS_SDK=no ;;
	  *)   AC_MSG_ERROR(bad value ${enableval} for --enable-sdk) ;;
	 esac
	],
	[IS_SDK=no])
AM_CONDITIONAL([IS_SDK], [test "x$IS_SDK" = "xyes"])

AC_ARG_ENABLE(audiotest, AC_HELP_STRING([--enable-audiotest], [build audio test program]),
	[
	 case "${enableval}" in
	  yes) WITH_AUDIOTEST=yes ;;
	  no)  WITH_AUDIOTEST=no ;;
	  *)   AC_MSG_ERROR(bad value ${enableval} for --enable-audiotest) ;;
	 esac
	],
	[WITH_AUDIOTEST=no])
AM_CONDITIONAL([WITH_AUDIOTEST], [test "x$WITH_AUDIOTEST" = "xyes"])

# Checks for libraries.
PKG_CHECK_MODULES(ALSA, alsa >= 1.0.15)
AC_SUBST(ALSA_CFLAGS)
AC_SUBST(ALSA_LIBS)

PKG_CHECK_MODULES(EXIF, libexif)
AC_SUBST(EXIF_CFLAGS)
AC_SUBST(EXIF_LIBS)

PKG_CHECK_MODULES(MMTA, mm-ta)
AC_SUBST(MMTA_CFLAGS)
AC_SUBST(MMTA_LIBS)

PKG_CHECK_MODULES(MMLOG, mm-log)
AC_SUBST(MMLOG_CFLAGS)
AC_SUBST(MMLOG_LIBS)

PKG_CHECK_MODULES(PASIMPLE, libpulse-simple)
AC_SUBST(PASIMPLE_CFLAGS)
AC_SUBST(PASIMPLE_LIBS)
PKG_CHECK_MODULES(PA, libpulse)
AC_SUBST(PA_CFLAGS)
AC_SUBST(PA_LIBS)

PKG_CHECK_MODULES(ASCN, libascenario)
AC_SUBST(ASCN_CFLAGS)
AC_SUBST(ASCN_LIBS)

PKG_CHECK_MODULES(INIPARSER, iniparser)
AC_SUBST(INIPARSER_CFLAGS)
AC_SUBST(INIPARSER_LIBS)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h memory.h stdlib.h string.h sys/time.h unistd.h errno.h sys/types.h sys/stat.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([memset select])

AC_CONFIG_FILES([
Makefile
pkgconfig-arm/Makefile
pkgconfig-arm/avsystem.pc
pkgconfig-arm/avsysaudio.pc
pkgconfig-i386/Makefile
pkgconfig-i386/avsystem.pc
pkgconfig-i386/avsysaudio.pc
init/Makefile
])
AC_OUTPUT

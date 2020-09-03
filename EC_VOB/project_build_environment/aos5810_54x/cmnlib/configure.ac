#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([accton_userlib], [0.1], [bug-report@address])
AM_INIT_AUTOMAKE([-Wall foreign])

AC_CONFIG_FILES([Makefile
                 netopeer/Makefile
	        ])
# Checks for programs.

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

#tool chain definition
#
AM_PROG_AR
AC_PROG_CC([${CROSS_COMPILE}gcc])
AC_SUBST([AR], [${CROSS_COMPILE}ar])
AC_PROG_LIBTOOL
AC_SUBST([LD], [${CROSS_COMPILE}ld])
AC_SUBST([STRIP], [${CROSS_COMPILE}strip])
AC_SUBST([CFLAGS], ['-Wall -Wno-int-to-pointer-cast -fno-strict-aliasing -g -DBROADCOM -D'${D_MODEL_NAME}])
AC_SUBST([LDFLAGS], ['-lrt'])

#Accton Source Directory
AC_ARG_VAR([ACCTONSRC_DIR], [the path of the top accton source directory])
AC_SUBST([ACCTONSRC_DIR], [${ACCSRC}])
AC_ARG_VAR([UBOOTSRC_DIR], [the path of the uboot source directory])
AC_SUBST([UBOOTSRC_DIR], ['$(top_builddir)/../prom/bootloader/uboot'])
AC_ARG_VAR([KERNELSRC_DIR], [the path of the Linux kernel source directory])
AC_ARG_VAR([PROJECT_NAME], [project name for build environment])
AC_SUBST([PROJECT_NAME], [${PROJECT_NAME}])

AC_OUTPUT

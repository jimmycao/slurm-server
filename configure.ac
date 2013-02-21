#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.58])
AC_INIT([slurm-server], [0.0.2], [jimmy.cao@emc.com])
AC_CONFIG_SRCDIR([src/allocate.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE(1.8 foreign -Wall -Werror)
# Checks for programs.
AC_PROG_CC

# Checks for libraries.


# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_CHECK_FUNCS([bzero socket strcasecmp strchr strdup strncasecmp strstr])

AC_CONFIG_FILES([
				Makefile
				src/Makefile
				etc/Makefile
				])
AC_OUTPUT
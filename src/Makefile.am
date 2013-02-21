AM_CPPFLAGS = -I/usr/local/include/slurm
AM_CFLAGS = -g -Wall 
LDADD = -L/usr/local/lib -lslurm -levent 

bin_PROGRAMS = slurm-server
slurm_server_SOURCES = allocate.h allocate.c config.h config.c info.c info.h \
						slurm_server.c socket_server.c socket_server.h  					
						
dist_pkgdata_DATA = help-config-file.txt ip.conf

						
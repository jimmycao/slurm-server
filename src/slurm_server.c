/*
 ============================================================================
 Name        : SlurmEnd.c
 Author      : Jimmy Cao
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include <signal.h>
#include <syslog.h>

#include "socket_server.h"
#include "info.h"
#include "allocate.h"
#include "config.h"

#include <slurm/slurm.h>

//#define IP_CONFIG_FILE "../etc/ip.conf"
#define IP_CONFIG_FILE "ip.conf"

static void sig_term_func(int SIG)
{
	if(SIG == SIGTERM){
		syslog(LOG_INFO, "terminated.");
		closelog();
		exit(0);
		// do some clean work, e.g., close file/socket
	}
}

static int daemon_init(void)
{
	pid_t pid;
	char IP[16];
	uint32_t PORT;

	get_IP_PORT(IP, &PORT, IP_CONFIG_FILE);

	if((pid = fork()) < 0)
		exit(-1);
	else if(pid != 0) //as to parent, fork will return child's pid
		exit (0);   //parent exit
	else if (pid == 0) {   	//child continues
		setsid();      //become session leader
		umask(0);      //clear file mode creation mask
		close(0);  //close stdin, stdout, stderr
		close(1);
		close(2);

		openlog("slurm server daemon:", LOG_PID, LOG_USER);
		syslog(LOG_INFO, "started.");
		signal(SIGTERM, sig_term_func);

		running(IP, PORT);//real work in child process
	}
	return 0;
}

int main (int argc, char *argv[])
{
//	if(daemon_init() == -1){
//		printf("can not fork self");
//		exit(0);
//	}
//=====================
//	char IP[16];
//	uint32_t PORT;
//
//	get_IP_PORT(IP, &PORT, IP_CONFIG_FILE);
//
//	running(IP, PORT);
//============================
	uint32_t jobid = 0;
	allocate_test(&jobid);
	sleep(5);
	if(jobid > 0)
		update_job(jobid);

	return 0;
}


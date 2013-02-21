/*
 * config.c
 *
 *  Created on: Sep 22, 2012
 *      Author: caoj7
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "config.h"

int get_IP_PORT (char *IP, uint32_t *PORT, char *if_config_filename)
{
	char line[128], name[16];
	FILE *fp = NULL;

	if(!(fp = fopen(if_config_filename, "r"))) {
		perror("ip.conf file does not exist!");
		return (1);
	}

	while(fgets(line, 128, fp) != NULL){
		if(strncmp(line, "ip", 2) == 0)
			sscanf(line, "%s %s", name, IP);
		else if(strncmp(line, "port", 4) == 0)
			sscanf(line, "%s %d", name, PORT);
	}

	fclose(fp);
	return 0;
}

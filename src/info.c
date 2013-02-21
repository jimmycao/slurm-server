/*
 * info.c
 *
 *  Created on: Sep 22, 2012
 *      Author: caoj7
 */

#include <stdio.h>
#include <stdlib.h>

#include <slurm/slurm.h>
#include <slurm/slurm_errno.h>

#include "info.h"

int get_total_nodes_slots_no (uint16_t *nodes, uint16_t *slots)
{
	int i;
	node_info_msg_t *node_info_msg_t_ptr = NULL;

	*nodes = 0;
	*slots = 0;
	//get node info
	if(slurm_load_node((time_t)NULL, &node_info_msg_t_ptr, SHOW_ALL)){
		slurm_perror("slurm_load_node error");
		exit(EXIT_FAILURE);
	}

	*nodes = node_info_msg_t_ptr->record_count;

	for(i = 0; i < *nodes; i++){
		(*slots) += node_info_msg_t_ptr->node_array[i].cpus;
//		printf("jimmy: i = %d, arch = %s \n", i, node_info_msg_t_ptr->node_array[i].arch);
//		printf("jimmy: i = %d, cores = %d \n", i, node_info_msg_t_ptr->node_array[i].cores);
//		printf("jimmy: i = %d, cpus = %d \n", i, node_info_msg_t_ptr->node_array[i].cpus);
//		printf("jimmy: i = %d, reason = %s \n", i, node_info_msg_t_ptr->node_array[i].reason);
//		printf("jimmy: i = %d, sockets = %s \n", i, node_info_msg_t_ptr->node_array[i].sockets);
//		printf("jimmy: i = %d, threads = %s \n", i, node_info_msg_t_ptr->node_array[i].threads);

	}
	return 0;
}

int get_free_nodes_slots_no (uint16_t *nodes, uint16_t *slots)
{
	*nodes = 0;
	*slots = 0;
	node_info_msg_t *node_info_msg_t_ptr = NULL;

	//get node info
	if(slurm_load_node((time_t)NULL, &node_info_msg_t_ptr, SHOW_ALL)){
		slurm_perror("slurm_load_node error");
		exit(EXIT_FAILURE);
	}

	int i;
	for(i = 0; i < node_info_msg_t_ptr->record_count; i++){
		if(node_info_msg_t_ptr->node_array[i].node_state == NODE_STATE_IDLE){
			(*nodes) ++;
			(*slots) += node_info_msg_t_ptr->node_array[i].cpus;
		}
	}
	return 0;
}

hostlist_t get_available_host_list_system()
{
	node_info_msg_t *node_info_ptr = NULL;
	hostlist_t hostlist;
	int i;

	if(slurm_load_node((time_t)NULL, &node_info_ptr, SHOW_ALL)){
		slurm_perror("ERROR: slurm_load_node");
		exit(-1);
	}

	hostlist = slurm_hostlist_create(NULL);
	for(i = 0; i < node_info_ptr->record_count;  i++){
		if(node_info_ptr->node_array[i].node_state == NODE_STATE_IDLE){
			 slurm_hostlist_push_host(hostlist, node_info_ptr->node_array[i].name);
		}
	}

	slurm_free_node_info_msg (node_info_ptr);

	return hostlist;
}


char* get_available_host_list_range_sytem()
{
	hostlist_t hostlist = get_available_host_list_system();
	char *range = slurm_hostlist_ranged_string_malloc (hostlist);
	slurm_hostlist_destroy(hostlist);
	return range;
}

hostlist_t choose_available_from_node_list(char *node_list)
{
	hostlist_t given_hl = slurm_hostlist_create (node_list);
	hostlist_t avail_hl_system  = get_available_host_list_system();

	hostlist_t result_hl = slurm_hostlist_create(NULL);
	char *hostname;
	while((hostname = slurm_hostlist_shift(given_hl))){
		if(slurm_hostlist_find (avail_hl_system, hostname) != -1) {
			slurm_hostlist_push_host(result_hl, hostname);
		}
	}

	slurm_hostlist_destroy(given_hl);
	slurm_hostlist_destroy(avail_hl_system);
	return result_hl;
}

char* get_hostlist_subset(char *host_name_list, uint16_t node_num)
{
	hostlist_t hostlist = slurm_hostlist_create(host_name_list);
	int sum = slurm_hostlist_count(hostlist);

	if(sum < node_num){
		slurm_perror ("error: node_num is larger than sum of host in hostlist");
		exit(-1);
	}

	hostlist_t temp_hl = slurm_hostlist_create(NULL);
	int i = 0;
	char *hostname;
	for(i = 0; i < node_num; i++){
		hostname = slurm_hostlist_shift(hostlist);
		slurm_hostlist_push_host(temp_hl, hostname);
	}

	char *range = slurm_hostlist_ranged_string_malloc (temp_hl);

	slurm_hostlist_destroy(temp_hl);
	slurm_hostlist_destroy(hostlist);
	return range;
}

void print_node_info()
{
	int i;
	node_info_msg_t *node_info_ptr = NULL;
	node_info_t *node_ptr;

	/* get and dump some node information */
	if ( slurm_load_node ((time_t) NULL,
						  &node_info_ptr, SHOW_ALL) ) {
		 slurm_perror ("slurm_load_node error");
		 exit (1);
	}

	/* The easy way to print... */
	slurm_print_node_info_msg (stdout, node_info_ptr, 0);

	/* A harder way.. */
	for (i = 0; i < node_info_ptr->record_count; i++) {
		 node_ptr = &node_info_ptr->node_array[i];
		 slurm_print_node_table(stdout, node_ptr, 1, 0);
	}


	/* The hardest way. */
	for (i = 0; i < node_info_ptr->record_count; i++) {
		 printf ("NodeName=%s CPUs=%u\n",
			  node_info_ptr->node_array[i].name,
			  node_info_ptr->node_array[i].cpus);
	}
	slurm_free_node_info_msg (node_info_ptr);
}

void print_pratition_info()
{
	int i, j, k;
	partition_info_msg_t *part_info_ptr = NULL;
	partition_info_t *part_ptr;

	node_info_msg_t *node_info_ptr = NULL;

	if ( slurm_load_node ((time_t) NULL,
							  &node_info_ptr, SHOW_ALL) ) {
		 slurm_perror ("slurm_load_node error");
		 exit (1);
	}

	/* get and dump some partition information */
	/* note that we use the node information loaded */
	/* above and we assume the node table entries have */
	/* not changed since */

	if ( slurm_load_partitions ((time_t) NULL,
								&part_info_ptr, SHOW_ALL) ) {
		 slurm_perror ("slurm_load_partitions error");
		 exit (1);
	}

	for (i = 0; i < part_info_ptr->record_count; i++) {
		 part_ptr = &part_info_ptr->partition_array[i];
		 printf ("PartitionName=%s Nodes=", part_ptr->name);
		 for (j = 0; part_ptr->node_inx; j+=2) {
			  if (part_ptr->node_inx[j] == -1)
				   break;
			  for (k = part_ptr->node_inx[j];
				   k <= part_ptr->node_inx[j+1];
				   k++) {
				   printf ("%s ", node_info_ptr->
						   node_array[k].name);
			  }
		 }
		 printf("\n\n");
	}

	slurm_free_node_info_msg (node_info_ptr);
	slurm_free_partition_info_msg (part_info_ptr);
}

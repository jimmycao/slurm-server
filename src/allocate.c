/*
 * allocate.c
 *
 *  Created on: Sep 22, 2012
 *      Author: caoj7
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#include <slurm/slurm.h>
#include <slurm/slurm_errno.h>

#include "allocate.h"
#include "info.h"

#define SIZE 8192
//#define MANDATORY 	1
//#define OPTIONAL  	0

int socket_fd;
/**
 *	select n nodes from the given node_range_list, get the selected node list
 *	IN:
 *		request_node_num: requested node num
 *		node_range_list: specified node range to select from
 *	OUT Parameter:
 *		final_req_node_list
 *	RET OUT
 *		-1 if requested node number is larger than available node number
 *		0  successful, the final request node list is returned as final_req_node_list
 */
static int get_nodelist_optional(uint16_t request_node_num, char *node_range_list,
								char *final_req_node_list)
{
	hostlist_t avail_hl_system;

	/*get all available hostlist in SLURM system*/
	avail_hl_system = get_available_host_list_system();

	if(request_node_num > slurm_hostlist_count(avail_hl_system))
		return -1;

	hostlist_t avail_hl_pool = choose_available_from_node_list(node_range_list);
	char *avail_node_pool_range = slurm_hostlist_ranged_string_malloc(avail_hl_pool);
	int avail_node_pool_num = slurm_hostlist_count(avail_hl_pool);

	if(request_node_num <= avail_node_pool_num) {
		char *subset = get_hostlist_subset(avail_node_pool_range, request_node_num);
		strcpy(final_req_node_list, subset);
	}else{  //avail_node_pool_num < reqeust_node_num <= avail_node_num_system
		hostlist_t hostlist = slurm_hostlist_create(avail_node_pool_range);
		int extra_needed_num = request_node_num - avail_node_pool_num;

		int i;
		for(i = 0; i < extra_needed_num;){
			char *hostname = slurm_hostlist_shift(avail_hl_system);
			if(slurm_hostlist_find(hostlist, hostname) == -1){
				slurm_hostlist_push_host(hostlist, hostname);
				i++;
			}
		}

		strcpy(final_req_node_list, slurm_hostlist_ranged_string_xmalloc(hostlist));
	}
	return 0;
}


static int get_nodelist_mandatory(uint16_t request_node_num, char *node_range_list,
								char *final_req_node_list, time_t timeout, int hint)
{
	hostlist_t avail_hl = choose_available_from_node_list(node_range_list);
	int avail_node_num = slurm_hostlist_count(avail_hl);

	char *avail_node_range = slurm_hostlist_ranged_string_malloc(avail_hl);

	if(request_node_num <= avail_node_num) {
		char *subset = get_hostlist_subset(avail_node_range, request_node_num);
		strcpy(final_req_node_list, subset);
	}else{
		if(timeout == 0 && hint == 1){
			sleep(10);
		}else{
			sleep(10);
			timeout -= 10;
			hint = 0;
			if(timeout < 0)
				return -1;
		}
		return get_nodelist_mandatory(request_node_num, node_range_list,
								final_req_node_list, timeout, hint);
	}
	return 0;
}


static void pending_callback(uint32_t job_id)
{
	char buf_rt[SIZE];
	sprintf(buf_rt, "job_id = %d, resource allocation is pending, please wait..", job_id);
	if(write(socket_fd, buf_rt, strlen(buf_rt)+1) < 0){
		perror("socket writing failed in pending_callback...");
		exit(-1);
	}
}

int allocate_nodes(uint32_t request_node_num, char *node_range_list,
					uint32_t *jobid, char *reponse_node_list,
					char *flag, time_t timeout, int fd)
{

	socket_fd = fd;
	job_desc_msg_t job_desc_msg;
	resource_allocation_response_msg_t *job_alloc_resp_msg;
	char final_req_node_list[SIZE];

	slurm_init_job_desc_msg (&job_desc_msg);
	job_desc_msg.user_id = getuid();
	job_desc_msg.group_id = getgid();
	job_desc_msg.contiguous = 0;


	if(!strcmp(node_range_list, "")){
		job_desc_msg.min_nodes = request_node_num;
	}else{
		if(!strcasecmp(flag, "mandatory")){
			int rt = get_nodelist_mandatory(request_node_num, node_range_list,
											final_req_node_list, timeout, 1);
			if(rt == -1){
				slurm_perror ("ERROR: timeout!");
				return -1; //the process with socket should not be terminated, so don't exit(-1);
			}
			job_desc_msg.req_nodes = final_req_node_list;
		} else {  //flag == "optional"
			int rt = get_nodelist_optional(request_node_num,
											node_range_list, final_req_node_list);
			if(rt == -1){
				job_desc_msg.min_nodes = request_node_num;
			}
			else{
				job_desc_msg.req_nodes = final_req_node_list;
			}
		}
	}

//	job_alloc_resp_msg = slurm_allocate_resources_blocking(&job_desc_msg,
//											timeout, pending_callback);
	job_alloc_resp_msg = slurm_allocate_resources_blocking(&job_desc_msg,
											timeout, NULL);
	if (!job_alloc_resp_msg) {
		slurm_perror ("ERROR: slurm_allocate_resources_blocking");
		return -1;
	}

	printf ("In Slurm-Server: Allocate node_list = %s to job_id = %u\n",
			job_alloc_resp_msg->node_list, job_alloc_resp_msg->job_id );

	*jobid = job_alloc_resp_msg->job_id;
	strcpy(reponse_node_list, job_alloc_resp_msg->node_list);

	//kill the job
	if (slurm_kill_job(job_alloc_resp_msg->job_id, SIGKILL, 0)) {
		 printf ("errno: kill job %d\n", slurm_get_errno());
		 exit (1);
	}
	printf ("canceled job_id %u\n", job_alloc_resp_msg->job_id );

	//free the allocated resource
	slurm_free_resource_allocation_response_msg(job_alloc_resp_msg);
	return 0;
}


int allocate_node_noblock (uint16_t min_nodes, uint16_t max_nodes,
		                   uint32_t *jobid, char *node_list)
{
	job_desc_msg_t job_desc_msg;
	resource_allocation_response_msg_t *job_alloc_resp_msg;

	slurm_init_job_desc_msg( &job_desc_msg );
//	job_desc_msg.name = ("job01 ");
	job_desc_msg.min_nodes = min_nodes;
	job_desc_msg.max_nodes = max_nodes;
	job_desc_msg.user_id = getuid();
	job_desc_msg.group_id = getgid();
	job_desc_msg.contiguous = 0;
//	job_desc_msg.time_limit = 1;

	if (slurm_allocate_resources(&job_desc_msg,
								 &job_alloc_resp_msg)) {
		 slurm_perror ("slurm_allocate_resources error");
		 exit (1);
	}

	printf ("Allocated nodes %s to job_id %u\n",
				job_alloc_resp_msg->node_list,
				job_alloc_resp_msg->job_id );

	*jobid = job_alloc_resp_msg->job_id;
	strcpy(node_list, job_alloc_resp_msg->node_list);

	//kill the job
	if (slurm_kill_job(job_alloc_resp_msg->job_id, SIGKILL, 0)) {
		 printf ("kill errno: %d\n", slurm_get_errno());
		 exit (1);
	}
	printf ("canceled job_id %u\n",
			job_alloc_resp_msg->job_id );
	//free the allocated resource
	slurm_free_resource_allocation_response_msg(
			  job_alloc_resp_msg);
	return 0;
}

int allocate_node_block(uint16_t min_nodes, uint16_t max_nodes,
		                  uint32_t *jobid, char *node_list)
{
	job_desc_msg_t job_desc_msg;
	resource_allocation_response_msg_t *job_alloc_resp_msg ;

	slurm_init_job_desc_msg( &job_desc_msg );
//	job_desc_msg.name = ("job02 ");
	job_desc_msg.min_nodes = min_nodes;
	job_desc_msg.max_nodes = max_nodes;
	job_desc_msg.user_id = getuid();
	job_desc_msg.group_id = getgid();
	job_desc_msg.contiguous = 0;
	job_desc_msg.time_limit = 1;

	if (!(job_alloc_resp_msg =
			slurm_allocate_resources_blocking(&job_desc_msg, 60, NULL))) {
		slurm_perror ("slurm_allocate_resources_blocking error");
		exit (1);
	}
	printf ("Allocated nodes %s to job_id %u\n",
			job_alloc_resp_msg->node_list,
			job_alloc_resp_msg->job_id );
	*jobid = job_alloc_resp_msg->job_id;
	strcpy(node_list, job_alloc_resp_msg->node_list);

	//kill the job
	if (slurm_kill_job(job_alloc_resp_msg->job_id, SIGKILL, 0)) {
		 printf ("kill %d\n", slurm_get_errno());
		 exit (1);
	}
	printf ("canceled job_id %u\n",
			job_alloc_resp_msg->job_id );
	//free the allocated resource
	slurm_free_resource_allocation_response_msg(
			  job_alloc_resp_msg);
	return 0;
}

int allocate_test(uint32_t *jobid)
{
	job_desc_msg_t job_desc_msg;
	resource_allocation_response_msg_t *job_alloc_resp_msg;

	slurm_init_job_desc_msg (&job_desc_msg);
	job_desc_msg.user_id = getuid();
	job_desc_msg.group_id = getgid();
	job_desc_msg.contiguous = 0;
//==========================================
	/*
	 * allocate according to min_nodes and max_nodes
	 * if the amount of available nodes >= 4, then 4 nodes will be allocated;
	 * else if the amount of available nodes == 3, then 3 nodes will be allocated;
	 * else if the amount of available nodes == 2, then 2 nodes will be allocated;
	 */
//	job_desc_msg.min_nodes = 2;
//	job_desc_msg.max_nodes = 4;
//==========================================
	/*
	 * allocate according to min_cpus and max_cpus
	 * priority: min_cpus first
	 * for a given node, if only one cpu is allocated, then the whole node is marked as allocated.
	 */
	job_desc_msg.min_cpus = 5;
//	job_desc_msg.max_cpus = 9;
//==========================================
	/* CAN NOT allocate resources according to
	 * a)job_desc_msg.num_tasks
	 * b)job_desc_msg.cpus_per_task
	 * c)job_desc_msg.ntasks_per_node
	 */
//		job_desc_msg.num_tasks = 10;
//		job_desc_msg.cpus_per_task = 1;
//		job_desc_msg.ntasks_per_node = 4;
	printf("job_desc_msg.num_tasks = %u, job_desc_msg.cpus_per_task = %u\n",
			job_desc_msg.num_tasks, job_desc_msg.cpus_per_task);
	time_t timeout = 10;

	job_alloc_resp_msg = slurm_allocate_resources_blocking(&job_desc_msg,
											timeout, NULL);
	if (!job_alloc_resp_msg) {
		perror("allocate failure, timeout or request too many nodes");
		return -1;
	}

	printf("job_alloc_resp_msg->alias_list = %s\n", job_alloc_resp_msg->alias_list);
	printf("job_alloc_resp_msg->cpu_count_reps = %u\n",  job_alloc_resp_msg->cpu_count_reps);
	printf("job_alloc_resp_msg->cpus_per_node = %u\n", job_alloc_resp_msg->cpus_per_node);
	printf("job_alloc_resp_msg->job_id = %u\n", job_alloc_resp_msg->job_id);
	*jobid = job_alloc_resp_msg->job_id;
	printf("job_alloc_resp_msg->node_cnt = %u\n", job_alloc_resp_msg->node_cnt);
	printf("job_alloc_resp_msg->node_list = %s\n", job_alloc_resp_msg->node_list);
	printf("job_alloc_resp_msg->num_cpu_groups = %u\n", job_alloc_resp_msg->num_cpu_groups);
	printf("job_alloc_resp_msg->pn_min_memory = %u\n", job_alloc_resp_msg->pn_min_memory);


	/* free the allocated resource msg */
	slurm_free_resource_allocation_response_msg(job_alloc_resp_msg);


	//kill the job, just for test
//	if (slurm_kill_job(job_alloc_resp_msg->job_id, SIGKILL, 0)) {
//		 error ("ERROR: kill job %d\n", slurm_get_errno());
//		 return -1;
//	}

	return 0;
}

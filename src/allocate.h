/*
 * allocate.h
 *
 *  Created on: Sep 22, 2012
 *      Author: caoj7
 */

#ifndef ALLOCATE_H_
#define ALLOCATE_H_

/**
 * request nodes from node_list
 *
 * if (flag == mandatory), all requested nodes must be allocated from node_list;
 * else if (flag == optional), try best to allocate node from node_list, and the
 * allocation should include all nodes in the given list that are currently available.
 * If that isn't enough to meet the node_num_request, then take  any other nodes that
 * are available to fill out the requested number.
 */
extern int allocate_nodes(uint32_t request_node_num, char *node_range_list,
					uint32_t *jobid, char *reponse_node_list,
					char *flag, time_t timeout, int fd);

extern int allocate_node_noblock (uint16_t min_nodes, uint16_t max_nodes,
								uint32_t *jobid, char *node_list);

extern int allocate_node_block (uint16_t min_nodes, uint16_t max_nodes,
        				uint32_t *jobid, char *node_list);

extern int allocate_test(uint32_t *jobid);

extern int update_job(uint32_t jobid);

extern int allocate_jobstep();

#endif /* ALLOCATE_H_ */

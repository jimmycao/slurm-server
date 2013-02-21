/*
 * info.h
 *
 *  Created on: Sep 22, 2012
 *      Author: caoj7
 */

#ifndef INFO_H_
#define INFO_H_

#include <stdint.h>
#include <slurm/slurm.h>

extern int get_total_nodes_slots_no(uint16_t *nodes, uint16_t *slots);
extern int get_free_nodes_slots_no(uint16_t *nodes, uint16_t *slots);

extern hostlist_t get_available_host_list_system();
extern char* get_available_host_list_range_sytem();
extern hostlist_t choose_available_from_node_list(char *request_node_list);
extern char* get_hostlist_subset(char *host_name_list, uint16_t node_num);

extern void print_node_info();
extern void print_pratition_info();

#endif /* INFO_H_ */

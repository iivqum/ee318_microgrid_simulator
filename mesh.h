/*
 * mesh.h
 *
 *  Created on: 23 Apr 2026
 *      Author: Josh
 */

#include <stdbool.h>

#ifndef MESH_H_
#define MESH_H_

typedef enum mesh_point_type {
/*
This is a point within the microgrid representing either
a connection, a load or a generation source
*/
	mesh_point_type_generator,
	mesh_point_type_load,
	mesh_point_type_connection
} mesh_point_type_t;

typedef struct mesh_point {
	// is_closed will be true if this is a connection and its closed
	bool is_boundary, is_closed;
	mesh_point_type_t what;
	float generation_level, impedance, voltage;
	/*
	Each point can be connected to a maximum of 2 nodes.
	Used to index the node list when a solution is found, -1 if unused
	Loads only have a single node. Whatever they're connected to, and ground.
	Switching nodes will always be shared between 2 nodes.
	*/
	int8_t node[2];
} mesh_point_t;

typedef struct mesh_node {
/*
A node is just like a normal circuit node.
*/
	float voltage;
	// If the node is a super node
	// Meaning it has one or more loads connected to it
	bool super;
	uint8_t super_index;
	// List of points that share this node.
	mesh_point_t *points;
} mesh_node_t;

typedef struct mesh {
	mesh_point_t points[24];
} mesh_t;

bool mesh_solve(mesh_t *system);
bool mesh_init(mesh_t *system);

#endif /* MESH_H_ */

/*
 * mesh.h
 *
 *  Created on: 23 Apr 2026
 *      Author: Josh
 */

#include <stdbool.h>

#ifndef MESH_H_
#define MESH_H_

// How many nodes are stored in each super node
#define MESH_SUPER_NODE_BUFFER_DEPTH 9
// How many super nodes can be in the mesh
#define MESH_SUPER_NODE_BUFFER_SIZE 10
// How many points in the mesh
#define MESH_POINT_BUFFER_SIZE 24
// How many nodes in the mesh
#define MESH_NODE_BUFFER_SIZE 9
// How many points in a node
#define MESH_NODE_POINT_BUFFER_SIZE 24

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
	// Voltage across this point
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
	// List of points that share this node.
	uint8_t indices[MESH_NODE_POINT_BUFFER_SIZE];
	uint8_t length;
} mesh_node_t;


typedef struct mesh_node_buffer {
	// Array of node indices
	uint8_t indices[MESH_SUPER_NODE_BUFFER_DEPTH];
	size_t length;
};

typedef struct mesh {
	mesh_point_t points[MESH_POINT_BUFFER_SIZE];
	mesh_node_t nodes[MESH_NODE_BUFFER_SIZE];
	mesh_node_buffer super_nodes[MESH_SUPER_NODE_BUFFER_SIZE];
	uint8_t num_super_nodes, num_nodes;
} mesh_t;

bool mesh_solve(mesh_t *system);
bool mesh_init(mesh_t *system);


#endif /* MESH_H_ */

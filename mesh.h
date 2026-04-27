/*
 * mesh.h
 *
 *  Created on: 23 Apr 2026
 *      Author: Josh
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "fsl_debug_console.h"

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
// How many points are connected to a node
#define MESH_NODE_CONNECTED_POINTS 4

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
} mesh_node_buffer_t;

typedef struct mesh {
	mesh_point_t points[MESH_POINT_BUFFER_SIZE];
	mesh_node_t nodes[MESH_NODE_BUFFER_SIZE];
	mesh_node_buffer_t super_nodes[MESH_SUPER_NODE_BUFFER_SIZE];
	uint8_t num_super_nodes, num_nodes;
} mesh_t;

bool mesh_solve(mesh_t *system);
bool mesh_init(mesh_t *system);
bool mesh_reset_buffers(mesh_t *system);

bool mesh_point_init(mesh_point_t *point);

bool mesh_node_buffer_insert(mesh_node_buffer_t *buf, uint8_t node_idx);
bool mesh_node_point_insert(mesh_node_t *node, uint8_t point_idx);

#endif /* MESH_H_ */

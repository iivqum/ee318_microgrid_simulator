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
#define MESH_NODE_BUFFER_SIZE 16
// How many points in a node
#define MESH_NODE_POINT_BUFFER_SIZE 24
// How many points are connected to a node
#define MESH_NODE_CONNECTED_POINTS 4

typedef struct mesh_node_buffer {
	// Array of node indices
	uint8_t indices[MESH_SUPER_NODE_BUFFER_DEPTH];
	size_t length;
} mesh_node_buffer_t;

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
	// Voltage across this component
	float generation_level, impedance, voltage;
	/*
	Stores which nodes are connected to this point.
	This is used to calculate the voltage across a component
	Will have a maximum of 2 nodes
	Points with a single node must be a load, otherwise the voltage
	across them is zero.
	Points with 2 nodes will have a voltage equal to the difference between
	those nodes.
	*/
	mesh_node_buffer_t nodes;
} mesh_point_t;

typedef struct mesh_node {
/*
A node is just like a normal circuit node.
*/
	// With respect to ground
	float voltage;
	// List of points that share this node.
	uint8_t indices[MESH_NODE_POINT_BUFFER_SIZE];
	uint8_t length;
	// Each point will correspond to a set bit in the mask
	// Easy way to check if nodes are connected is by ANDing these masks
	uint32_t point_mask;
	// Nodes connected to this node
	mesh_node_buffer_t cons;
} mesh_node_t;

typedef struct mesh {
	mesh_point_t points[MESH_POINT_BUFFER_SIZE];
	mesh_node_t nodes[MESH_NODE_BUFFER_SIZE];
	mesh_node_buffer_t super_nodes[MESH_SUPER_NODE_BUFFER_SIZE];
	mesh_node_buffer_t source_nodes;
	uint8_t num_super_nodes, num_nodes;
	float load_balance;
	bool solution_valid;
} mesh_t;

bool mesh_solve(mesh_t *system);
// Open every connection that isn't a load or generator
void mesh_reset_connections(mesh_t *system);
bool mesh_resolve(mesh_t *system);
bool mesh_init(mesh_t *system);
// Make every grid point a connection
void mesh_reset_grid_points(mesh_t *system);
bool mesh_reset_buffers(mesh_t *system);
float mesh_get_load_balance(mesh_t *system);
bool mesh_build_node_graph(mesh_t *system);
mesh_point_t *mesh_get_point(mesh_t *system, uint8_t row, uint8_t col);
mesh_point_t *mesh_get_point_display_mapped(mesh_t *system, uint8_t row, uint8_t col);

bool mesh_point_init(mesh_point_t *point);

bool mesh_node_buffer_insert(mesh_node_buffer_t *buf, uint8_t node_idx);
bool mesh_node_point_insert(mesh_node_t *node, uint8_t point_idx);

#endif /* MESH_H_ */

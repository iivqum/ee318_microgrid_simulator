/*
 * mesh.c
 *
 *  Created on: 23 Apr 2026
 *      Author: Josh
 */


#include "mesh.h"
#include <stddef.h>

/*
The grid looks like this. There are 24 points.
Start indexing at the first element and work down.

	0		x x x
			| | |
	3  	   x•x•x•x
		    | | |
	7       x x x
		    | | |
	10     x•x•x•x
		    | | |
	14      x x x
	        | | |
	17     x•x•x•x
			| | |
	21      x x x

There are 9 nodes which connect each row of 4. These nodes
are connected through to the adjacent rows of 3. Nodes
are shown by the dots.

The node_relation array gives which points are connected to each
node.
*/

uint8_t node_relation[9][4] = {
		{0, 3, 4, 7},
		{1, 4, 5, 8},
		{2, 5, 6, 9},

		{7, 10, 11, 14},
		{8, 11, 12, 15},
		{9, 12, 13, 16},

		{14, 17, 18, 21},
		{15, 18, 19, 22},
		{16, 19, 20, 23},
};

// ANDing nodes together will tell us which points intersect
uint32_t node_relation_bitwise[9] = {
		(1 << 0) | (1 << 3) | (1 << 4) | (1 << 7),
		(1 << 1) | (1 << 4) | (1 << 5) | (1 << 8),
		(1 << 2) | (1 << 5) | (1 << 6) | (1 << 9),

		(1 << 7) | (1 << 10) | (1 << 11) | (1 << 14),
		(1 << 8) | (1 << 11) | (1 << 12) | (1 << 15),
		(1 << 9) | (1 << 12) | (1 << 13) | (1 << 16),

		(1 << 14) | (1 << 17) | (1 << 18) | (1 << 21),
		(1 << 15) | (1 << 18) | (1 << 19) | (1 << 22),
		(1 << 16) | (1 << 19) | (1 << 20) | (1 << 23)
};

uint8_t count_trailing_zeros(uint32_t n) {
	// Find intersection of 2 nodes
	if (n == 0)
		return 32;
	uint8_t zeros = 0;
	while (!(n & 1)) {
		zeros++;
		n >>= 1;
	}
	return zeros;
}

bool mesh_point_init(mesh_point_t *point) {
	point->what = mesh_point_type_connection;
	point->is_closed = true;
	point->generation_level = 0;
	point->impedance = 1;
	point->voltage = 0;

	return true;
}

bool mesh_node_buffer_insert(mesh_node_buffer_t *buf, uint8_t node_idx) {
	if (buf->length < MESH_SUPER_NODE_BUFFER_DEPTH) {
		buf->indices[buf->length] = node_idx;
		buf->length++;

		return true;
	}

	return false;
}

bool mesh_node_point_insert(mesh_node_t *node, uint8_t point_idx) {
	if (node->length < MESH_NODE_POINT_BUFFER_SIZE) {
		node->indices[node->length] = point_idx;
		node->length++;

		return true;
	}

	return false;
}

bool mesh_reset_buffers(mesh_t *system) {
	system->num_super_nodes = 0;
	system->num_nodes = 0;

	for (int node_idx = 0; node_idx < MESH_NODE_BUFFER_SIZE; node_idx++) {
		system->nodes[node_idx].length = 0;
	}

	for (int super_node_idx = 0; super_node_idx < MESH_SUPER_NODE_BUFFER_SIZE; super_node_idx++) {
		system->super_nodes[super_node_idx].length = 0;
	}

	return true;
}

bool mesh_init(mesh_t *system) {
/*
First, a collection of every possible node in the circuit is created
assuming every point is a switch. Every point is a component in the
circuit.

Then this graph is simplified by the solver, considering the loads.
The simplified graph will merge nodes which are connected by loads.
*/

	// Make every point a connection by default
	for (int i = 0; i < 24; i++) {
		mesh_point_init(&system->points[i]);
	}

	mesh_reset_buffers(system);

	return true;
}

bool mesh_solve(mesh_t *system) {
/*
 	We are trying to solve x = A^-1 * Z

 	N = number of circuit nodes
	M = number of circuit sources

	A =  G B
		 C D

	A is a (M + N) * (M + N) matrix.

	G is a N * N matrix. Diagonals are the components
	connected to each node i,j. The off-diagonals is the
	negative sum of the impedances connected from node i to j.

	B is an N * M matrix. It contains entries that define where
	a voltage source is connected. Only -1, 1 and 0 entries are
	allowed. B[j][k] decides if voltage source k is connected to node j

	C is the transpose of B if no dependent sources are present.

	D is an M * M matrix. This is always zero is there are
	no dependent sources.

	Z is an (M + N) * 1 matrix.

	Z = i
		e

	i in this case is a zero matrix is no current sources
	are present. The entry e is an M * 1 matrix that encodes
	voltage source values.

	When A is found, take the inverse and solve for x.

	x is an (M + N) * 1 matrix. Top N elements are the node voltages.
	Bottom M elements are the currents through the M independent sources.
*/
	mesh_reset_buffers(system);

	// Generate a simplified network by considering loads
	mesh_point_t *point;

	for (int node_idx = 0; node_idx < MESH_NODE_BUFFER_SIZE; node_idx++) {
		// Each node has 4 points
		bool load = false;
		for (int point_idx = 0; point_idx < MESH_NODE_CONNECTED_POINTS; point_idx++) {
			point = &system->points[node_relation[node_idx][point_idx]];

			if (point->what == mesh_point_type_load) {
				load = true;
				break;
			}
		}

		if (load) {
			// If there is a load at this node, check if any of its load points
			// intersect with any existing supernodes. If it does, then we merge
			// This node with the intersecting supernode.
			mesh_node_buffer_t *super;
			// Each node can only intersect with another node by a single point
			// Because of the grid layout
			uint32_t intersection = 0;
			bool merge = false;
			for (int super_node_idx = 0; super_node_idx < system->num_super_nodes; super_node_idx++) {
				super = &system->super_nodes[super_node_idx];

				for (int merger_index = 0; merger_index < super->length; merger_index++) {
					uint8_t node = super->indices[merger_index];
					intersection = node_relation_bitwise[node_idx] & node_relation_bitwise[node];
					// Make sure intersection does not equal 0
					if (intersection) {
						if (system->points[count_trailing_zeros(intersection)].what == mesh_point_type_load) {
							mesh_node_buffer_insert(super, node_idx);
							merge = true;
							break;
						}
					}
				}

				if (merge)
					break;
			}
			if (!merge)
				// If no merge happened, create a new supernode
				mesh_node_buffer_insert(&system->super_nodes[system->num_super_nodes++], node_idx);
		} else {
			// If there are no loads, simply add the node to the list with all its points
			mesh_node_point_insert(&system->nodes[system->num_nodes], node_relation[node_idx][0]);
			mesh_node_point_insert(&system->nodes[system->num_nodes], node_relation[node_idx][1]);
			mesh_node_point_insert(&system->nodes[system->num_nodes], node_relation[node_idx][2]);
			mesh_node_point_insert(&system->nodes[system->num_nodes++], node_relation[node_idx][3]);
		}
	}

	// Now solve the system after its been simplified

	return true;
}

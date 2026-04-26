/*
 * mesh.c
 *
 *  Created on: 23 Apr 2026
 *      Author: Josh
 */


#include "mesh.h"

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
	system->points[i]->what = mesh_point_type_connection;
	system->points[i]->is_closed = true;
}
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

// Generate a list of simplified nodes



}

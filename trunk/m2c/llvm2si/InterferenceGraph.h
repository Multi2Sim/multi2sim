/*
 * InterferenceGraph.h
 *
 *  Created on: Nov 15, 2013
 *      Author: nathan
 */

#ifndef M2C_LLVM2SI_INTERFERENCE_GRAPH_H
#define M2C_LLVM2SI_INTERFERENCE_GRAPH_H

#include <lib/cpp/Bitmap.h>

namespace llvm2si
{

class InterferenceGraph {

	int size;
	misc::Bitmap graph;

public:

	/* Constructor */
	InterferenceGraph(int size);

	/* Return the value of the interference graph at position given. */
	int Get(int element1, int element2);
	int GetSize() { return size; }

	/* Set the value of the interference graph at the position */
	void Set(int element1, int element2, bool value);

};

}

#endif /* INTERFERENCEGRAPH_H_ */

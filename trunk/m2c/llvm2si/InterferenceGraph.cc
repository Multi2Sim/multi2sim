#include "InterferenceGraph.h"

#include <cassert>
#include <lib/cpp/Bitmap.h>

namespace llvm2si
{

InterferenceGraph::InterferenceGraph(int size) : graph(size*(size-1)/2)
{
	/* Initialize */

	assert (size > 0);
//				panic("%s: invalid value for size (%d)",
//						__FUNCTION__, size);

	this->size = size;
	//this->graph = new Bitmap(size*(size-1)/2);
}

int InterferenceGraph::Get(int element1, int element2) {
	/* Check for valid sizes */
	assert (element1 >= 0 && element1 < size);
//		panic("%s: invalid value for 'element1' (%d)",
//				__FUNCTION__, element1);

	assert (element2 >= 0 && element2 < size);
//			panic("%s: invalid value for 'element2' (%d)",
//					__FUNCTION__, element2);

	if (element1 == element2)
		return 1;

	/* Swap if element2 is bigger than element1 */
	if (element1 > element2)
	{
		int temp = element1;
		element1 = element2;
		element2 = temp;
	}

	int width = size;

	return (int)graph[element1*width-(element1*element1+3*element1)/2+element2-1];
}

void InterferenceGraph::Set(int element1, int element2, bool value) {
	/* Check for valid sizes */
	assert (element1 >= 0 && element1 < size);
//		panic("%s: invalid value for 'element1' (%d)",
//				__FUNCTION__, element1);

	assert (element2 >= 0 && element2 < size);
//			panic("%s: invalid value for 'element2' (%d)",
//					__FUNCTION__, element2);

	if (element1 == element2)
		return;

	/* Swap if element2 is bigger than element1 */
	if (element1 > element2)
	{
		int temp = element1;
		element1 = element2;
		element2 = temp;
	}

	graph.Set(element1*size-(element1*element1+3*element1)/2+element2-1, value);
}

}


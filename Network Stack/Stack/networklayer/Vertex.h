#pragma once

#include <stdint.h>
#include "Weight.h"

typedef int VertexIndex;


struct Vertex {
	VertexIndex target;
	Weight weight;
	Vertex(VertexIndex arg_target, Weight arg_weight)
		: target(arg_target), weight(arg_weight) { };
};


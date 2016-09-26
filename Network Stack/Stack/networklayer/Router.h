#pragma once

#include "Dijkstra.h"

#include "Vertex.h"
#include "../datatypes/Address.h"
#include <vector>
#include <algorithm>

using namespace std;

#include <vector>
#include "../datatypes/Address.h"

#include "DijkstraGraph.h"


using namespace std;

class Router {

private:

	struct findVertexIndex : std::unary_function<Vertex, bool> {
		VertexIndex id;

		findVertexIndex(VertexIndex i) :id(i) { }
		bool operator()(Vertex const& v) const {
			return v.target == id;
		}
	};

	vector<Address> vertexaddressmap;
	DijkstraGraph   graph;

	vector<Address> convertVertexIndexVectorToAddressVector(vector<VertexIndex> & vect);

protected:

public:

	Router();
	~Router();
	vector<Address> getRoute(Address source, Address destination);
	Address         getNextAddressAlongRoute(Address source, Address destination);
	bool            addNeighbor(Address parent, Address neighbor);
	DijkstraGraph   getGraph(void);
	vector<Address> getAddressMap(void);
};


#include "Router.h"

Router::Router() {

}

Router::~Router() {

}

/**
* Determine the route from one address to another.
*/
vector<Address> Router::getRoute(Address source, Address destination) {

	// Get the index from the mapping.
	VertexIndex sourceindex = (find(vertexaddressmap.begin(), vertexaddressmap.end(), source) - vertexaddressmap.begin());
	VertexIndex destindex = (find(vertexaddressmap.begin(), vertexaddressmap.end(), destination) - vertexaddressmap.begin());

	// Check if the parent node is already in the vector.
	if (sourceindex == vertexaddressmap.size() || destindex == vertexaddressmap.size()) {
		return vector<Address>();
	}

	std::vector<Weight>      min_distance;
	std::vector<VertexIndex> previous;

	// Compute the (shortest) path.
	DijkstraComputePaths(sourceindex, graph, min_distance, previous);
	vector<VertexIndex> path = DijkstraGetShortestPathTo(destindex, previous);

	// Convert the list of Vertex Indices to a list of Addresses.
	return convertVertexIndexVectorToAddressVector(path);
}

/**
* Convert a list of Vertex Indexes to a list of Addresses.
*/
vector<Address> Router::convertVertexIndexVectorToAddressVector(vector<VertexIndex> & vect) {

	vector<Address> result;

	for (VertexIndex i : vect)
		result.push_back(vertexaddressmap[i]);

	return result;
}

/**
* Get the first element of the route to another address.
*/
Address Router::getNextAddressAlongRoute(Address source, Address destination) {

	vector<Address> route = getRoute(source, destination);
	if (route.size() < 1)
		return BROADCAST_ADDRESS;
	return route[1];
}

/**
* Add a node to the routing list.
*/
bool Router::addNeighbor(Address parent, Address neighbor) {
	bool isnew = false;

	// We cannot add ourself to the vertex list, because our distance is undefined.
	if (parent == neighbor)
		return false;

	// First check if both the parent and neighbor exist, create them otherwise.

	// Get the index from the mapping.
	VertexIndex parentindex = (find(vertexaddressmap.begin(), vertexaddressmap.end(), parent) - vertexaddressmap.begin());
	// Check if the parent node is already in the vector.
	if (parentindex == vertexaddressmap.size()) {
		// No, insert the parent.
		vertexaddressmap.push_back(parent);
		graph.push_back(vector<Vertex>());
	}

	// Get the neighbor index from the mapping.
	VertexIndex neighborindex = (find(vertexaddressmap.begin(), vertexaddressmap.end(), neighbor) - vertexaddressmap.begin());
	// Check if the parent node is already in the vector.
	if (neighborindex == vertexaddressmap.size()) {
		// No, insert the neighbor.
		vertexaddressmap.push_back(neighbor);
		graph.push_back(vector<Vertex>());

		isnew = true;
	}

	// Link the parent to the neighbor and vice versa.

	// Check if the neighbor is already registered for the parent node.
	if (find_if(graph._Myfirst[parentindex].begin(), graph._Myfirst[parentindex].end(), findVertexIndex(neighborindex)) == graph._Myfirst[parentindex].end())  {
		// No, insert it.
		graph[parentindex].push_back(Vertex(neighborindex, 1));
	}

	// Check if the parent is already registered for the neighbor node.
	if (find_if(graph._Myfirst[neighborindex].begin(), graph._Myfirst[neighborindex].end(), findVertexIndex(parentindex)) == graph._Myfirst[neighborindex].end()) {
		// No, insert it.
		graph._Myfirst[neighborindex].push_back(Vertex(parentindex, 1));
	}

	return isnew;
}

DijkstraGraph Router::getGraph() {

	return graph;
}

vector<Address> Router::getAddressMap() {
	return vertexaddressmap;
}

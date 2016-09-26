/*
* Based on earlier publications by an anonymous author:
* http://rosettacode.org/wiki/Dijkstra%27s_algorithm
*/
#include <iostream>
#include <vector>
#include <string>
#include <list>

#include <limits> // for numeric_limits

#include <set>
#include <utility> // for pair
#include <algorithm>
#include <iterator>

#include "DijkstraGraph.h"
#include "Vertex.h"

//const Weight max_weight = std::numeric_limits<double>::infinity();

/**
* Compute the shortest distance path using a NeighborGraph.
*/
static void DijkstraComputePaths(VertexIndex source,
	const DijkstraGraph &adjacency_list,
	std::vector<Weight> &min_distance,
	std::vector<VertexIndex> &previous)
{
	int n = adjacency_list.size();
	min_distance.clear();
	min_distance.resize(n, std::numeric_limits<double>::infinity());
	min_distance[source] = 0;
	previous.clear();
	previous.resize(n, -1);
	std::set<std::pair<Weight, VertexIndex> > vertex_queue;
	vertex_queue.insert(std::make_pair(min_distance[source], source));

	while (!vertex_queue.empty())
	{
		Weight dist = vertex_queue.begin()->first;
		VertexIndex u = vertex_queue.begin()->second;
		vertex_queue.erase(vertex_queue.begin());

		// Visit each edge exiting u
		const std::vector<Vertex> &neighbors = adjacency_list[u];
		for (std::vector<Vertex>::const_iterator neighbor_iter = neighbors.begin();
			neighbor_iter != neighbors.end();
			neighbor_iter++)
		{
			VertexIndex v = neighbor_iter->target;
			Weight weight = neighbor_iter->weight;
			Weight distance_through_u = dist + weight;
			if (distance_through_u < min_distance[v]) {
				vertex_queue.erase(std::make_pair(min_distance[v], v));

				min_distance[v] = distance_through_u;
				previous[v] = u;
				vertex_queue.insert(std::make_pair(min_distance[v], v));

			}

		}
	}
}

/**
* Get the shortest path by crawling the precalculated graph.
*/
static vector<VertexIndex> DijkstraGetShortestPathTo(
	VertexIndex vertex, const std::vector<VertexIndex> &previous)
{
	std::vector<VertexIndex> path;
	while (vertex != -1) {
		path.insert(path.begin(), vertex);
		vertex = previous[vertex];
	}

	return path;
}
#ifndef APOLLOTA_TRIANGULATION_QUERIES_H_
#define APOLLOTA_TRIANGULATION_QUERIES_H_

#include "triangulation.h"

namespace apollota
{

class TriangulationQueries
{
public:
	typedef std::tr1::unordered_map<std::size_t, std::set<std::size_t> > IDsMap;
	typedef std::vector< std::vector<std::size_t> > IDsGraph;
	typedef std::tr1::unordered_map<Pair, std::set<std::size_t>, Pair::HashFunctor> PairsMap;

	static IDsMap collect_neighbors_map_from_quadruples_map(const Triangulation::QuadruplesMap& quadruples_map)
	{
		IDsMap neighbors_map;
		for(Triangulation::QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			const Quadruple& quadruple=it->first;
			for(int a=0;a<4;a++)
			{
				for(int b=a+1;b<4;b++)
				{
					neighbors_map[quadruple.get(a)].insert(quadruple.get(b));
					neighbors_map[quadruple.get(b)].insert(quadruple.get(a));
				}
			}
		}
		return neighbors_map;
	}

	static IDsGraph collect_ids_graph_from_ids_map(const IDsMap& neighbors_map, const std::size_t number_of_vertices)
	{
		IDsGraph neighbors_graph(number_of_vertices);
		for(IDsMap::const_iterator it=neighbors_map.begin();it!=neighbors_map.end();++it)
		{
			if((it->first)<neighbors_graph.size())
			{
				neighbors_graph[it->first].insert(neighbors_graph[it->first].end(), it->second.begin(), it->second.end());
			}
		}
		return neighbors_graph;
	}

	static IDsMap collect_vertices_map_from_vertices_vector(const Triangulation::VerticesVector& vertices_vector)
	{
		IDsMap ids_vertices_map;
		for(std::size_t i=0;i<vertices_vector.size();i++)
		{
			const Quadruple& quadruple=vertices_vector[i].first;
			for(int a=0;a<4;a++)
			{
				ids_vertices_map[quadruple.get(a)].insert(i);
			}
		}
		return ids_vertices_map;
	}

	static PairsMap collect_pairs_vertices_map_from_vertices_vector(const Triangulation::VerticesVector& vertices_vector)
	{
		PairsMap pairs_vertices_map;
		for(std::size_t i=0;i<vertices_vector.size();i++)
		{
			const Quadruple& quadruple=vertices_vector[i].first;
			for(int a=0;a<4;a++)
			{
				for(int b=a+1;b<4;b++)
				{
					pairs_vertices_map[Pair(quadruple.get(a), quadruple.get(b))].insert(i);
				}
			}
		}
		return pairs_vertices_map;
	}
};

}

#endif /* APOLLOTA_TRIANGULATION_QUERIES_H_ */

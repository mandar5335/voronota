#ifndef APOLLOTA_CONTACT_REMAINDER_H_
#define APOLLOTA_CONTACT_REMAINDER_H_

#include "triangulation.h"
#include "subdivided_icosahedron.h"
#include "rotation.h"

namespace apollota
{

class ContactRemainder
{
public:
	typedef std::tr1::unordered_map<std::size_t, std::set<std::size_t> > IDsMap;

	struct TriangleRecord
	{
		SimplePoint p[3];

		TriangleRecord(const SimplePoint& a, const SimplePoint& b, const SimplePoint& c)
		{
			p[0]=a;
			p[1]=b;
			p[2]=c;
		}
	};

	typedef std::list<TriangleRecord> Remainder;

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

	static Remainder construct_contact_remainder(
			const std::vector<SimpleSphere>& spheres,
			const Triangulation::VerticesVector& vertices_vector,
			const std::set<std::size_t>& vertices_ids,
			const std::size_t a_id,
			const double probe,
			const SubdividedIcosahedron& raw_sih)
	{
		Remainder result;
		if(a_id<spheres.size() && check_remainder_possible(vertices_vector, vertices_ids, probe))
		{
			const SimpleSphere& a=spheres[a_id];
			const std::multimap<double, std::size_t> neighbor_ids=collect_neighbors_with_distances(a, spheres, collect_neighbors_from_vertices(a_id, vertices_vector, vertices_ids));
			if(!neighbor_ids.empty())
			{
				const SimpleSphere a_expanded=SimpleSphere(a, a.r+probe);
				result=init_remainder(a_expanded, raw_sih);
				for(std::multimap<double, std::size_t>::const_iterator it=neighbor_ids.begin();it!=neighbor_ids.end();++it)
				{
					const std::size_t c_id=it->second;
					if(c_id<spheres.size())
					{
						const SimpleSphere& c=spheres[c_id];
						const SimpleSphere c_expanded=SimpleSphere(c, c.r+probe);
						cut_remainder(c_expanded, result);
					}
				}
			}
		}
		return result;
	}

private:
	static bool check_remainder_possible(
			const Triangulation::VerticesVector& vertices_vector,
			const std::set<std::size_t>& vertices_ids,
			const double probe)
	{
		for(std::set<std::size_t>::const_iterator it=vertices_ids.begin();it!=vertices_ids.end();++it)
		{
			const std::size_t vertex_id=(*it);
			if(vertex_id<vertices_vector.size() && vertices_vector[vertex_id].second.r>probe)
			{
				return true;
			}
		}
		return false;
	}

	static std::set<std::size_t> collect_neighbors_from_vertices(
			const std::size_t a_id,
			const Triangulation::VerticesVector& vertices_vector,
			const std::set<std::size_t>& vertices_ids)
	{
		std::set<std::size_t> neighbors_ids;
		for(std::set<std::size_t>::const_iterator it=vertices_ids.begin();it!=vertices_ids.end();++it)
		{
			const std::size_t vertex_id=(*it);
			if(vertex_id<vertices_vector.size())
			{
				const Quadruple& quadruple=vertices_vector[vertex_id].first;
				for(int i=0;i<4;i++)
				{
					const std::size_t neighbor_id=quadruple.get(i);
					if(neighbor_id!=a_id)
					{
						neighbors_ids.insert(neighbor_id);
					}
				}
			}
		}
		return neighbors_ids;
	}

	static std::multimap<double, std::size_t> collect_neighbors_with_distances(
			const SimpleSphere& a,
			const std::vector<SimpleSphere>& spheres,
			const std::set<std::size_t>& neighbor_ids)
	{
		std::multimap<double, std::size_t> result;
		for(std::set<std::size_t>::const_iterator it=neighbor_ids.begin();it!=neighbor_ids.end();++it)
		{
			const std::size_t c_id=(*it);
			if(c_id<spheres.size())
			{
				const SimpleSphere& c=spheres[c_id];
				result.insert(std::make_pair(minimal_distance_from_sphere_to_sphere(a, c), c_id));
			}
		}
		return result;
	}

	static Remainder init_remainder(
			const SimpleSphere& sphere,
			const SubdividedIcosahedron& raw_sih)
	{
		Remainder result;
		SubdividedIcosahedron sih=raw_sih;
		sih.fit_into_sphere(sphere, sphere.r);
		for(std::size_t i=0;i<sih.triples().size();i++)
		{
			const Triple& t=sih.triples()[i];
			result.push_back(TriangleRecord(sih.vertices()[t.get(0)], sih.vertices()[t.get(1)], sih.vertices()[t.get(2)]));
		}
		return result;
	}

	static void cut_remainder(const SimpleSphere& sphere, Remainder& remainder)
	{
		int marks[3]={0, 0, 0};
		Remainder::iterator it=remainder.begin();
		while(it!=remainder.end())
		{
			for(int i=0;i<3;i++)
			{
				marks[i]=distance_from_point_to_point(it->p[i], sphere)<sphere.r ? 1 : 0;
			}
			int marks_sum=marks[0]+marks[1]+marks[2];
			if(marks_sum==3)
			{
				it=remainder.erase(it);
			}
			else if(marks_sum==2)
			{
				int s0=(marks[0]==0 ? 0 : (marks[1]==0 ? 1 : 2));
				int s1=(s0==0 ? 1 : 0);
				int s2=(s0==2 ? 1 : 2);
				SimplePoint c01;
				SimplePoint c02;
				if(intersect_vector_with_sphere(sphere, it->p[s0], it->p[s1], c01) && intersect_vector_with_sphere(sphere, it->p[s0], it->p[s2], c02))
				{
					remainder.insert(it, TriangleRecord(it->p[s0], c01, c02));
					it=remainder.erase(it);
				}
			}
			else if(marks_sum==1)
			{
				int s0=(marks[0]==1 ? 0 : (marks[1]==1 ? 1 : 2));
				int s1=(s0==0 ? 1 : 0);
				int s2=(s0==2 ? 1 : 2);
				SimplePoint c01;
				SimplePoint c02;
				if(intersect_vector_with_sphere(sphere, it->p[s0], it->p[s1], c01) && intersect_vector_with_sphere(sphere, it->p[s0], it->p[s2], c02))
				{
					remainder.insert(it, TriangleRecord(it->p[s1], c01, c02));
					remainder.insert(it, TriangleRecord(it->p[s1], c02, it->p[s2]));
					it=remainder.erase(it);
				}
			}
			else
			{
				++it;
			}
		}
	}

	static bool intersect_vector_with_sphere(const SimpleSphere& sphere, const SimplePoint& a, const SimplePoint& b, SimplePoint& c)
	{
		if(sphere.r<=0.0)
		{
			return false;
		}
		const double d_oa=distance_from_point_to_point(a, sphere);
		const double d_ob=distance_from_point_to_point(b, sphere);
		if((d_oa<sphere.r && d_ob<sphere.r) || (d_oa>sphere.r && d_ob>sphere.r))
		{
			return false;
		}
		else if(d_oa>sphere.r && d_ob<sphere.r)
		{
			return intersect_vector_with_sphere(sphere, b, a, c);
		}
		else
		{
			const double angle_oac=min_angle(a, sphere, b);
			if(equal(angle_oac, 0.0) || equal(angle_oac, Rotation::pi()))
			{
				c=SimplePoint(sphere)+((b-SimplePoint(sphere)).unit()*d_ob);
			}
			else
			{
				const double k=sin(angle_oac)/sphere.r;
				double sin_aio=k*d_oa;
				if(sin_aio<-1.0)
				{
					sin_aio=-1.0;
				}
				else if(sin_aio>1.0)
				{
					sin_aio=1.0;
				}
				const double angle_aco=asin(sin_aio);
				const double angle_aoc=Rotation::pi()-(angle_oac+angle_aco);
				const double d_ai=sin(angle_aoc)/k;
				c=a+((b-a).unit()*d_ai);
			}
			return true;
		}
	}
};

}

#endif /* APOLLOTA_CONTACT_REMAINDER_H_ */

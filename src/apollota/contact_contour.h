#ifndef APOLLOTA_CONTACT_CONTOUR_H_
#define APOLLOTA_CONTACT_CONTOUR_H_

#include <vector>
#include <list>

#include "triangulation.h"
#include "hyperboloid_between_two_spheres.h"
#include "rotation.h"

namespace apollota
{

class ContactContour
{
public:
	typedef std::tr1::unordered_map<Pair, std::set<std::size_t>, Pair::HashFunctor> PairsIDsMap;

	struct PointRecord
	{
		SimplePoint p;
		std::size_t left_id;
		std::size_t right_id;

		PointRecord(const SimplePoint& p, const std::size_t left_id, const std::size_t right_id) : p(p), left_id(left_id), right_id(right_id)
		{
		}
	};

	typedef std::list<PointRecord> Contour;

	static PairsIDsMap collect_pairs_vertices_map_from_vertices_vector(const Triangulation::VerticesVector& vertices_vector)
	{
		PairsIDsMap pairs_vertices_map;
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

	static std::list<Contour> construct_contact_contours(
			const std::vector<SimpleSphere>& spheres,
			const Triangulation::VerticesVector& vertices_vector,
			const std::set<std::size_t>& vertices_ids,
			const std::size_t a_id,
			const std::size_t b_id,
			const double probe,
			const double step,
			const int projections)
	{
		std::list<Contour> result;
		if(a_id<spheres.size() && b_id<spheres.size())
		{
			const SimpleSphere& a=spheres[a_id];
			const SimpleSphere& b=spheres[b_id];
			const std::multimap<double, std::size_t> neighbor_ids=collect_pair_neighbors_with_distances(a, spheres, collect_pair_neighbors_from_pair_vertices(a_id, b_id, vertices_vector, vertices_ids));
			const Contour initial_contour=construct_circular_contour(a, b, a_id, vertices_vector, vertices_ids, probe, step);
			if(!initial_contour.empty())
			{
				result.push_back(initial_contour);
				for(std::multimap<double, std::size_t>::const_iterator it=neighbor_ids.begin();it!=neighbor_ids.end();++it)
				{
					const std::size_t c_id=it->second;
					if(c_id<spheres.size())
					{
						const SimpleSphere& c=spheres[c_id];
						std::list<Contour>::iterator jt=result.begin();
						while(jt!=result.end())
						{
							Contour& contour=(*jt);
							std::list<Contour> segments;
							if(cut_and_split_contour(a, c, c_id, contour, segments))
							{
								if(!contour.empty())
								{
									mend_contour(a, b, c, c_id, step, projections, contour);
									++jt;
								}
								else
								{
									if(!segments.empty())
									{
										for(std::list<Contour>::iterator st=segments.begin();st!=segments.end();++st)
										{
											mend_contour(a, b, c, c_id, step, projections, (*st));
										}
										filter_contours_intersecting_sphere(construct_bounding_sphere_of_vertices_centers(vertices_vector, vertices_ids, step), segments);
										if(!segments.empty())
										{
											result.splice(jt, segments);
										}
									}
									jt=result.erase(jt);
								}
							}
							else
							{
								++jt;
							}
						}
					}
				}
			}
		}
		return result;
	}

	static std::vector< std::vector<SimplePoint> > collect_points_from_contours(const std::list<Contour>& contours)
	{
		std::vector< std::vector<SimplePoint> > result;
		result.reserve(contours.size());
		for(std::list<Contour>::const_iterator it=contours.begin();it!=contours.end();++it)
		{
			const Contour& contour=(*it);
			if(!contour.empty())
			{
				const std::size_t contour_size=contour.size();
				result.push_back(std::vector<SimplePoint>());
				result.back().reserve(contour_size);
				for(Contour::const_iterator jt=contour.begin();jt!=contour.end();++jt)
				{
					result.back().push_back(jt->p);
				}
			}
		}
		return result;
	}

	static std::vector< std::vector<SimplePoint> > collect_meshes_from_contours(
			const std::list<Contour>& contours,
			const SimpleSphere& a,
			const SimpleSphere& b)
	{
		std::vector< std::vector<SimplePoint> > result;
		result.reserve(contours.size());
		for(std::list<Contour>::const_iterator it=contours.begin();it!=contours.end();++it)
		{
			const Contour& contour=(*it);
			if(!contour.empty())
			{
				const std::size_t contour_size=contour.size();

				result.push_back(std::vector<SimplePoint>());
				result.back().reserve(contour_size+1);

				{
					SimplePoint mc(0.0, 0.0, 0.0);
					for(Contour::const_iterator jt=contour.begin();jt!=contour.end();++jt)
					{
						mc=mc+(jt->p);
					}
					mc=mc*(1.0/static_cast<double>(contour_size));
					mc=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(mc, a, b);
					result.back().push_back(mc);
				}

				for(Contour::const_iterator jt=contour.begin();jt!=contour.end();++jt)
				{
					result.back().push_back(jt->p);
				}
				result.back().push_back(contour.begin()->p);
			}
		}
		return result;
	}

private:
	static std::set<std::size_t> collect_pair_neighbors_from_pair_vertices(
			const std::size_t a_id,
			const std::size_t b_id,
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
					if(neighbor_id!=a_id && neighbor_id!=b_id)
					{
						neighbors_ids.insert(neighbor_id);
					}
				}
			}
		}
		return neighbors_ids;
	}

	static std::multimap<double, std::size_t> collect_pair_neighbors_with_distances(
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

	static bool check_if_radiuses_of_vertices_are_below_probe(
			const Triangulation::VerticesVector& vertices_vector,
			const std::set<std::size_t>& vertices_ids,
			const double probe)
	{
		for(std::set<std::size_t>::const_iterator it=vertices_ids.begin();it!=vertices_ids.end();++it)
		{
			if(vertices_vector[(*it)].second.r>=probe)
			{
				return false;
			}
		}
		return true;
	}

	static SimpleSphere construct_bounding_sphere_of_vertices_centers(
			const Triangulation::VerticesVector& vertices_vector,
			const std::set<std::size_t>& vertices_ids,
			const double radius_extension)
	{
		SimplePoint mc;
		for(std::set<std::size_t>::const_iterator it=vertices_ids.begin();it!=vertices_ids.end();++it)
		{
			mc=mc+SimplePoint(vertices_vector[(*it)].second);
		}
		mc=mc*(1.0/static_cast<double>(vertices_ids.size()));
		SimpleSphere result(mc, 0.0);
		for(std::set<std::size_t>::const_iterator it=vertices_ids.begin();it!=vertices_ids.end();++it)
		{
			result.r=std::max(result.r, distance_from_point_to_point(result, vertices_vector[(*it)].second));
		}
		result.r+=radius_extension;
		return result;
	}

	static Contour construct_circular_contour(
			const SimpleSphere& a,
			const SimpleSphere& b,
			const std::size_t a_id,
			const Triangulation::VerticesVector& vertices_vector,
			const std::set<std::size_t>& vertices_ids,
			const double probe,
			const double step)
	{
		Contour result;
		const SimpleSphere a_expanded=custom_sphere_from_point<SimpleSphere>(a, a.r+probe);
		const SimpleSphere b_expanded=custom_sphere_from_point<SimpleSphere>(b, b.r+probe);
		if(sphere_intersects_sphere(a_expanded, b_expanded))
		{
			const SimplePoint axis=sub_of_points<SimplePoint>(b, a).unit();
			if(vertices_ids.size()>1 && check_if_radiuses_of_vertices_are_below_probe(vertices_vector, vertices_ids, probe))
			{
				construct_circular_contour_from_base_and_axis(a_id, construct_bounding_sphere_of_vertices_centers(vertices_vector, vertices_ids, step), axis, step, result);
				for(Contour::iterator it=result.begin();it!=result.end();++it)
				{
					it->p=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(it->p, a, b);
				}
			}
			else
			{
				construct_circular_contour_from_base_and_axis(a_id, intersection_circle_of_two_spheres<SimpleSphere>(a_expanded, b_expanded), axis, step, result);
			}
		}
		return result;
	}

	static void construct_circular_contour_from_base_and_axis(
			const std::size_t a_id,
			const SimpleSphere& base,
			const SimplePoint& axis,
			const double step,
			Contour& result)
	{
		Rotation rotation(axis, 0);
		const SimplePoint first_point=any_normal_of_vector<SimplePoint>(rotation.axis)*base.r;
		const double angle_step=std::max(std::min(360*(step/(2*Rotation::pi()*base.r)), 60.0), 5.0);
		result.push_back(PointRecord(sum_of_points<SimplePoint>(base, first_point), a_id, a_id));
		for(rotation.angle=angle_step;rotation.angle<360;rotation.angle+=angle_step)
		{
			result.push_back(PointRecord(sum_of_points<SimplePoint>(base, rotation.rotate<SimplePoint>(first_point)), a_id, a_id));
		}
	}

	static bool cut_and_split_contour(
			const SimpleSphere& a,
			const SimpleSphere& c,
			const std::size_t c_id,
			Contour& contour,
			std::list<Contour>& segments)
	{
		const std::size_t outsiders_count=mark_contour(a, c, c_id, contour);
		if(outsiders_count>0)
		{
			if(outsiders_count<contour.size())
			{
				std::list<Contour::iterator> cuts;
				const int cuts_count=cut_contour(a, c, c_id, contour, cuts);
				if(cuts_count>0 && cuts_count%2==0)
				{
					if(cuts_count>2)
					{
						order_cuts(cuts);
						split_contour(contour, cuts, segments);
					}
				}
			}
			else
			{
				contour.clear();
			}
			return true;
		}
		return false;
	}

	static std::size_t mark_contour(
			const SimpleSphere& a,
			const SimpleSphere& c,
			const std::size_t c_id,
			Contour& contour)
	{
		std::size_t outsiders_count=0;
		for(Contour::iterator it=contour.begin();it!=contour.end();++it)
		{
			if(minimal_distance_from_point_to_sphere(it->p, c)<minimal_distance_from_point_to_sphere(it->p, a))
			{
				it->left_id=c_id;
				it->right_id=c_id;
				outsiders_count++;
			}
		}
		return outsiders_count;
	}

	static std::size_t cut_contour(
			const SimpleSphere& a,
			const SimpleSphere& c,
			const std::size_t c_id,
			Contour& contour,
			std::list<Contour::iterator>& cuts)
	{
		std::size_t cuts_count=0;
		Contour::iterator it=contour.begin();
		while(it!=contour.end())
		{
			if(it->left_id==c_id && it->right_id==c_id)
			{
				Contour::iterator left_it=get_left_iterator(contour, it);
				Contour::iterator right_it=get_right_iterator(contour, it);

				if(left_it->right_id!=c_id)
				{
					const SimplePoint& p0=it->p;
					const SimplePoint& p1=left_it->p;
					const double l=HyperboloidBetweenTwoSpheres::intersect_vector_with_hyperboloid(p0, p1, a, c);
					cuts.push_back(contour.insert(it, PointRecord(p0+((p1-p0).unit()*l), left_it->right_id, it->left_id)));
					cuts_count++;
				}

				if(right_it->left_id!=c_id)
				{
					const SimplePoint& p0=it->p;
					const SimplePoint& p1=right_it->p;
					const double l=HyperboloidBetweenTwoSpheres::intersect_vector_with_hyperboloid(p0, p1, a, c);
					cuts.push_back(contour.insert(right_it, PointRecord(p0+((p1-p0).unit()*l), it->right_id, right_it->left_id)));
					cuts_count++;
				}

				it=contour.erase(it);
			}
			else
			{
				++it;
			}
		}
		return cuts_count;
	}

	static void order_cuts(std::list<Contour::iterator>& cuts)
	{
		double sums[2]={0.0, 0.0};
		for(int i=0;i<2;i++)
		{
			if(i==1)
			{
				shift_list(cuts, false);
			}
			std::list<Contour::iterator>::const_iterator it=cuts.begin();
			while(it!=cuts.end())
			{
				std::list<Contour::iterator>::const_iterator next=it;
				++next;
				if(next!=cuts.end())
				{
					sums[i]+=distance_from_point_to_point((*it)->p, (*next)->p);
					it=next;
					++it;
				}
				else
				{
					it=cuts.end();
				}
			}
		}
		if(sums[0]<sums[1])
		{
			shift_list(cuts, true);
		}
	}

	static std::size_t split_contour(
			Contour& contour,
			const std::list<Contour::iterator>& ordered_cuts,
			std::list<Contour>& segments)
	{
		std::size_t segments_count=0;
		std::list<Contour::iterator>::const_iterator it=ordered_cuts.begin();
		while(it!=ordered_cuts.end())
		{
			std::list<Contour::iterator>::const_iterator next=it;
			++next;
			if(next!=ordered_cuts.end())
			{
				if((*next)!=get_right_iterator(contour, (*it)))
				{
					segments_count++;
					Contour segment;
					Contour::iterator jt=(*it);
					do
					{
						segment.push_back(*jt);
						jt=get_right_iterator(contour, jt);
					}
					while(jt!=(*next));
					segments.push_back(segment);
				}
				it=next;
				++it;
			}
			else
			{
				it=ordered_cuts.end();
			}
		}
		if(segments_count>0)
		{
			contour.clear();
		}
		return segments_count;
	}

	static void mend_contour(
			const SimpleSphere& a,
			const SimpleSphere& b,
			const SimpleSphere& c,
			const std::size_t c_id,
			const double step,
			const int projections,
			Contour& contour)
	{
		Contour::iterator it=contour.begin();
		while(it!=contour.end())
		{
			if(it->left_id!=c_id && it->right_id==c_id)
			{
				Contour::iterator jt=get_right_iterator(contour, it);
				if(jt->left_id==c_id)
				{
					SimplePoint& p0=it->p;
					SimplePoint& p1=jt->p;
					for(int e=0;e<projections;e++)
					{
						p0=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p0, b, c);
						p0=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p0, a, c);
						p0=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p0, a, b);
						p1=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p1, b, c);
						p1=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p1, a, c);
						p1=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p1, a, b);
					}
					const double distance=distance_from_point_to_point(p0, p1);
					if(distance>step)
					{
						const int leap_distance=static_cast<int>(floor(distance/step+0.5));
						const double leap_size=distance/static_cast<double>(leap_distance);
						const SimplePoint direction=(p1-p0).unit();
						for(int leap=1;leap<leap_distance;leap++)
						{
							SimplePoint p=p0+(direction*(leap_size*static_cast<double>(leap)));
							for(int e=0;e<projections;e++)
							{
								p=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p, b, c);
								p=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p, a, c);
								p=HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(p, a, b);
							}
							contour.insert(jt, PointRecord(p, c_id, c_id));
						}
					}
				}
			}
			++it;
		}
	}

	static bool check_contour_intersects_sphere(const SimpleSphere& shell, const Contour& contour)
	{
		for(Contour::const_iterator it=contour.begin();it!=contour.end();++it)
		{
			if(distance_from_point_to_point(shell, it->p)<=shell.r)
			{
				return true;
			}
		}
		return false;
	}

	static void filter_contours_intersecting_sphere(const SimpleSphere& shell, std::list<Contour>& contours)
	{
		std::list<Contour>::iterator it=contours.begin();
		while(it!=contours.end())
		{
			if(!check_contour_intersects_sphere(shell, (*it)))
			{
				it=contours.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	template<typename List, typename Iterator>
	static Iterator get_left_iterator(List& container, const Iterator& iterator)
	{
		Iterator left_it=iterator;
		if(left_it==container.begin())
		{
			left_it=container.end();
		}
		--left_it;
		return left_it;
	}

	template<typename List, typename Iterator>
	static Iterator get_right_iterator(List& container, const Iterator& iterator)
	{
		Iterator right_it=iterator;
		++right_it;
		if(right_it==container.end())
		{
			right_it=container.begin();
		}
		return right_it;
	}

	template<typename List>
	static void shift_list(List& list, const bool reverse)
	{
		if(!reverse)
		{
			list.push_front(*list.rbegin());
			list.pop_back();
		}
		else
		{
			list.push_back(*list.begin());
			list.pop_front();
		}
	}
};

}

#endif /* APOLLOTA_CONTACT_CONTOUR_H_ */

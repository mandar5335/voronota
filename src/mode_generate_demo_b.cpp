#include "apollota/spheres_boundary_construction.h"
#include "apollota/triangulation_queries.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/opengl_printer.h"
#include "auxiliaries/program_options_handler.h"

namespace
{

struct EdgeCurveParameters
{
	bool valid;
	std::vector<apollota::SimplePoint> p;
	std::vector<double> w;

	EdgeCurveParameters() : valid(false), p(3), w(3, 1)
	{
	}
};

std::pair<bool, apollota::Triple> get_common_triple_of_two_quadruples(const apollota::Quadruple& q1, const apollota::Quadruple& q2)
{
	for(unsigned int i=0;i<4;i++)
	{
		if(!q2.contains(q1.get(i)))
		{
			apollota::Triple t=q1.exclude(i);
			if(q2.contains(t.get(0)) && q2.contains(t.get(1)) && q2.contains(t.get(2)))
			{
				return std::pair<bool, apollota::Triple>(true, t);
			}
			else
			{
				return std::pair<bool, apollota::Triple>(false, apollota::Triple());
			}
		}
	}
	return std::pair<bool, apollota::Triple>(false, apollota::Triple());
}

std::pair<bool, apollota::SimplePoint> calculate_middle_control_point(
		const std::vector<apollota::SimpleSphere>& spheres,
		const apollota::Triple& t,
		const apollota::SimplePoint& start,
		const apollota::SimplePoint& end)
{
	static const double pi=acos(-1.0);
	if(!(spheres[t.get(0)].r==spheres[t.get(1)].r && spheres[t.get(0)].r==spheres[t.get(2)].r))
	{
		const apollota::SimplePoint p[2]={start, end};
		const apollota::SimplePoint d[2]={(p[1]-p[0]).unit(), (p[0]-p[1]).unit()};
		apollota::SimplePoint v[2];
		double a[2]={0.0, 0.0};
		for(int i=0;i<2;i++)
		{
			const std::vector<apollota::SimpleSphere> c=apollota::TangentSphereOfThreeSpheres::calculate(
					apollota::SimpleSphere((apollota::SimplePoint(spheres[t.get(0)])-p[i]).unit(), 0.0),
					apollota::SimpleSphere((apollota::SimplePoint(spheres[t.get(1)])-p[i]).unit(), 0.0),
					apollota::SimpleSphere((apollota::SimplePoint(spheres[t.get(2)])-p[i]).unit(), 0.0));
			if(c.size()==1)
			{
				v[i]=apollota::SimplePoint(c[0]).unit();
				double cos_value=(v[i]*d[i]);
				if(cos_value<0)
				{
					v[i]=v[i].inverted();
					cos_value=0.0-cos_value;
				}
				a[i]=acos(cos_value);
			}
			else
			{
				v[i]=d[i];
				a[i]=0.0;
			}
		}

		if(a[0]==0.0 || a[1]==0.0)
		{
			return std::make_pair(false, (p[0]+p[1])*0.5);
		}

		const double a2=pi-(a[0]+a[1]);
		if(a2>0.0)
		{
			const double a2=pi-(a[0]+a[1]);
			return std::make_pair(true, (p[0]+(v[0]*((p[1]-p[0]).module()*(sin(a[1]/sin(a2)))))));
		}
	}
	return std::make_pair(false, (start+end)*0.5);
}

apollota::SimplePoint calculate_barycentric_coordinates(const apollota::SimplePoint& a, const apollota::SimplePoint& b, const apollota::SimplePoint& c, const apollota::SimplePoint& p)
{
	const apollota::SimplePoint v0=(b-a);
	const apollota::SimplePoint v1=(c-a);
	const apollota::SimplePoint v2=(p-a);
	const double d00=(v0*v0);
	const double d01=(v0*v1);
	const double d11=(v1*v1);
	const double d20=(v2*v0);
	const double d21=(v2*v1);
	const double denom=(d00*d11-d01*d01);
	const double v=(d11*d20-d01*d21)/denom;
	const double w=(d00*d21-d01*d20)/denom;
	const double u=1.0-v-w;
	return apollota::SimplePoint(u, v, w);
}

EdgeCurveParameters calculate_edge_curve_parameters(
		const std::vector<apollota::SimpleSphere>& spheres,
		const apollota::Triangulation::VerticesVector& vertices_vector,
		const apollota::Pair& vertices_ids)
{
	EdgeCurveParameters ecp;
	const std::pair<bool, apollota::Triple> common_triple=get_common_triple_of_two_quadruples(vertices_vector[vertices_ids.get(0)].first, vertices_vector[vertices_ids.get(1)].first);
	if(common_triple.first)
	{
		const apollota::SimpleSphere& start_sphere=vertices_vector[vertices_ids.get(0)].second;
		const apollota::SimpleSphere& end_sphere=vertices_vector[vertices_ids.get(1)].second;
		const apollota::SimplePoint start_point(start_sphere);
		const apollota::SimplePoint end_point(end_sphere);
		const std::pair<bool, apollota::SimplePoint> middle_point=calculate_middle_control_point(spheres, common_triple.second, start_point, end_point);
		ecp.p[0]=start_point;
		ecp.p[1]=middle_point.second;
		ecp.p[2]=end_point;
		ecp.valid=true;
	}
	return ecp;
}

}

void generate_demo_b(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'x y z r')");
	pohw.describe_io("stdout", false, true, "nothing");

	const double probe=poh.restrict_value_in_range(0.01, 14.0, poh.argument<double>(pohw.describe_option("--probe", "number", "probe radius"), 1.4));
	const std::size_t b1=poh.argument<double>(pohw.describe_option("--b1", "number", "first ball number"));
	const std::size_t b2=poh.argument<double>(pohw.describe_option("--b2", "number", "second ball number"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::IOUtilities().read_lines_to_set(std::cin, spheres);

	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	if(!(b1<spheres.size() && b2<spheres.size()))
	{
		throw std::runtime_error("Invalid ball numbers.");
	}

	const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(spheres, probe*2.0);
	spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, false, false);
	const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);
	const apollota::Triangulation::VerticesGraph vertices_graph=apollota::Triangulation::construct_vertices_graph(spheres, triangulation_result.quadruples_map);
	const apollota::TriangulationQueries::PairsMap pairs_vertices=apollota::TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector);

	auxiliaries::OpenGLPrinter opengl_printer;

	for(apollota::TriangulationQueries::PairsMap::const_iterator pairs_vertices_it=pairs_vertices.begin();pairs_vertices_it!=pairs_vertices.end();++pairs_vertices_it)
	{
		const apollota::Pair& pair=pairs_vertices_it->first;
		const std::set<std::size_t>& pair_vertices_set=pairs_vertices_it->second;
		std::map< std::size_t, std::vector<std::size_t> > pair_vertices_graph;
		for(std::set<std::size_t>::const_iterator pair_vertices_it=pair_vertices_set.begin();pair_vertices_it!=pair_vertices_set.end();++pair_vertices_it)
		{
			const std::size_t id1=(*pair_vertices_it);
			const std::vector<std::size_t>& all_neighhbors=vertices_graph[id1];
			std::vector<std::size_t>& selected_neighbors=pair_vertices_graph[id1];
			for(std::vector<std::size_t>::const_iterator all_neighhbors_it=all_neighhbors.begin();all_neighhbors_it!=all_neighhbors.end();++all_neighhbors_it)
			{
				const std::size_t id2=(*all_neighhbors_it);
				if(id1!=id2 && pair_vertices_set.count(id2)>0)
				{
					selected_neighbors.push_back(id2);
				}
			}
			if(selected_neighbors.size()>2)
			{
				std::cerr << selected_neighbors.size() << " " << pair.get(0) << " " << pair.get(1) << "\n";
			}
		}
	}

	std::set<std::size_t> selected_vertex_ids;
	for(std::size_t i=0;i<vertices_vector.size();i++)
	{
		if(vertices_vector[i].first.contains(b1) && vertices_vector[i].first.contains(b2))
		{
			selected_vertex_ids.insert(i);
		}
	}
	for(std::set<std::size_t>::const_iterator i_it=selected_vertex_ids.begin();i_it!=selected_vertex_ids.end();++i_it)
	{
		const std::size_t i=(*i_it);
		for(std::size_t j=0;j<vertices_graph[i].size();j++)
		{
			if(i<vertices_graph[i][j] && selected_vertex_ids.count(vertices_graph[i][j])>0)
			{
				opengl_printer.add_color(0x00FF00);
				opengl_printer.add_line_strip(vertices_vector[i].second, vertices_vector[vertices_graph[i][j]].second);

				EdgeCurveParameters ecp=calculate_edge_curve_parameters(spheres, vertices_vector, apollota::Pair(i, vertices_graph[i][j]));
				if(ecp.valid)
				{
					opengl_printer.add_color(0xFF0000);
					opengl_printer.add_line_strip(ecp.p);
				}
			}
		}
	}

	opengl_printer.print_pymol_script("demo", false, std::cout);
}

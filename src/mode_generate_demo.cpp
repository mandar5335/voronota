#include "apollota/triangulation_queries.h"
#include "apollota/spheres_boundary_construction.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/opengl_printer.h"
#include "auxiliaries/program_options_handler.h"

void generate_demo(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'x y z r')");
	pohw.describe_io("stdout", false, true, "drawing script fo PyMol");

	const double probe=poh.restrict_value_in_range(0.01, 14.0, poh.argument<double>(pohw.describe_option("--probe", "number", "probe radius"), 1.4));
	const std::string representation=poh.argument<std::string>(pohw.describe_option("--representation", "string", "representation name"), "");

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

	const std::size_t input_spheres_count=spheres.size();
	const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(spheres, probe*2.0);
	spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());

	std::vector< std::pair<apollota::Triple, apollota::SimpleSphere> > alpha_shape_triples;
	{
		const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, false, false);
		const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);
		const apollota::TriangulationQueries::TriplesMap triples_map=apollota::TriangulationQueries::collect_triples_vertices_map_from_vertices_vector(vertices_vector);
		alpha_shape_triples.reserve(triples_map.size()/4);
		for(apollota::TriangulationQueries::TriplesMap::const_iterator triples_map_it=triples_map.begin();triples_map_it!=triples_map.end();++triples_map_it)
		{
			const apollota::Triple& triple=triples_map_it->first;
			const std::set<std::size_t>& vertices_ids=triples_map_it->second;
			if(triple.get_min_max().second<input_spheres_count)
			{
				bool exposed=false;
				for(std::set<std::size_t>::const_iterator vertices_ids_it=vertices_ids.begin();(!exposed && vertices_ids_it!=vertices_ids.end());++vertices_ids_it)
				{
					exposed=vertices_vector[*vertices_ids_it].second.r>probe;
				}
				if(exposed
						&& apollota::minimal_distance_from_sphere_to_sphere(spheres[triple.get(0)], spheres[triple.get(1)])<(probe*2.0)
						&& apollota::minimal_distance_from_sphere_to_sphere(spheres[triple.get(0)], spheres[triple.get(2)])<(probe*2.0)
						&& apollota::minimal_distance_from_sphere_to_sphere(spheres[triple.get(1)], spheres[triple.get(2)])<(probe*2.0))
				{
					const std::vector<apollota::SimpleSphere> tangents=apollota::TangentSphereOfThreeSpheres::calculate(spheres[triple.get(0)], spheres[triple.get(1)], spheres[triple.get(2)], probe);
					if(tangents.size()==2)
					{
						std::vector<bool> tangents_valid(tangents.size(), true);
						for(std::size_t i=0;i<tangents.size();i++)
						{
							for(std::set<std::size_t>::const_iterator vertices_ids_it=vertices_ids.begin();(tangents_valid[i] && vertices_ids_it!=vertices_ids.end());++vertices_ids_it)
							{
								const apollota::Quadruple& quadruple=vertices_vector[*vertices_ids_it].first;
								const int number_of_triple_in_quadruple=quadruple.number_of_subtuple(triple);
								if(number_of_triple_in_quadruple>=0)
								{
									tangents_valid[i]=!apollota::sphere_intersects_sphere(tangents[i], spheres[quadruple.get(number_of_triple_in_quadruple)]);
								}
							}
						}
						if(tangents_valid[0] && tangents_valid[1] && apollota::sphere_intersects_sphere(tangents[0], tangents[1]))
						{
							tangents_valid[0]=false;
							tangents_valid[1]=false;
						}
						for(std::size_t i=0;i<tangents.size();i++)
						{
							if(tangents_valid[i])
							{
								alpha_shape_triples.push_back(std::make_pair(triple, tangents[i]));
							}
						}
					}
				}
			}
		}
	}

	apollota::TriangulationQueries::PairsMap alpha_shape_pairs;
	{
		for(std::size_t i=0;i<alpha_shape_triples.size();i++)
		{
			const apollota::Triple& triple=alpha_shape_triples[i].first;
			for(unsigned int j=0;j<3;j++)
			{
				alpha_shape_pairs[triple.exclude(j)].insert(i);
			}
		}
	}

	{
		std::set<std::size_t> counts;
		for(apollota::TriangulationQueries::PairsMap::const_iterator pairs_it=alpha_shape_pairs.begin();pairs_it!=alpha_shape_pairs.end();++pairs_it)
		{
			const std::set<std::size_t>& triples_ids=pairs_it->second;
			counts.insert(triples_ids.size());
		}
		for(std::set<std::size_t>::const_iterator it=counts.begin();it!=counts.end();++it)
		{
			std::cerr << (*it) << "\n";
		}
	}

	if(representation=="")
	{
		auxiliaries::OpenGLPrinter opengl_printer;
		opengl_printer.add_color(0xFFFF00);
		for(std::size_t i=0;i<alpha_shape_triples.size();i++)
		{
			const apollota::Triple& triple=alpha_shape_triples[i].first;
			std::vector<apollota::SimplePoint> vertices(3);
			for(unsigned int j=0;j<3;j++)
			{
				vertices[j]=apollota::SimplePoint(spheres[triple.get(j)]);
			}
			const std::vector<apollota::SimplePoint> normals(3, ((vertices[1]-vertices[0])&(vertices[2]-vertices[0])).unit());
			opengl_printer.add_triangle_strip(vertices, normals);
		}
		opengl_printer.add_color(0xFF0000);
		for(apollota::TriangulationQueries::PairsMap::const_iterator pairs_it=alpha_shape_pairs.begin();pairs_it!=alpha_shape_pairs.end();++pairs_it)
		{
			const apollota::Pair& pair=pairs_it->first;
			opengl_printer.add_line_strip(spheres[pair.get(0)], spheres[pair.get(1)]);
		}
		opengl_printer.print_pymol_script("alpha_shape", true, std::cout);
	}
	else if(representation=="presurface")
	{
		auxiliaries::OpenGLPrinter opengl_printer;
		opengl_printer.add_color(0xFFFF00);
		for(std::size_t i=0;i<alpha_shape_triples.size();i++)
		{
			const apollota::Triple& triple=alpha_shape_triples[i].first;
			const apollota::SimpleSphere& tangent=alpha_shape_triples[i].second;
			std::vector<apollota::SimplePoint> vertices(3);
			for(unsigned int j=0;j<3;j++)
			{
				const apollota::SimplePoint a(spheres[triple.get(j)]);
				const apollota::SimplePoint b(tangent);
				vertices[j]=(b+((a-b).unit()*tangent.r));
			}
			const std::vector<apollota::SimplePoint> normals(3, ((vertices[1]-vertices[0])&(vertices[2]-vertices[0])).unit());
			opengl_printer.add_triangle_strip(vertices, normals);
		}
		opengl_printer.add_color(0xFF0000);
		for(apollota::TriangulationQueries::PairsMap::const_iterator pairs_it=alpha_shape_pairs.begin();pairs_it!=alpha_shape_pairs.end();++pairs_it)
		{
			const apollota::Pair& pair=pairs_it->first;
			const std::set<std::size_t>& triples_ids=pairs_it->second;
			if(triples_ids.size()==2)
			{
				const apollota::SimpleSphere tangents[2]={alpha_shape_triples[*triples_ids.begin()].second, alpha_shape_triples[*triples_ids.rbegin()].second};
				std::vector<apollota::SimplePoint> vertices(4);
				for(unsigned int j=0;j<4;j++)
				{
					const apollota::SimpleSphere& tangent=tangents[j/2];
					const apollota::SimplePoint a(spheres[pair.get(j%2)]);
					const apollota::SimplePoint b(tangent);
					vertices[j]=(b+((a-b).unit()*tangent.r));
				}
				const std::vector<apollota::SimplePoint> normals(4, ((vertices[1]-vertices[0])&(vertices[2]-vertices[0])).unit());
				opengl_printer.add_triangle_strip(vertices, normals);
			}
			else
			{
				opengl_printer.add_line_strip(spheres[pair.get(0)], spheres[pair.get(1)]);
			}
		}
		opengl_printer.print_pymol_script("presurface", true, std::cout);
	}
}

#include <iostream>
#include <stdexcept>

#include "apollota/constrained_contacts_construction.h"

#include "auxiliaries/opengl_printer.h"

#include "modescommon_assert_options.h"
#include "modescommon_read_sphere.h"
#include "modescommon_handle_contact.h"

namespace
{

typedef modescommon::contact::Comment Comment;
typedef modescommon::contact::ContactValue ContactValue;

std::string draw_inter_atom_contact(
		const std::vector<apollota::SimpleSphere>& spheres,
		const apollota::Triangulation::VerticesVector& vertices_vector,
		const apollota::TriangulationQueries::PairsMap& pairs_vertices,
		const std::size_t a_id,
		const std::size_t b_id,
		const double probe,
		const double step,
		const int projections)
{
	auxiliaries::OpenGLPrinter opengl_printer;
	if(a_id<spheres.size() && b_id<spheres.size())
	{
		apollota::TriangulationQueries::PairsMap::const_iterator pairs_vertices_it=pairs_vertices.find(apollota::Pair(a_id, b_id));
		if(pairs_vertices_it!=pairs_vertices.end())
		{
			const std::list<apollota::ConstrainedContactContour::Contour> contours=apollota::ConstrainedContactContour::construct_contact_contours(
					spheres, vertices_vector, pairs_vertices_it->second, a_id, b_id, probe, step, projections);
			for(std::list<apollota::ConstrainedContactContour::Contour>::const_iterator contours_it=contours.begin();contours_it!=contours.end();++contours_it)
			{
				const apollota::ConstrainedContactContour::Contour& contour=(*contours_it);
				const std::vector<apollota::SimplePoint> outline=apollota::ConstrainedContactContour::collect_points_from_contour(contour);
				opengl_printer.add_triangle_fan(
						apollota::HyperboloidBetweenTwoSpheres::project_point_on_hyperboloid(apollota::mass_center<apollota::SimplePoint>(outline.begin(), outline.end()), spheres[a_id], spheres[b_id]),
						outline,
						apollota::sub_of_points<apollota::SimplePoint>(spheres[b_id], spheres[a_id]).unit());
			}
		}
	}
	return opengl_printer.str();
}

std::string draw_solvent_contact(
		const std::vector<apollota::SimpleSphere>& spheres,
		const apollota::Triangulation::VerticesVector& vertices_vector,
		const apollota::TriangulationQueries::IDsMap& ids_vertices,
		const std::size_t a_id,
		const double probe,
		const apollota::SubdividedIcosahedron& sih)
{
	auxiliaries::OpenGLPrinter opengl_printer;
	if(a_id<spheres.size())
	{
		apollota::TriangulationQueries::IDsMap::const_iterator ids_vertices_it=ids_vertices.find(a_id);
		if(ids_vertices_it!=ids_vertices.end())
		{
			const apollota::ConstrainedContactRemainder::Remainder remainder=apollota::ConstrainedContactRemainder::construct_contact_remainder(
					spheres, vertices_vector, ids_vertices_it->second, a_id, probe, sih);
			for(apollota::ConstrainedContactRemainder::Remainder::const_iterator remainder_it=remainder.begin();remainder_it!=remainder.end();++remainder_it)
			{
				std::vector<apollota::SimplePoint> ts(3);
				std::vector<apollota::SimplePoint> ns(3);
				for(int i=0;i<3;i++)
				{
					ts[i]=remainder_it->p[i];
					ns[i]=apollota::sub_of_points<apollota::SimplePoint>(ts[i], spheres[a_id]).unit();
				}
				opengl_printer.add_triangle_strip(ts, ns);
			}
		}
	}
	return opengl_printer.str();
}

}

void calculate_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--annotate", "", "flag to annotate contacts using balls comments"));
		list_of_option_descriptions.push_back(OD("--probe", "number", "probe radius"));
		list_of_option_descriptions.push_back(OD("--exclude-hidden-balls", "", "flag to exclude hidden input balls"));
		list_of_option_descriptions.push_back(OD("--step", "number", "curve step length"));
		list_of_option_descriptions.push_back(OD("--projections", "number", "curve optimization depth"));
		list_of_option_descriptions.push_back(OD("--sih-depth", "number", "spherical surface optimization depth"));
		list_of_option_descriptions.push_back(OD("--draw", "", "flag to output graphics for annotated contacts"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of balls\n";
			std::cerr << "              (default line format: 'x y z r # comments')\n";
			std::cerr << "              (annotated line format: 'x y z r # atomSerial chainID resSeq resName atomName [altLoc iCode]')\n";
			std::cerr << "stdout  ->  list of contacts\n";
			std::cerr << "              (default line format: 'b1 b2 area')\n";
			std::cerr << "              (annotated line format: 'annotation1 annotation2 area distance [graphics]')\n";
			return;
		}
	}

	const bool annotate=poh.contains_option("--annotate");
	const bool exclude_hidden_balls=poh.contains_option("--exclude-hidden-balls");
	const double probe=std::max(0.01, std::min(14.0, poh.argument<double>("--probe", 1.4)));
	const double step=std::max(0.05, std::min(0.5, poh.argument<double>("--step", 0.2)));
	const int projections=std::max(1, std::min(10, poh.argument<int>("--projections", 5)));
	const int sih_depth=std::max(1, std::min(5, poh.argument<int>("--sih-depth", 3)));
	const bool draw=poh.contains_option("--draw");

	std::vector<apollota::SimpleSphere> spheres;
	std::vector<Comment> input_spheres_comments;
	if(annotate)
	{
		std::pair< std::vector<apollota::SimpleSphere>*, std::vector<Comment>* > spheres_with_comments(&spheres, &input_spheres_comments);
		auxiliaries::read_lines_to_container(std::cin, "", modescommon::add_sphere_and_comments_from_stream_to_vectors<apollota::SimpleSphere, Comment>, spheres_with_comments);
		if(spheres.size()!=input_spheres_comments.size())
		{
			throw std::runtime_error("Number of comments does not match number of spheres.");
		}
	}
	else
	{
		auxiliaries::read_lines_to_container(std::cin, "#", modescommon::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
	}
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	const std::size_t input_spheres_count=spheres.size();
	const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::ConstrainedContactsConstruction::construct_artificial_boundary(spheres, probe*2.0);
	spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, exclude_hidden_balls, false);
	const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);
	const apollota::TriangulationQueries::IDsMap ids_map=apollota::TriangulationQueries::collect_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map);

	std::map<apollota::Pair, double> interactions_map;
	
	const std::map<apollota::Pair, double> constrained_contacts=apollota::ConstrainedContactsConstruction::construct_contacts(spheres, vertices_vector, probe, step, projections);
	for(std::map<apollota::Pair, double>::const_iterator it=constrained_contacts.begin();it!=constrained_contacts.end();++it)
	{
		if(it->first.get(0)<input_spheres_count && it->first.get(1)<input_spheres_count)
		{
			interactions_map[it->first]=it->second;
		}
	}

	const std::map<std::size_t, double> constrained_contact_remainders=apollota::ConstrainedContactsConstruction::construct_contact_remainders(spheres, vertices_vector, probe, sih_depth);
	for(std::map<std::size_t, double>::const_iterator it=constrained_contact_remainders.begin();it!=constrained_contact_remainders.end();++it)
	{
		if(it->first<input_spheres_count)
		{
			interactions_map[apollota::Pair(it->first, it->first)]=it->second;
		}
	}

	if(!input_spheres_comments.empty())
	{
		const apollota::TriangulationQueries::PairsMap pairs_vertices=(draw ? apollota::TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector) : apollota::TriangulationQueries::PairsMap());
		const apollota::TriangulationQueries::IDsMap ids_vertices=(draw ? apollota::TriangulationQueries::collect_vertices_map_from_vertices_vector(vertices_vector) : apollota::TriangulationQueries::IDsMap());
		const apollota::SubdividedIcosahedron sih(draw ? sih_depth : 0);

		std::map< std::pair<Comment, Comment>, ContactValue > output_map_of_contacts;
		for(std::map<apollota::Pair, double>::const_iterator it=interactions_map.begin();it!=interactions_map.end();++it)
		{
			const double area=it->second;
			if(area>0.0)
			{
				const std::size_t a_id=it->first.get(0);
				const std::size_t b_id=it->first.get(1);
				const std::pair<Comment, Comment> comments(input_spheres_comments[a_id], (a_id==b_id ? Comment::solvent() : input_spheres_comments[b_id]));
				ContactValue& value=output_map_of_contacts[modescommon::contact::refine_pair(comments, comments.second<comments.first)];
				value.area=area;
				if(a_id!=b_id)
				{
					value.dist=apollota::distance_from_point_to_point(spheres[a_id], spheres[b_id]);
				}
				else
				{
					value.dist=(spheres[a_id].r+(probe*3.0));
				}
				if(draw)
				{
					value.graphics=(a_id==b_id ?
							draw_solvent_contact(spheres, vertices_vector, ids_vertices, a_id, probe, sih) :
							draw_inter_atom_contact(spheres, vertices_vector, pairs_vertices, a_id, b_id, probe, step, projections));
				}
			}
		}
		for(std::map< std::pair<Comment, Comment>, ContactValue >::const_iterator it=output_map_of_contacts.begin();it!=output_map_of_contacts.end();++it)
		{
			modescommon::contact::print_contact_record(it->first, it->second, true, std::cout);
		}
	}
	else
	{
		for(std::map<apollota::Pair, double>::const_iterator it=interactions_map.begin();it!=interactions_map.end();++it)
		{
			std::cout << it->first.get(0) << " " << it->first.get(1) << " " << it->second << "\n";
		}
	}
}

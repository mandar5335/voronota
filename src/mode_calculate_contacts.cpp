#include <iostream>
#include <stdexcept>

#include "apollota/constrained_contacts_construction.h"
#include "apollota/spheres_boundary_construction.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"
#include "auxiliaries/opengl_printer.h"

#include "modescommon/ball_value.h"
#include "modescommon/contact_value.h"

namespace
{

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

bool check_inter_atom_contact_centrality(
		const std::vector<apollota::SimpleSphere>& spheres,
		const apollota::TriangulationQueries::PairsMap& pairs_neighbors,
		const std::size_t a_id,
		const std::size_t b_id)
{
	if(a_id<spheres.size() && b_id<spheres.size())
	{
		apollota::TriangulationQueries::PairsMap::const_iterator pairs_neighbors_it=pairs_neighbors.find(apollota::Pair(a_id, b_id));
		if(pairs_neighbors_it!=pairs_neighbors.end())
		{
			const std::set<std::size_t>& neighbors=pairs_neighbors_it->second;
			if(!neighbors.empty())
			{
				const apollota::SimplePoint pa(spheres[a_id]);
				const apollota::SimplePoint pb(spheres[b_id]);
				const double ra=spheres[a_id].r;
				const double rb=spheres[b_id].r;
				const double distance_to_a_or_b=((apollota::distance_from_point_to_point(pa, pb)-ra-rb)*0.5);
				const apollota::SimplePoint p=(pa+((pb-pa).unit()*(ra+distance_to_a_or_b)));
				for(std::set<std::size_t>::const_iterator neighbors_it=neighbors.begin();neighbors_it!=neighbors.end();++neighbors_it)
				{
					const std::size_t c_id=(*neighbors_it);
					if(c_id<spheres.size() && apollota::minimal_distance_from_point_to_sphere(p, spheres[c_id])<distance_to_a_or_b)
					{
						return false;
					}
				}
			}
		}
	}
	return true;
}

bool check_inter_atom_contact_peripherial(
		const std::vector<apollota::SimpleSphere>& spheres,
		const apollota::Triangulation::VerticesVector& vertices_vector,
		const apollota::TriangulationQueries::PairsMap& pairs_vertices,
		const std::size_t a_id,
		const std::size_t b_id,
		const double probe)
{
	if(a_id<spheres.size() && b_id<spheres.size())
	{
		apollota::TriangulationQueries::PairsMap::const_iterator pairs_vertices_it=pairs_vertices.find(apollota::Pair(a_id, b_id));
		if(pairs_vertices_it!=pairs_vertices.end())
		{
			const std::set<std::size_t>& vertices_ids=pairs_vertices_it->second;
			for(std::set<std::size_t>::const_iterator vertices_ids_it=vertices_ids.begin();vertices_ids_it!=vertices_ids.end();++vertices_ids_it)
			{
				const std::size_t vertex_id=(*vertices_ids_it);
				if(vertex_id<vertices_vector.size() && vertices_vector[vertex_id].second.r>probe)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool identify_mock_solvent(const auxiliaries::ChainResidueAtomDescriptor& crad)
{
	return (crad.name=="w" && crad.resName=="w");
}

}

void calculate_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false,
			"list of balls\n(default mode line format: 'x y z r')\n(annotated mode line format: 'annotation x y z r tags adjuncts')");
	pohw.describe_io("stdout", false, true,
			"list of contacts\n(default mode line format: 'b1 b2 area')\n(annotated mode line format: 'annotation1 annotation2 area distance tags adjuncts [graphics]')");

	const bool annotated=poh.contains_option(pohw.describe_option("--annotated", "", "flag to enable annotated mode"));
	const double probe=poh.restrict_value_in_range(0.01, 14.0, poh.argument<double>(pohw.describe_option("--probe", "number", "probe radius"), 1.4));
	const bool exclude_hidden_balls=poh.contains_option(pohw.describe_option("--exclude-hidden-balls", "", "flag to exclude hidden input balls"));
	const double step=poh.restrict_value_in_range(0.05, 0.5, poh.argument<double>(pohw.describe_option("--step", "number", "curve step length"), 0.2));
	const int projections=poh.restrict_value_in_range(1, 10, poh.argument<int>(pohw.describe_option("--projections", "number", "curve optimization depth"), 5));
	const int sih_depth=poh.restrict_value_in_range(1, 5, poh.argument<int>(pohw.describe_option("--sih-depth", "number", "spherical surface optimization depth"), 3));
	const bool add_mirrored=poh.contains_option(pohw.describe_option("--add-mirrored", "", "flag to add mirrored contacts to non-annnotated output"));
	const bool draw=poh.contains_option(pohw.describe_option("--draw", "", "flag to output graphics for annotated contacts"));
	const bool tag_centrality=poh.contains_option(pohw.describe_option("--tag-centrality", "", "flag to tag contacts centrality"));
	const bool tag_peripherial=poh.contains_option(pohw.describe_option("--tag-peripherial", "", "flag to tag peripherial contacts"));
	const std::string volumes_output=poh.argument<std::string>(pohw.describe_option("--volumes-output", "string", "file path to output constrained cells volumes"), "");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::vector<apollota::SimpleSphere> spheres;
	std::vector< std::pair<auxiliaries::ChainResidueAtomDescriptor, BallValue> > input_ball_records;
	std::set<std::size_t> mock_solvent_ids;
	if(annotated)
	{
		auxiliaries::IOUtilities().read_lines_to_map(std::cin, input_ball_records);
		spheres.reserve(input_ball_records.size());
		for(std::size_t i=0;i<input_ball_records.size();i++)
		{
			spheres.push_back(apollota::SimpleSphere(input_ball_records[i].second));
			if(identify_mock_solvent(input_ball_records[i].first))
			{
				mock_solvent_ids.insert(i);
			}
		}
	}
	else
	{
		auxiliaries::IOUtilities().read_lines_to_set(std::cin, spheres);
	}
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	const std::size_t input_spheres_count=spheres.size();
	const std::vector<apollota::SimpleSphere> artificial_boundary=apollota::construct_artificial_boundary(spheres, probe*2.0);
	spheres.insert(spheres.end(), artificial_boundary.begin(), artificial_boundary.end());

	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, exclude_hidden_balls, false);
	const apollota::Triangulation::VerticesVector vertices_vector=apollota::Triangulation::collect_vertices_vector_from_quadruples_map(triangulation_result.quadruples_map);

	std::map<apollota::Pair, double> interactions_map;
	std::pair< bool, std::map<std::size_t, double> > volumes_map_bundle(!volumes_output.empty(), std::map<std::size_t, double>());
	
	{
		const std::map<apollota::Pair, double> constrained_contacts=apollota::ConstrainedContactsConstruction::construct_contacts(spheres, vertices_vector, probe, step, projections, mock_solvent_ids, volumes_map_bundle);
		for(std::map<apollota::Pair, double>::const_iterator it=constrained_contacts.begin();it!=constrained_contacts.end();++it)
		{
			if(it->first.get(0)<input_spheres_count && it->first.get(1)<input_spheres_count)
			{
				interactions_map[it->first]=it->second;
			}
		}
	}

	if(mock_solvent_ids.empty())
	{
		const std::map<std::size_t, double> constrained_contact_remainders=apollota::ConstrainedContactsConstruction::construct_contact_remainders(spheres, vertices_vector, probe, sih_depth, volumes_map_bundle);
		for(std::map<std::size_t, double>::const_iterator it=constrained_contact_remainders.begin();it!=constrained_contact_remainders.end();++it)
		{
			if(it->first<input_spheres_count)
			{
				interactions_map[apollota::Pair(it->first, it->first)]=it->second;
			}
		}
	}

	if(!input_ball_records.empty())
	{
		const apollota::TriangulationQueries::PairsMap pairs_vertices=(draw ? apollota::TriangulationQueries::collect_pairs_vertices_map_from_vertices_vector(vertices_vector) : apollota::TriangulationQueries::PairsMap());
		const apollota::TriangulationQueries::IDsMap ids_vertices=(draw ? apollota::TriangulationQueries::collect_vertices_map_from_vertices_vector(vertices_vector) : apollota::TriangulationQueries::IDsMap());
		const apollota::SubdividedIcosahedron sih(draw ? sih_depth : 0);
		const apollota::TriangulationQueries::PairsMap pairs_neighbors=(tag_centrality ? apollota::TriangulationQueries::collect_pairs_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map) : apollota::TriangulationQueries::PairsMap());

		std::map< auxiliaries::ChainResidueAtomDescriptorsPair, ContactValue > output_map_of_contacts;
		for(std::map<apollota::Pair, double>::const_iterator it=interactions_map.begin();it!=interactions_map.end();++it)
		{
			const double area=it->second;
			if(area>0.0)
			{
				const std::size_t a_id=it->first.get(0);
				const std::size_t b_id=it->first.get(1);
				auxiliaries::ChainResidueAtomDescriptor crad_a=input_ball_records[a_id].first;
				auxiliaries::ChainResidueAtomDescriptor crad_b=input_ball_records[b_id].first;
				if(identify_mock_solvent(crad_a))
				{
					crad_a=auxiliaries::ChainResidueAtomDescriptor::solvent();
				}
				if(a_id==b_id || identify_mock_solvent(crad_b))
				{
					crad_b=auxiliaries::ChainResidueAtomDescriptor::solvent();
				}
				if(crad_a!=auxiliaries::ChainResidueAtomDescriptor::solvent() || crad_b!=auxiliaries::ChainResidueAtomDescriptor::solvent())
				{
					ContactValue value;
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
					if(tag_centrality && crad_a!=auxiliaries::ChainResidueAtomDescriptor::solvent() && crad_b!=auxiliaries::ChainResidueAtomDescriptor::solvent() && check_inter_atom_contact_centrality(spheres, pairs_neighbors, a_id, b_id))
					{
						value.props.tags.insert("central");
					}
					if(tag_peripherial && crad_a!=auxiliaries::ChainResidueAtomDescriptor::solvent() && crad_b!=auxiliaries::ChainResidueAtomDescriptor::solvent() && check_inter_atom_contact_peripherial(spheres, vertices_vector, pairs_vertices, a_id, b_id, probe))
					{
						value.props.tags.insert("peripherial");
					}
					output_map_of_contacts[auxiliaries::ChainResidueAtomDescriptorsPair(crad_a, crad_b)].add(value);
				}
			}
		}
		auxiliaries::IOUtilities().write_map(output_map_of_contacts, std::cout);
	}
	else
	{
		if(add_mirrored)
		{
			std::map< std::pair<std::size_t, std::size_t>, double > mirrored_interactions_map;
			for(std::map<apollota::Pair, double>::const_iterator it=interactions_map.begin();it!=interactions_map.end();++it)
			{
				mirrored_interactions_map[std::make_pair(it->first.get(0), it->first.get(1))]=it->second;
				if(it->first.get(0)!=it->first.get(1))
				{
					mirrored_interactions_map[std::make_pair(it->first.get(1), it->first.get(0))]=it->second;
				}
			}
			for(std::map< std::pair<std::size_t, std::size_t>, double >::const_iterator it=mirrored_interactions_map.begin();it!=mirrored_interactions_map.end();++it)
			{
				std::cout << it->first.first << " " << it->first.second << " " << it->second << "\n";
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

	if(volumes_map_bundle.first && !volumes_map_bundle.second.empty() && !volumes_output.empty())
	{
		const std::map<std::size_t, double>& volumes_map=volumes_map_bundle.second;
		if(!input_ball_records.empty())
		{
			std::map<auxiliaries::ChainResidueAtomDescriptor, double> output_volumes_map;
			for(std::map<std::size_t, double>::const_iterator it=volumes_map.begin();it!=volumes_map.end();++it)
			{
				const double volume=it->second;
				if(volume>0.0)
				{
					const std::size_t a_id=it->first;
					if(a_id<input_ball_records.size())
					{
						const auxiliaries::ChainResidueAtomDescriptor& crad_a=input_ball_records[a_id].first;
						output_volumes_map[crad_a]=volume;
					}
				}
			}
			auxiliaries::IOUtilities().write_map_to_file(output_volumes_map, volumes_output);
		}
		else
		{
			std::map<std::size_t, double> output_volumes_map;
			for(std::map<std::size_t, double>::const_iterator it=volumes_map.begin();it!=volumes_map.end();++it)
			{
				const double volume=it->second;
				if(volume>0.0)
				{
					const std::size_t a_id=it->first;
					if(a_id<input_spheres_count)
					{
						output_volumes_map[a_id]=volume;
					}
				}
			}
			auxiliaries::IOUtilities().write_map_to_file(output_volumes_map, volumes_output);
		}
	}
}

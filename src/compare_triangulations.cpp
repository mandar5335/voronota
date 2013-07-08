#include <iostream>
#include <fstream>

#include "apollota/comparison_of_triangulations.h"

#include "auxiliaries/io_utilities.h"
#include "auxiliaries/command_line_options_handler.h"

namespace
{

void add_sphere_from_stream_to_vector(std::istream& input, std::vector<apollota::SimpleSphere>& spheres)
{
	apollota::SimpleSphere sphere;
	input >> sphere.x >> sphere.y >> sphere.z >> sphere.r;
	if(!input.fail())
	{
		spheres.push_back(sphere);
	}
}

void add_quadruple_from_stream_to_vector(std::istream& input, std::vector<apollota::Quadruple>& quadruples)
{
	std::vector<std::size_t> numbers(4, 0);
	for(std::size_t i=0;i<numbers.size() && !input.fail();i++)
	{
		input >> numbers[i];
	}
	if(!input.fail())
	{
		quadruples.push_back(apollota::Quadruple(numbers));
	}

}

}

void compare_triangulations(const auxiliaries::CommandLineOptionsHandler& clo)
{
	{
		typedef auxiliaries::CommandLineOptionsHandler Clo;
		Clo::MapOfOptionDescriptions map_of_option_descriptions;
		map_of_option_descriptions["--bounding-spheres-hierarchy-first-radius"]=Clo::OptionDescription(true, "initial radius for bounding sphere hierarchy");
		map_of_option_descriptions["--first-triangulation-file"]=Clo::OptionDescription(true, "path to the first triangulation file");
		map_of_option_descriptions["--second-triangulation-file"]=Clo::OptionDescription(true, "path to the second triangulation file");
		if(clo.contains_option("--help"))
		{
			Clo::print_map_of_option_descriptions(map_of_option_descriptions, "    ", 50, std::cerr);
			return;
		}
		else
		{
			clo.compare_with_map_of_option_descriptions(map_of_option_descriptions);
		}
	}

	const double bounding_spheres_hierarchy_first_radius=clo.argument<double>("--bounding-spheres-hierarchy-first-radius", 3.5);
	if(bounding_spheres_hierarchy_first_radius<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy first radius is not greater than 1.");
	}

	const std::string first_triangulation_file=clo.argument<std::string>("--first-triangulation-file");
	const std::string second_triangulation_file=clo.argument<std::string>("--second-triangulation-file");

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", add_sphere_from_stream_to_vector, spheres);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided.");
	}

	std::ifstream first_triangulation_stream(first_triangulation_file.c_str(), std::ifstream::in);
	std::ifstream second_triangulation_stream(second_triangulation_file.c_str(), std::ifstream::in);

	std::vector<apollota::Quadruple> first_triangulation_quadruples;
	auxiliaries::read_lines_to_container(std::cin, "#", add_quadruple_from_stream_to_vector, first_triangulation_quadruples);
	std::vector<apollota::Quadruple> second_triangulation_quadruples;
	auxiliaries::read_lines_to_container(std::cin, "#", add_quadruple_from_stream_to_vector, first_triangulation_quadruples);

	const apollota::ComparisonOfTriangulations::Result differences=apollota::ComparisonOfTriangulations::calculate_directional_difference_between_two_sets_of_quadruples(spheres, bounding_spheres_hierarchy_first_radius, first_triangulation_quadruples, second_triangulation_quadruples);

	std::cout << "all_differences " << differences.all_differences.size() << "\n";
	std::cout << "confirmed_differences " << differences.confirmed_differences.size() << "\n";
}

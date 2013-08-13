#include <iostream>

#include "apollota/triangulation.h"

#include "modes_commons.h"

namespace
{

inline std::set<std::string> get_available_processing_method_names()
{
	std::set<std::string> names;
	names.insert("sequential");
#ifdef _OPENMP
	names.insert("openmp");
#endif
	return names;
}

inline std::string list_strings_from_set(const std::set<std::string>& names)
{
	std::ostringstream output;
	for(std::set<std::string>::const_iterator it=names.begin();it!=names.end();++it)
	{
		output << " '" << (*it) << "'";
	}
	return output.str();
}

inline bool number_is_power_of_two(const unsigned int x)
{
	return ( (x>0) && ((x & (x-1))==0) );
}

}

void calculate_triangulation_in_parallel(const auxiliaries::ProgramOptionsHandler& poh)
{
	const std::set<std::string> available_processing_method_names=get_available_processing_method_names();

	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--method"].init("string", "processing method name, variants are:"+list_strings_from_set(available_processing_method_names), true);
		basic_map_of_option_descriptions["--parts"].init("number", "number of parts for splitting, must be power of 2", true);
		basic_map_of_option_descriptions["--selection"].init("numbers", "numbers of selected parts - if not provided, all parts are selected");
		basic_map_of_option_descriptions["--skip-output"].init("", "flag to disable output of the resulting triangulation");
		basic_map_of_option_descriptions["--print-log"].init("", "flag to print log of calculations");
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions full_map_of_option_descriptions=basic_map_of_option_descriptions;
		full_map_of_option_descriptions["--init-radius-for-BSH"].init("number", "initial radius for bounding sphere hierarchy");
		if(poh.contains_option("--help") || poh.contains_option("--help-full"))
		{
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions(poh.contains_option("--help-full") ? full_map_of_option_descriptions : basic_map_of_option_descriptions, std::cerr);
			std::cerr << "\n";
			std::cerr << "  stdin   <-  list of balls (line format: 'x y z r')\n";
			std::cerr << "  stdout  ->  list of quadruples with tangent spheres (line format: 'q1 q2 q3 q4 x y z r')\n";
			return;
		}
		else
		{
			poh.compare_with_map_of_option_descriptions(full_map_of_option_descriptions);
		}
	}

	const std::string method=poh.argument<std::string>("--method");
	if(available_processing_method_names.count(method)==0)
	{
		throw std::runtime_error("Invalid processing method name, acceptable values are:"+list_strings_from_set(available_processing_method_names)+".");
	}

	const unsigned int parts=poh.argument<unsigned int>("--parts");
	if(!number_is_power_of_two(parts))
	{
		throw std::runtime_error("Number of parts must be power of 2.");
	}

	const std::vector<unsigned int> selection=poh.argument_vector<unsigned int>("--selection");
	for(std::size_t i=0;i<selection.size();i++)
	{
		if(selection[i]>=parts)
		{
			throw std::runtime_error("Every selection number should be less than number of parts.");
		}
	}

	const bool skip_output=poh.contains_option("--skip-output");

	const bool print_log=poh.contains_option("--print-log");

	const double init_radius_for_BSH=poh.argument<double>("--init-radius-for-BSH", 3.5);
	if(init_radius_for_BSH<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy initial radius should be greater than 1.");
	}

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	const std::vector< std::vector<std::size_t> > all_distributed_ids=apollota::SplittingOfSpheres::split_for_number_of_parts(spheres, parts);
	std::vector< std::vector<std::size_t> > distributed_ids;
	{
		if(selection.empty())
		{
			distributed_ids=all_distributed_ids;
		}
		else
		{
			distributed_ids.reserve(selection.size());
			for(std::size_t i=0;i<selection.size();i++)
			{
				if(selection[i]<all_distributed_ids.size())
				{
					distributed_ids.push_back(all_distributed_ids[selection[i]]);
				}
			}
		}
	}
	if(distributed_ids.empty())
	{
		throw std::runtime_error("No requested parts available.");
	}

	const apollota::BoundingSpheresHierarchy bsh(spheres, init_radius_for_BSH, 1);

	std::vector<apollota::Triangulation::QuadruplesMap> distributed_quadruples_maps(distributed_ids.size());

	if(method=="sequential")
	{
		for(std::size_t i=0;i<distributed_ids.size();i++)
		{
			distributed_quadruples_maps[i]=apollota::Triangulation::construct_result_for_admittance_set(bsh, distributed_ids[i]).quadruples_map;
		}
	}
#ifdef _OPENMP
	else if(method=="openmp")
	{
		int errors=0;
		{
#pragma omp parallel for reduction(+:errors)
			for(std::size_t i=0;i<distributed_ids.size();i++)
			{
				try
				{
					distributed_quadruples_maps[i]=apollota::Triangulation::construct_result_for_admittance_set(bsh, distributed_ids[i]).quadruples_map;
				}
				catch(...)
				{
					errors++;
				}
			}
		}
		if(errors>0)
		{
			throw std::runtime_error("Parallel processing failed because of exception.");
		}
	}
#endif
	else
	{
		throw std::runtime_error("Processing method '"+method+"' is not available.");
	}

	std::size_t sum_of_all_produced_quadruples_counts=0;
	apollota::Triangulation::QuadruplesMap result_quadruples_map;
	for(std::size_t i=0;i<distributed_quadruples_maps.size();i++)
	{
		sum_of_all_produced_quadruples_counts+=distributed_quadruples_maps[i].size();
		apollota::Triangulation::merge_quadruples_maps(distributed_quadruples_maps[i], result_quadruples_map);
	}

	if(!skip_output)
	{
		apollota::Triangulation::print_quadruples_map(result_quadruples_map, std::cout);
	}

	if(print_log)
	{
		std::clog << "balls " << spheres.size() << "\n";
		std::clog << "processing_method " << method << "\n";
		std::clog << "all_parts " << all_distributed_ids.size() << "\n";
		std::clog << "processed_parts " << distributed_ids.size() << "\n";
		std::clog << "quadruples " << result_quadruples_map.size() << "\n";
		std::clog << "tangent_spheres " << apollota::Triangulation::count_tangent_spheres_in_quadruples_map(result_quadruples_map) << "\n";
		std::clog << "parallel_results_overlap " << (static_cast<double>(sum_of_all_produced_quadruples_counts)/static_cast<double>(result_quadruples_map.size())-1.0) << "\n";
	}
}

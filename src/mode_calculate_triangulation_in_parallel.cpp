#include <iostream>

#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <thread>
#endif

#include "apollota/triangulation.h"
#include "apollota/splitting_of_spheres.h"

#include "modes_commons.h"

namespace
{

inline std::set<std::string> get_available_processing_method_names()
{
	std::set<std::string> names;

	names.insert("sequential");

#ifdef __GXX_EXPERIMENTAL_CXX0X__
	names.insert("std::thread");
#endif

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

inline void run_thread_job(const apollota::BoundingSpheresHierarchy* bsh_ptr, const std::vector<std::size_t>* thread_ids_ptr, apollota::Triangulation::QuadruplesMap* result_quadruples_map_ptr)
{
	apollota::Triangulation::merge_quadruples_maps(apollota::Triangulation::construct_result_for_admittance_set(*bsh_ptr, *thread_ids_ptr).quadruples_map, *result_quadruples_map_ptr);
}

}

void calculate_triangulation_in_parallel(const auxiliaries::ProgramOptionsHandler& poh)
{
	const std::set<std::string> available_processing_method_names=get_available_processing_method_names();

	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--method"].init("string", "processing method name, variants are:"+list_strings_from_set(available_processing_method_names));
		basic_map_of_option_descriptions["--parts"].init("number", "minimal number of parts for splitting");
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

	std::string method=poh.argument<std::string>("--method");
	if(available_processing_method_names.count(method)==0)
	{
		throw std::runtime_error("Invalid processing method name, acceptable values are:"+list_strings_from_set(available_processing_method_names)+".");
	}

	const unsigned int parts=poh.argument<double>("--parts");
	if(parts<1 || parts>1024)
	{
		throw std::runtime_error("Number of parts should be in interval [1, 1024].");
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

	const std::vector< std::vector<std::size_t> > distributed_ids=apollota::SplittingOfSpheres::split_for_number_of_parts(spheres, parts);

	const apollota::BoundingSpheresHierarchy bsh(spheres, init_radius_for_BSH, 1);

	std::size_t sum_of_all_produced_quadruples_counts=0;
	apollota::Triangulation::QuadruplesMap result_quadruples_map;

	if(method=="sequential")
	{
		for(std::size_t i=0;i<distributed_ids.size();i++)
		{
			apollota::Triangulation::QuadruplesMap temp_quadruples_map;
			run_thread_job(&bsh, &distributed_ids[i], &temp_quadruples_map);
			sum_of_all_produced_quadruples_counts+=temp_quadruples_map.size();
			apollota::Triangulation::merge_quadruples_maps(temp_quadruples_map, result_quadruples_map);
		}
	}
	else if(method=="std::thread")
	{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
		{
			std::vector<std::thread> thread_handles;
			std::vector<apollota::Triangulation::QuadruplesMap> distributed_quadruples_maps(distributed_ids.size());
			for(std::size_t i=0;i<distributed_ids.size();i++)
			{
				thread_handles.push_back(std::thread(run_thread_job, &bsh, &distributed_ids[i], &distributed_quadruples_maps[i]));
			}
			for(std::size_t i=0;i<thread_handles.size();i++)
			{
				thread_handles[i].join();
				sum_of_all_produced_quadruples_counts+=distributed_quadruples_maps[i].size();
				apollota::Triangulation::merge_quadruples_maps(distributed_quadruples_maps[i], result_quadruples_map);
			}
		}
#else
#endif
	}
	else if(method=="openmp")
	{
#ifdef _OPENMP
		{
			std::vector<apollota::Triangulation::QuadruplesMap> distributed_quadruples_maps(distributed_ids.size());

			{
#pragma omp parallel for
				for(std::size_t i=0;i<distributed_ids.size();i++)
				{
					run_thread_job(&bsh, &distributed_ids[i], &distributed_quadruples_maps[i]);
				}
			}

			for(std::size_t i=0;i<distributed_quadruples_maps.size();i++)
			{
				sum_of_all_produced_quadruples_counts+=distributed_quadruples_maps[i].size();
				apollota::Triangulation::merge_quadruples_maps(distributed_quadruples_maps[i], result_quadruples_map);
			}
		}
#endif
	}

	if(!skip_output)
	{
		apollota::Triangulation::print_quadruples_map(result_quadruples_map, std::cout);
	}

	if(print_log)
	{
		std::clog << "processing " << method << "\n";
		std::clog << "balls " << spheres.size() << "\n";

		std::clog << "parts " << distributed_ids.size() << " :";
		for(std::size_t i=0;i<distributed_ids.size();i++)
		{
			std::clog << " " << distributed_ids[i].size();
		}
		std::clog << "\n";

		std::clog << "quadruples " << result_quadruples_map.size() << "\n";
		std::clog << "tangent_spheres " << apollota::Triangulation::count_tangent_spheres_in_quadruples_map(result_quadruples_map) << "\n";

		std::clog << "efficiency " << result_quadruples_map.size() << "/" << sum_of_all_produced_quadruples_counts << "=" << (static_cast<double>(result_quadruples_map.size())/static_cast<double>(sum_of_all_produced_quadruples_counts)*100.0) << "%\n";
	}
}

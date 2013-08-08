#include <iostream>

#ifdef ENABLE_STD_THREAD
#include <thread>
#endif

#include "apollota/triangulation.h"
#include "apollota/splitting_set_of_spheres.h"

#include "modes_commons.h"

namespace
{

inline unsigned int estimate_splitting_depth(const unsigned int threads)
{
	unsigned int depth=0;
	while((static_cast<unsigned int>(1 << depth)<threads) && (depth<16))
	{
		depth++;
	}
	return depth;
}

template<typename T>
inline std::vector< std::vector<T> > distribute_objects_to_threads(const std::vector<T>& objects, const unsigned int threads)
{
	std::vector< std::vector<T> > result(threads);
	if(threads>0)
	{
		for(std::size_t i=0;i<objects.size();i++)
		{
			result[static_cast<unsigned int>(i)%threads].push_back(objects[i]);
		}
	}
	return result;
}

inline void run_thread_job(const apollota::BoundingSpheresHierarchy* bsh_ptr, const std::vector< std::vector<std::size_t> >* thread_ids_ptr, apollota::Triangulation::QuadruplesMap* result_quadruples_map_ptr)
{
	for(std::size_t j=0;j<thread_ids_ptr->size();j++)
	{
		apollota::Triangulation::merge_quadruples_maps(apollota::Triangulation::construct_result_for_admittance_set(*bsh_ptr, thread_ids_ptr->at(j)).quadruples_map, *result_quadruples_map_ptr);
	}
}

}

void calculate_triangulation_in_parallel(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--threads"].init("number", "number of threads");
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

	const unsigned int threads=poh.argument<double>("--threads");
	if(threads<1 || threads>1024)
	{
		throw std::runtime_error("Number of threads should be between in interval [1,1024].");
	}

	const unsigned int depth=estimate_splitting_depth(threads);

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

	const std::vector< std::vector< std::vector<std::size_t> > > distributed_ids=distribute_objects_to_threads(apollota::SplittingSetOfSpheres::split(spheres, depth), threads);

	const apollota::BoundingSpheresHierarchy bsh(spheres, init_radius_for_BSH, 1);

	apollota::Triangulation::QuadruplesMap result_quadruples_map;

	std::string parallelization_method_name="none";

#ifdef ENABLE_STD_THREAD
	{
		parallelization_method_name="std::thread";
		std::vector<std::thread> thread_handles;
		std::vector<apollota::Triangulation::QuadruplesMap> distributed_quadruples_maps(distributed_ids.size());
		for(std::size_t i=0;i<distributed_ids.size();i++)
		{
			thread_handles.push_back(std::thread(run_thread_job, &bsh, &distributed_ids[i], &distributed_quadruples_maps[i]));
		}
		for(std::size_t i=0;i<thread_handles.size();i++)
		{
			thread_handles[i].join();
			apollota::Triangulation::merge_quadruples_maps(distributed_quadruples_maps[i], result_quadruples_map);
		}
	}
#else
	{
		for(std::size_t i=0;i<distributed_ids.size();i++)
		{
			run_thread_job(&bsh, &distributed_ids[i], &result_quadruples_map);
		}
	}
#endif

	if(!skip_output)
	{
		apollota::Triangulation::print_quadruples_map(result_quadruples_map, std::cout);
	}

	if(print_log)
	{
		std::clog << "parallelization " << parallelization_method_name << "\n";
		std::clog << "balls " << spheres.size() << "\n";

		std::clog << "threads " << distributed_ids.size() << " :";
		for(std::size_t i=0;i<distributed_ids.size();i++)
		{
			const std::vector< std::vector<std::size_t> >& thread_ids=distributed_ids[i];
			std::clog << " (";
			for(std::size_t j=0;j<thread_ids.size();j++)
			{
				std::clog << " " << thread_ids[j].size();
			}
			std::clog << " )";
		}
		std::clog << "\n";

		std::clog << "quadruples " << result_quadruples_map.size() << "\n";
		std::clog << "tangent_spheres " << apollota::Triangulation::count_tangent_spheres_in_quadruples_map(result_quadruples_map) << "\n";
	}
}

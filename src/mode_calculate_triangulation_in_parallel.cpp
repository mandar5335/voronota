#include <iostream>

#ifdef ENABLE_MPI
#include <mpi.h>
#include <cstring>
#endif

#include "apollota/triangulation.h"

#include "modes_commons.h"

namespace
{

struct ParallelComputationResult
{
	ParallelComputationResult() : number_of_input_spheres(0), number_of_initialized_parts(0), number_of_produced_quadruples(0)
	{
	}

	std::size_t number_of_input_spheres;
	std::size_t number_of_initialized_parts;
	std::size_t number_of_produced_quadruples;
	apollota::Triangulation::QuadruplesMap merged_quadruples_map;
};

class ParallelComputationProcessingSimulated
{
public:
	static void process(
			const std::size_t parts,
			const double init_radius_for_BSH,
			ParallelComputationResult& result)
	{
		std::vector<apollota::SimpleSphere> spheres;
		auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
		result.number_of_input_spheres=spheres.size();

		const std::vector< std::vector<std::size_t> > distributed_ids=apollota::SplittingOfSpheres::split_for_number_of_parts(spheres, parts);
		result.number_of_initialized_parts=distributed_ids.size();

		const apollota::BoundingSpheresHierarchy bsh(spheres, init_radius_for_BSH, 1);

		for(std::size_t i=0;i<distributed_ids.size();i++)
		{
			const apollota::Triangulation::QuadruplesMap partial_quadruples_map=apollota::Triangulation::construct_result_for_admittance_set(bsh, distributed_ids[i]).quadruples_map;
			result.number_of_produced_quadruples+=partial_quadruples_map.size();
			apollota::Triangulation::merge_quadruples_maps(partial_quadruples_map, result.merged_quadruples_map);
		}
	}
};

#ifdef _OPENMP
class ParallelComputationProcessingWithOpenMP
{
public:
	static void process(
			const std::size_t parts,
			const double init_radius_for_BSH,
			ParallelComputationResult& result)
	{
		std::vector<apollota::SimpleSphere> spheres;
		auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
		result.number_of_input_spheres=spheres.size();

		const std::vector< std::vector<std::size_t> > distributed_ids=apollota::SplittingOfSpheres::split_for_number_of_parts(spheres, parts);
		result.number_of_initialized_parts=distributed_ids.size();

		const apollota::BoundingSpheresHierarchy bsh(spheres, init_radius_for_BSH, 1);

		std::vector<apollota::Triangulation::QuadruplesMap> distributed_quadruples_maps(distributed_ids.size());
		std::vector<int> distributed_errors(distributed_ids.size(), 0);

		{
	#pragma omp parallel for
			for(std::size_t i=0;i<distributed_ids.size();i++)
			{
				try
				{
					distributed_quadruples_maps[i]=apollota::Triangulation::construct_result_for_admittance_set(bsh, distributed_ids[i]).quadruples_map;
				}
				catch(...)
				{
					distributed_errors[i]=1;
				}
			}
		}

		{
			std::ostringstream errors_summary_stream;
			for(std::size_t i=0;i<distributed_errors.size();i++)
			{
				if(distributed_errors[i]!=0)
				{
					errors_summary_stream << " " << i;
				}
			}
			const std::string errors_summary=errors_summary_stream.str();
			if(!errors_summary.empty())
			{
				throw std::runtime_error("Parallel processing failed because of exceptions in parts:"+errors_summary+".");
			}
		}

		for(std::size_t i=0;i<distributed_quadruples_maps.size();i++)
		{
			result.number_of_produced_quadruples+=distributed_quadruples_maps[i].size();
			apollota::Triangulation::merge_quadruples_maps(distributed_quadruples_maps[i], result.merged_quadruples_map);
		}
	}
};
#endif

#ifdef ENABLE_MPI
class ParallelComputationProcessingWithMPI
{
public:
	static bool process(
			const std::vector<std::string>& argv,
			const std::size_t parts,
			const double init_radius_for_BSH,
			ParallelComputationResult& result)
	{
		MPIHandle mpi_handle(argv);

		if(mpi_handle.size()<=2)
		{
			ParallelComputationProcessingSimulated::process(parts, init_radius_for_BSH, result);
		}
		else
		{
			const int QUADRUPLES_MAP_DATA_TAG=1;

			std::vector<apollota::SimpleSphere> spheres;
			{
				std::vector<double> spheres_plain_vector;
				int spheres_plain_vector_length=0;
				if(mpi_handle.rank()==0)
				{
					auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
					fill_plain_vector_from_spheres(spheres, spheres_plain_vector);
					spheres_plain_vector_length=static_cast<int>(spheres_plain_vector.size());
				}
				MPI_Bcast(&spheres_plain_vector_length, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
				if(mpi_handle.rank()!=0)
				{
					spheres_plain_vector.resize(static_cast<std::size_t>(spheres_plain_vector_length));
				}
				MPI_Bcast(spheres_plain_vector.data(), spheres_plain_vector_length, MPI_DOUBLE, 0, MPI_COMM_WORLD);
				if(mpi_handle.rank()!=0)
				{
					fill_spheres_from_plain_vector(spheres_plain_vector, spheres);
				}
			}
			result.number_of_input_spheres=spheres.size();

			const std::vector< std::vector<std::size_t> > distributed_ids=apollota::SplittingOfSpheres::split_for_number_of_parts(spheres, parts);
			result.number_of_initialized_parts=distributed_ids.size();

			{
				std::vector<double> plain_vector;
				if(mpi_handle.rank()==0)
				{
					for(std::size_t i=0;i<distributed_ids.size();i++)
					{
						MPI_Status status;
						MPI_Probe(MPI_ANY_SOURCE, QUADRUPLES_MAP_DATA_TAG, MPI_COMM_WORLD, &status);
						int plain_vector_size=0;
						MPI_Get_count(&status, MPI_DOUBLE, &plain_vector_size);
						if(plain_vector_size>0)
						{
							plain_vector.resize(static_cast<std::size_t>(plain_vector_size));
							MPI_Recv(plain_vector.data(), plain_vector_size, MPI_DOUBLE, status.MPI_SOURCE, QUADRUPLES_MAP_DATA_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
							apollota::Triangulation::QuadruplesMap partial_quadruples_map;
							fill_quadruples_map_from_plain_vector(plain_vector, partial_quadruples_map);
							result.number_of_produced_quadruples+=partial_quadruples_map.size();
							apollota::Triangulation::merge_quadruples_maps(partial_quadruples_map, result.merged_quadruples_map);
						}
					}
				}
				else
				{
					const apollota::BoundingSpheresHierarchy bsh(spheres, init_radius_for_BSH, 1);
					for(std::size_t i=0;i<distributed_ids.size();i++)
					{
						if(mpi_handle.rank()==(static_cast<int>(i)%(mpi_handle.size()-1)+1))
						{
							fill_plain_vector_from_quadruples_map(apollota::Triangulation::construct_result_for_admittance_set(bsh, distributed_ids[i]).quadruples_map, plain_vector);
							MPI_Send(plain_vector.data(), static_cast<int>(plain_vector.size()), MPI_DOUBLE, 0, QUADRUPLES_MAP_DATA_TAG, MPI_COMM_WORLD);
						}
					}
				}
			}
		}

		mpi_handle.set_abort_on_destruction(false);
		return (mpi_handle.rank()==0);
	}

private:
	class MPIHandle
	{
	public:
		MPIHandle(const std::vector<std::string>& argv) :
			abort_on_destruction_(true),
			argc_(static_cast<int>(argv.size())),
			argv_(0),
			size_(0),
			rank_(0)
		{
			argv_=new char*[argc_];
			for(int i=0;i<argc_;i++)
			{
				argv_[i]=new char[argv[i].size()+1];
				strcpy(argv_[i], argv[i].c_str());
			}
			MPI_Init(&argc_, &argv_);
	        MPI_Comm_size(MPI_COMM_WORLD, &size_);
	        MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
		}

		~MPIHandle()
		{
			if(abort_on_destruction_)
			{
				MPI_Abort(MPI_COMM_WORLD, 1);
			}
			else
			{
				MPI_Finalize();
			}

			for(int i=0;i<argc_;i++)
			{
				delete[] argv_[i];
			}
			delete argv_;
		}

		void set_abort_on_destruction(const bool abort_on_destruction)
		{
			abort_on_destruction_=abort_on_destruction;
		}

		const int size() const
		{
			return size_;
		}

		const int rank() const
		{
			return rank_;
		}

	private:
		bool abort_on_destruction_;
		int argc_;
		char** argv_;
		int size_;
		int rank_;
	};

	static void fill_plain_vector_from_spheres(const std::vector<apollota::SimpleSphere>& spheres, std::vector<double>& plain_vector)
	{
		plain_vector.resize(spheres.size()*4);
		for(std::size_t i=0;i<spheres.size();i++)
		{
			plain_vector[i*4+0]=spheres[i].x;
			plain_vector[i*4+1]=spheres[i].y;
			plain_vector[i*4+2]=spheres[i].z;
			plain_vector[i*4+3]=spheres[i].r;
		}
	}

	static void fill_spheres_from_plain_vector(const std::vector<double>& plain_vector, std::vector<apollota::SimpleSphere>& spheres)
	{
		spheres.resize(plain_vector.size()/4);
		for(std::size_t i=0;i<spheres.size();i++)
		{
			spheres[i].x=plain_vector[i*4+0];
			spheres[i].y=plain_vector[i*4+1];
			spheres[i].z=plain_vector[i*4+2];
			spheres[i].r=plain_vector[i*4+3];
		}
	}

	static void fill_plain_vector_from_quadruples_map(const apollota::Triangulation::QuadruplesMap& quadruples_map, std::vector<double>& plain_vector)
	{
		plain_vector.resize(apollota::Triangulation::count_tangent_spheres_in_quadruples_map(quadruples_map)*8);
		std::size_t i=0;
		for(apollota::Triangulation::QuadruplesMap::const_iterator it=quadruples_map.begin();it!=quadruples_map.end();++it)
		{
			const apollota::Quadruple& q=it->first;
			const std::vector<apollota::SimpleSphere>& ts=it->second;
			for(std::size_t j=0;j<ts.size();j++)
			{
				plain_vector[i*8+0]=q.get(0);
				plain_vector[i*8+1]=q.get(1);
				plain_vector[i*8+2]=q.get(2);
				plain_vector[i*8+3]=q.get(3);
				plain_vector[i*8+4]=ts[j].x;
				plain_vector[i*8+5]=ts[j].y;
				plain_vector[i*8+6]=ts[j].z;
				plain_vector[i*8+7]=ts[j].r;
				i++;
			}
		}
	}

	static void fill_quadruples_map_from_plain_vector(const std::vector<double>& plain_vector, apollota::Triangulation::QuadruplesMap& quadruples_map)
	{
		quadruples_map.clear();
		const std::size_t n=(plain_vector.size()/8);
		for(std::size_t i=0;i<n;i++)
		{
			quadruples_map[apollota::Quadruple(
					static_cast<std::size_t>(plain_vector[i*8+0]+0.5),
					static_cast<std::size_t>(plain_vector[i*8+1]+0.5),
					static_cast<std::size_t>(plain_vector[i*8+2]+0.5),
					static_cast<std::size_t>(plain_vector[i*8+3]+0.5))].push_back(apollota::SimpleSphere(
							plain_vector[i*8+4],
							plain_vector[i*8+5],
							plain_vector[i*8+6],
							plain_vector[i*8+7]));
		}
	}
};
#endif

inline bool number_is_power_of_two(const unsigned long x)
{
	return ( (x>0) && ((x & (x-1))==0) );
}

}

void calculate_triangulation_in_parallel(const auxiliaries::ProgramOptionsHandler& poh)
{
	std::set<std::string> available_processing_method_names;
	{
		available_processing_method_names.insert("simulated");
#ifdef _OPENMP
		available_processing_method_names.insert("openmp");
#endif
#ifdef ENABLE_MPI
		available_processing_method_names.insert("mpi");
#endif
	}

	std::string available_processing_method_names_string;
	{
		std::ostringstream output;
		for(std::set<std::string>::const_iterator it=available_processing_method_names.begin();it!=available_processing_method_names.end();++it)
		{
			output << " '" << (*it) << "'";
		}
		available_processing_method_names_string=output.str();
	}

	{
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--method"].init("string", "processing method name, variants are:"+available_processing_method_names_string, true);
		basic_map_of_option_descriptions["--parts"].init("number", "number of parts for splitting, must be power of 2", true);
		basic_map_of_option_descriptions["--print-log"].init("", "flag to print log of calculations");
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions full_map_of_option_descriptions=basic_map_of_option_descriptions;
		full_map_of_option_descriptions["--skip-output"].init("", "flag to disable output of the resulting triangulation");
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
		throw std::runtime_error("Invalid processing method name, acceptable values are:"+available_processing_method_names_string+".");
	}

	const std::size_t parts=poh.argument<std::size_t>("--parts");
	if(!number_is_power_of_two(parts))
	{
		throw std::runtime_error("Number of parts must be power of 2.");
	}

	const bool skip_output=poh.contains_option("--skip-output");

	const bool print_log=poh.contains_option("--print-log");

	const double init_radius_for_BSH=poh.argument<double>("--init-radius-for-BSH", 3.5);
	if(init_radius_for_BSH<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy initial radius should be greater than 1.");
	}

	ParallelComputationResult result;
	bool master_finished=true;

	if(method=="simulated")
	{
		ParallelComputationProcessingSimulated::process(parts, init_radius_for_BSH, result);
	}
#ifdef _OPENMP
	else if(method=="openmp")
	{
		ParallelComputationProcessingWithOpenMP::process(parts, init_radius_for_BSH, result);
	}
#endif
#ifdef ENABLE_MPI
	else if(method=="mpi")
	{
		master_finished=ParallelComputationProcessingWithMPI::process(poh.unused_argv(), parts, init_radius_for_BSH, result);
	}
#endif
	else
	{
		throw std::runtime_error("Processing method '"+method+"' is not available.");
	}

	if(master_finished)
	{
		if(!skip_output)
		{
			apollota::Triangulation::print_quadruples_map(result.merged_quadruples_map, std::cout);
		}

		if(print_log)
		{
			std::clog << "balls " << result.number_of_input_spheres << "\n";
			std::clog << "parts " << result.number_of_initialized_parts << "\n";
			std::clog << "quadruples " << result.merged_quadruples_map.size() << "\n";
			std::clog << "tangent_spheres " << apollota::Triangulation::count_tangent_spheres_in_quadruples_map(result.merged_quadruples_map) << "\n";
			std::clog << "parallel_results_overlap " << (result.merged_quadruples_map.empty() ? static_cast<double>(0) : (static_cast<double>(result.number_of_produced_quadruples)/static_cast<double>(result.merged_quadruples_map.size()))) << "\n";
		}
	}
}

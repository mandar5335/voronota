#include <iostream>
#include <functional>
#include <exception>
#include <limits>

#include "apollota/safe_comparison_of_numbers.h"

#include "auxiliaries/command_line_options.h"
#include "auxiliaries/clog_redirector.h"

void calculate_triangulation(const auxiliaries::CommandLineOptions& clo);
void compare_triangulations(const auxiliaries::CommandLineOptions& clo);
void get_balls_from_pdb_file(const auxiliaries::CommandLineOptions& clo);

int main(const int argc, const char** argv)
{
	std::cin.exceptions(std::istream::badbit);
	std::cout.exceptions(std::ostream::badbit);
	std::ios_base::sync_with_stdio(false);

	std::string mode;

	try
	{
		auxiliaries::CommandLineOptions clo(argc, argv);

		mode=clo.isopt("--mode") ? clo.arg<std::string>("--mode") : std::string("");
		clo.remove_option("--mode");

		const std::string clog_filename=clo.isopt("--clog-file") ? clo.arg<std::string>("--clog-file") : std::string("");
		clo.remove_option("--clog-file");

		auxiliaries::CLogRedirector clog_redirector;
		if(!clog_filename.empty() && !clog_redirector.init(clog_filename))
		{
			std::cerr << "Failed to redirect clog to file: " << clog_filename << "." << std::endl;
			return 1;
		}

		if(clo.isopt("--epsilon"))
		{
			const double epsilon=clo.arg<double>("--epsilon");
			if(epsilon<=0.0)
			{
				std::cerr << "Provided epsilon is invalid, it needs to be greater than 0.\n";
				return 1;
			}
			apollota::comparison_epsilon_reference()=epsilon;
		}
		clo.remove_option("--epsilon");

		typedef std::pointer_to_unary_function<const auxiliaries::CommandLineOptions&, void> ModeFunctionPointer;
		std::map< std::string, ModeFunctionPointer > modes_map;

		modes_map["calculate-triangulation"]=ModeFunctionPointer(calculate_triangulation);
		modes_map["compare-triangulations"]=ModeFunctionPointer(compare_triangulations);
		modes_map["get-balls-from-pdb-file"]=ModeFunctionPointer(get_balls_from_pdb_file);

		if(modes_map.count(mode)==1)
		{
			modes_map.find(mode)->second(clo);
		}
		else
		{
			std::cerr << "Unspecified running mode. Available modes are:" << std::endl;
			for(std::map< std::string, ModeFunctionPointer >::const_iterator it=modes_map.begin();it!=modes_map.end();++it)
			{
				std::cerr << "  --mode " << it->first << std::endl;
			}
			return 1;
		}
	}
	catch(const std::exception& e)
	{
		if(mode.empty())
		{
			std::cerr << "Exception was caught: ";
		}
		else
		{
			std::cerr << "Operation '" << mode << "' was not successful because exception was caught: ";
		}
		std::cerr << "[" << (e.what()) << "]." << std::endl;
		return 1;
	}
	catch(...)
	{
		std::cerr << "Unknown exception caught." << std::endl;
		return 1;
	}

	std::cin.ignore(std::numeric_limits<std::streamsize>::max());

	return 0;
}

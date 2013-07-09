#include <iostream>
#include <functional>
#include <exception>
#include <limits>

#include "apollota/safe_comparison_of_numbers.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/clog_redirector.h"

void calculate_triangulation(const auxiliaries::ProgramOptionsHandler& poh);
void compare_triangulations(const auxiliaries::ProgramOptionsHandler& poh);
void get_balls_from_pdb_file(const auxiliaries::ProgramOptionsHandler& poh);

int main(const int argc, const char** argv)
{
	typedef std::pointer_to_unary_function<const auxiliaries::ProgramOptionsHandler&, void> ModeFunctionPointer;
	typedef std::map<std::string, ModeFunctionPointer> ModesMap;

	std::cin.exceptions(std::istream::badbit);
	std::cout.exceptions(std::ostream::badbit);
	std::ios_base::sync_with_stdio(false);

	try
	{
		ModesMap modes_map;
		modes_map["calculate-triangulation"]=ModeFunctionPointer(calculate_triangulation);
		modes_map["compare-triangulations"]=ModeFunctionPointer(compare_triangulations);
		modes_map["get-balls-from-pdb-file"]=ModeFunctionPointer(get_balls_from_pdb_file);

		auxiliaries::ProgramOptionsHandler poh(argc, argv);

		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions basic_map_of_option_descriptions;
		basic_map_of_option_descriptions["--mode"].init("string", "running mode", true);
		basic_map_of_option_descriptions["--help"].init("", "flag to print usage help");
		auxiliaries::ProgramOptionsHandler::MapOfOptionDescriptions full_map_of_option_descriptions=basic_map_of_option_descriptions;
		full_map_of_option_descriptions["--clog-file"].init("string", "path to file for log stream redirection");
		full_map_of_option_descriptions["--epsilon"].init("number", "threshold for floating-point numbers comparison");
		full_map_of_option_descriptions["--help-full"].init("", "flag to print full usage help");

		if(!poh.contains_option("--mode") || poh.contains_option("--help") || poh.contains_option("--help-full"))
		{
			poh.set_option("--help");

			std::cerr << "\nCommon options\n\n";
			auxiliaries::ProgramOptionsHandler::print_map_of_option_descriptions(poh.contains_option("--help-full") ? full_map_of_option_descriptions : basic_map_of_option_descriptions, std::cerr);
			std::cerr << "\n\n";
			for(ModesMap::const_iterator it=modes_map.begin();it!=modes_map.end();++it)
			{
				std::cerr << "--mode " << it->first << "\n\n";
				it->second(poh);
				std::cerr << "\n\n";
			}
			std::cerr << std::endl;

			return 1;
		}
		else
		{
			poh.compare_with_map_of_option_descriptions(full_map_of_option_descriptions, true);

			const std::string mode=poh.argument<std::string>("--mode");
			poh.remove_option("--mode");
			if(modes_map.count(mode)==0)
			{
				std::ostringstream msg;
				msg << "Invalid mode. Available modes are:\n";
				for(ModesMap::const_iterator it=modes_map.begin();it!=modes_map.end();++it)
				{
					msg << "  --mode " << it->first << "\n";
				}
				throw std::runtime_error(msg.str());
			}

			const std::string clog_filename=poh.argument<std::string>("--clog-file", "");
			poh.remove_option("--clog-file");
			auxiliaries::CLogRedirector clog_redirector;
			if(!clog_filename.empty() && !clog_redirector.init(clog_filename))
			{
				std::ostringstream msg;
				msg << "Failed to redirect clog to file '" << clog_filename << "'.";
				throw std::runtime_error(msg.str());
			}

			const double epsilon=poh.argument<double>("--epsilon", -1.0);
			poh.remove_option("--epsilon");
			if(epsilon>0.0)
			{
				apollota::comparison_epsilon_reference()=epsilon;
			}

			modes_map.find(mode)->second(poh);

			return 0;
		}
	}
	catch(const auxiliaries::ProgramOptionsHandler::Exception& e)
	{
		std::cerr << "\nInvalid parameters: " << (e.what()) << "\n";
		std::cerr << std::endl;
	}
	catch(const std::exception& e)
	{
		std::cerr << "\nException caught: " << (e.what()) << "\n";
		std::cerr << std::endl;
	}
	catch(...)
	{
		std::cerr << "\nUnknown exception caught.\n";
		std::cerr << std::endl;
	}

	return 2;
}

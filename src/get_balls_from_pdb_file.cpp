#include <iostream>
#include <fstream>
#include <sstream>

#include "auxiliaries/pdb_file_parsing.h"
#include "auxiliaries/atom_radius_assigner.h"
#include "auxiliaries/command_line_options.h"

void get_balls_from_pdb_file(const auxiliaries::CommandLineOptions& clo)
{
	clo.check_allowed_options("--include-heteroatoms --default-radius: --only-default-radius --radii-file: --output-serial-numbers");

	const bool include_heteroatoms=clo.isopt("--include-heteroatoms");
	const double default_radius=clo.isarg("--default-radius") ? clo.arg<double>("--default-radius") : 1.70;
	const bool only_default_radius=clo.isopt("--only-default-radius");
	const std::string radii_file=clo.isarg("--radii-file") ? clo.arg<std::string>("--radii-file") : std::string();
	const bool output_serial_numbers=clo.isopt("--output-serial-numbers");

	const std::vector<auxiliaries::PDBFileParsing::AtomRecord> atoms=auxiliaries::PDBFileParsing::read_atom_records_from_pdb_file_stream(std::cin, include_heteroatoms);

	auxiliaries::AtomRadiusAssigner atom_radius_assigner(default_radius);
	if(!only_default_radius)
	{
		if(radii_file.empty())
		{
			atom_radius_assigner.add_radius_by_descriptor("*", "C*", 1.70);
			atom_radius_assigner.add_radius_by_descriptor("*", "N*", 1.55);
			atom_radius_assigner.add_radius_by_descriptor("*", "O*", 1.52);
			atom_radius_assigner.add_radius_by_descriptor("*", "P*", 1.80);
		}
		else
		{
			std::ifstream input(radii_file.c_str(), std::ios::in);
			while(input.good())
			{
				std::string line;
				std::getline(input, line);
				if(!line.empty())
				{
					std::istringstream line_input(line);
					std::string resName;
					std::string name;
					double radius=default_radius;
					line_input >> resName >> name >> radius;
					if(!line_input.fail())
					{
						atom_radius_assigner.add_radius_by_descriptor(resName, name, radius);
					}
				}
			}
		}
	}

	for(std::size_t i=0;i<atoms.size();i++)
	{
		const auxiliaries::PDBFileParsing::AtomRecord& atom=atoms[i];
		std::cout << atom.x << " " << atom.y << " " << atom.z << " " << atom_radius_assigner.get_atom_radius(atom.resName, atom.name);
		if(output_serial_numbers)
		{
			std::cout << " " << atom.serial;
		}
		std::cout << "\n";
	}
}

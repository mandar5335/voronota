#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cmath>

#include "modescommon/assert_options.h"
#include "modescommon/handle_annotations.h"
#include "modescommon/handle_mappings.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

struct EnergyDescriptor
{
	double total_area;
	double strange_area;
	double energy;

	EnergyDescriptor() : total_area(0), strange_area(0), energy(0)
	{
	}

	void add(const EnergyDescriptor& ed)
	{
		total_area+=ed.total_area;
		strange_area+=ed.strange_area;
		energy+=ed.energy;
	}
};

struct EnergyScore
{
	double normalized_energy;
	double energy_score;
	double actuality_score;
	double quality_score;

	EnergyScore() : normalized_energy(0.0), energy_score(0.0), actuality_score(0.0), quality_score(0.0)
	{
	}
};

struct EnergyScoreCalculationParameter
{
	double erf_mean;
	double erf_sd;

	EnergyScoreCalculationParameter(const double erf_mean, const double erf_sd) : erf_mean(erf_mean), erf_sd(erf_sd)
	{
	}
};

inline EnergyScore calculate_energy_score_from_energy_descriptor(const EnergyDescriptor& ed, const EnergyScoreCalculationParameter& escp)
{
	static const double square_root_of_two=sqrt(2.0);
	EnergyScore es;
	if(ed.total_area>0.0)
	{
		es.normalized_energy=ed.energy/ed.total_area;
		es.energy_score=1.0-(0.5*(1.0+erf((es.normalized_energy-escp.erf_mean)/(square_root_of_two*escp.erf_sd))));
		es.actuality_score=1.0-(ed.strange_area/ed.total_area);
		es.quality_score=(es.energy_score*es.actuality_score);
	}
	return es;
}

inline void print_score(const std::string& name, const EnergyDescriptor& ed, const EnergyScoreCalculationParameter& escp, std::ostream& output)
{
	const EnergyScore es=calculate_energy_score_from_energy_descriptor(ed, escp);
	output << name << " ";
	output << es.quality_score << " " << es.normalized_energy << " " << es.energy_score << " " << es.actuality_score << " ";
	output << ed.total_area << " " << ed.strange_area << " " << ed.energy << "\n";
}

void print_pair_scores_to_file(const std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >& map_of_pair_energy_descriptors, const EnergyScoreCalculationParameter& escp, const std::string& filename)
{
	if(!filename.empty())
	{
		std::ofstream foutput(filename.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=map_of_pair_energy_descriptors.begin();it!=map_of_pair_energy_descriptors.end();++it)
			{
				print_score(it->first.first.str()+" "+it->first.second.str(), it->second, escp, foutput);
			}
		}
	}
}

void print_single_scores_to_file(const std::map<CRAD, EnergyDescriptor>& map_of_single_energy_descriptors, const EnergyScoreCalculationParameter& escp, const std::string& filename)
{
	if(!filename.empty())
	{
		std::ofstream foutput(filename.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map<CRAD, EnergyDescriptor>::const_iterator it=map_of_single_energy_descriptors.begin();it!=map_of_single_energy_descriptors.end();++it)
			{
				print_score(it->first.str(), it->second, escp, foutput);
			}
		}
	}
}

void print_single_scores_summary(const std::string& name, const std::map<CRAD, EnergyDescriptor>& map_of_single_energy_descriptors, const EnergyScoreCalculationParameter& escp, std::ostream& output)
{
	EnergyScore es_sum;
	for(std::map<CRAD, EnergyDescriptor>::const_iterator it=map_of_single_energy_descriptors.begin();it!=map_of_single_energy_descriptors.end();++it)
	{
		const EnergyDescriptor& ed=it->second;
		const EnergyScore es=calculate_energy_score_from_energy_descriptor(ed, escp);
		es_sum.quality_score+=es.quality_score;
		es_sum.normalized_energy+=es.normalized_energy;
		es_sum.energy_score+=es.energy_score;
		es_sum.actuality_score+=es.actuality_score;
	}
	output << name << " " << map_of_single_energy_descriptors.size() << " ";
	output << es_sum.quality_score << " " << es_sum.normalized_energy << " " << es_sum.energy_score << " " << es_sum.actuality_score << "\n";
}

}

void score_contacts_potential(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--output-summed-areas", "", "flag to output summed areas instead of potential values"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 area')\n";
			std::cerr << "stdout  ->  list of potential values (line format: 'annotation1 annotation2 value')\n";
			return;
		}
	}

	const bool output_summed_areas=poh.contains_option("--output-summed-areas");

	std::map< std::pair<CRAD, CRAD>, double > map_of_total_areas;
	auxiliaries::read_lines_to_container(std::cin, modescommon::add_chain_residue_atom_descriptors_pair_value_from_stream_to_map<true>, map_of_total_areas);
	if(map_of_total_areas.empty())
	{
		throw std::runtime_error("No contacts input.");
	}

	std::map<CRAD, double> map_of_generalized_total_areas;
	double sum_of_all_areas=0.0;
	for(std::map< std::pair<CRAD, CRAD>, double >::iterator it=map_of_total_areas.begin();it!=map_of_total_areas.end();++it)
	{
		const std::pair<CRAD, CRAD>& crads=it->first;
		map_of_generalized_total_areas[crads.first]+=(it->second);
		map_of_generalized_total_areas[crads.second]+=(it->second);
		sum_of_all_areas+=(it->second);
	}

	for(std::map< std::pair<CRAD, CRAD>, double >::iterator it=map_of_total_areas.begin();it!=map_of_total_areas.end();++it)
	{
		const std::pair<CRAD, CRAD>& crads=it->first;
		const double ab=it->second;
		if(output_summed_areas)
		{
			std::cout << crads.first.str() << " " << crads.second.str() << " " << ab << "\n";
		}
		else
		{
			const double ax=map_of_generalized_total_areas[crads.first];
			const double bx=map_of_generalized_total_areas[crads.second];
			if(ab>0.0 && ax>0.0 && bx>0.0)
			{
				const double potential_value=(0.0-log((ab*sum_of_all_areas)/(ax*bx)));
				std::cout << crads.first.str() << " " << crads.second.str() << " " << potential_value << "\n";
			}
		}
	}
}

void score_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--potential-file", "string", "file path to input potential values", true));
		list_of_option_descriptions.push_back(OD("--inter-atom-scores-file", "string", "file path to output inter-atom scores"));
		list_of_option_descriptions.push_back(OD("--inter-residue-scores-file", "string", "file path to output inter-residue scores"));
		list_of_option_descriptions.push_back(OD("--atom-scores-file", "string", "file path to output atom scores"));
		list_of_option_descriptions.push_back(OD("--residue-scores-file", "string", "file path to output residue scores"));
		list_of_option_descriptions.push_back(OD("--depth", "number", "neighborhood normalization depth"));
		list_of_option_descriptions.push_back(OD("--erf-mean", "number", "mean parameter for error function"));
		list_of_option_descriptions.push_back(OD("--erf-sd", "number", "sd parameter for error function"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 area')\n";
			std::cerr << "stdout  ->  line of global scores\n";
			return;
		}
	}

	const std::string potential_file=poh.argument<std::string>("--potential-file");
	const std::string inter_atom_scores_file=poh.argument<std::string>("--inter-atom-scores-file", "");
	const std::string inter_residue_scores_file=poh.argument<std::string>("--inter-residue-scores-file", "");
	const std::string atom_scores_file=poh.argument<std::string>("--atom-scores-file", "");
	const std::string residue_scores_file=poh.argument<std::string>("--residue-scores-file", "");
	const int depth=poh.argument<int>("--depth", 1);
	const double erf_mean=poh.argument<double>("--erf-mean", 0.3);
	const double erf_sd=poh.argument<double>("--erf-sd", 0.2);

	const EnergyScoreCalculationParameter escp(erf_mean, erf_sd);

	std::map< std::pair<CRAD, CRAD>, double > map_of_contacts;
	{
		auxiliaries::read_lines_to_container(std::cin, modescommon::add_chain_residue_atom_descriptors_pair_value_from_stream_to_map<false>, map_of_contacts);
		if(map_of_contacts.empty())
		{
			throw std::runtime_error("No contacts input.");
		}
	}

	std::map< std::pair<CRAD, CRAD>, double > map_of_potential_values;
	{
		if(!potential_file.empty())
		{
			std::ifstream finput(potential_file.c_str(), std::ios::in);
			if(finput.good())
			{
				auxiliaries::read_lines_to_container(finput, modescommon::add_chain_residue_atom_descriptors_pair_value_from_stream_to_map<false>, map_of_potential_values);
			}
		}
		if(map_of_potential_values.empty())
		{
			throw std::runtime_error("No potential values input.");
		}
	}

	std::map< std::pair<CRAD, CRAD>, EnergyDescriptor > inter_atom_energy_descriptors;
	{
		for(std::map< std::pair<CRAD, CRAD>, double >::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			EnergyDescriptor& ed=inter_atom_energy_descriptors[crads];
			ed.total_area=it->second;
			std::map< std::pair<CRAD, CRAD>, double >::const_iterator potential_value_it=
					map_of_potential_values.find(modescommon::refine_pair_by_ordering(std::make_pair(crads.first.without_numbering(), crads.second.without_numbering())));
			if(potential_value_it!=map_of_potential_values.end())
			{
				ed.energy=ed.total_area*(potential_value_it->second);
			}
			else
			{
				ed.strange_area=ed.total_area;
			}
		}
		print_pair_scores_to_file(inter_atom_energy_descriptors, escp, inter_atom_scores_file);
	}

	std::map< std::pair<CRAD, CRAD>, EnergyDescriptor > inter_residue_energy_descriptors;
	{
		for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			inter_residue_energy_descriptors[modescommon::refine_pair_by_ordering(std::make_pair(crads.first.without_atom(), crads.second.without_atom()))].add(it->second);
		}
		print_pair_scores_to_file(inter_residue_energy_descriptors, escp, inter_residue_scores_file);
	}

	const std::map<CRAD, EnergyDescriptor> atom_energy_descriptors=modescommon::construct_single_mapping_of_descriptors_from_pair_mapping_of_descriptors(inter_atom_energy_descriptors, depth);
	print_single_scores_to_file(atom_energy_descriptors, escp, atom_scores_file);

	const std::map<CRAD, EnergyDescriptor> residue_energy_descriptors=modescommon::construct_single_mapping_of_descriptors_from_pair_mapping_of_descriptors(inter_residue_energy_descriptors, depth);
	print_single_scores_to_file(residue_energy_descriptors, escp, residue_scores_file);

	{
		EnergyDescriptor global_ed;
		for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
		{
			global_ed.add(it->second);
		}
		print_score("global", global_ed, escp, std::cout);
	}

	print_single_scores_summary("atom_level_summary", atom_energy_descriptors, escp, std::cout);
	print_single_scores_summary("residue_level_summary", residue_energy_descriptors, escp, std::cout);
}

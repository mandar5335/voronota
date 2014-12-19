#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cmath>

#include "modescommon/assert_options.h"
#include "modescommon/handle_annotations.h"
#include "modescommon/handle_mappings.h"
#include "modescommon/handle_sequences.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

struct EnergyDescriptor
{
	double total_area;
	double strange_area;
	double energy;
	int contacts_count;

	EnergyDescriptor() : total_area(0), strange_area(0), energy(0), contacts_count(0)
	{
	}

	void add(const EnergyDescriptor& ed)
	{
		total_area+=ed.total_area;
		strange_area+=ed.strange_area;
		energy+=ed.energy;
		contacts_count+=ed.contacts_count;
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

	EnergyScoreCalculationParameter(
			const double erf_mean,
			const double erf_sd) :
				erf_mean(erf_mean),
				erf_sd(erf_sd)
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

inline void print_score(const std::string& name, const EnergyDescriptor& ed, const EnergyScoreCalculationParameter& escp, const bool detailed, std::ostream& output)
{
	const EnergyScore es=calculate_energy_score_from_energy_descriptor(ed, escp);
	output << name << " " << es.quality_score;
	if(detailed)
	{
		output << " " << es.normalized_energy << " " << es.energy_score << " " << es.actuality_score << " ";
		output << ed.total_area << " " << ed.strange_area << " " << ed.energy << " " << ed.contacts_count;
	}
	output << "\n";
}

void print_pair_scores_to_file(const std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >& map_of_pair_energy_descriptors, const EnergyScoreCalculationParameter& escp, const bool detailed, const std::string& filename)
{
	if(!filename.empty())
	{
		std::ofstream foutput(filename.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=map_of_pair_energy_descriptors.begin();it!=map_of_pair_energy_descriptors.end();++it)
			{
				print_score(it->first.first.str()+" "+it->first.second.str(), it->second, escp, detailed, foutput);
			}
		}
	}
}

void print_single_scores_to_file(const std::map<CRAD, EnergyDescriptor>& map_of_single_energy_descriptors, const EnergyScoreCalculationParameter& escp, const bool detailed, const std::string& filename)
{
	if(!filename.empty())
	{
		std::ofstream foutput(filename.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map<CRAD, EnergyDescriptor>::const_iterator it=map_of_single_energy_descriptors.begin();it!=map_of_single_energy_descriptors.end();++it)
			{
				print_score(it->first.str(), it->second, escp, detailed, foutput);
			}
		}
	}
}

void print_single_scores_summary(const std::string& name, const std::map<CRAD, EnergyDescriptor>& map_of_single_energy_descriptors, const std::size_t reference_size, const EnergyScoreCalculationParameter& escp, const bool detailed, std::ostream& output)
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
	output << name << " " << (es_sum.quality_score/static_cast<double>(std::max(reference_size, map_of_single_energy_descriptors.size())));
	if(detailed)
	{
		output << " " << reference_size << " " << map_of_single_energy_descriptors.size() << " " << es_sum.quality_score << " " << es_sum.normalized_energy << " " << es_sum.energy_score << " " << es_sum.actuality_score;
	}
	output << "\n";
}

template<typename T>
std::map< CRAD, T > inject_residue_scores_into_target_sequence(const std::string& target_sequence, const std::map< CRAD, T >& residue_scores)
{
	std::map< CRAD, T > result;
	for(std::size_t i=0;i<target_sequence.size();i++)
	{
		CRAD crad;
		crad.resSeq=static_cast<int>(i+1);
		crad.resName=auxiliaries::ResidueLettersCoding::convert_residue_code_small_to_big(std::string(1, target_sequence[i]));
		result[crad]=T();
	}
	for(typename std::map< CRAD, T >::const_iterator it=residue_scores.begin();it!=residue_scores.end();++it)
	{
		CRAD crad;
		crad.resSeq=it->first.resSeq;
		crad.resName=it->first.resName;
		typename std::map< CRAD, T >::iterator jt=result.find(crad);
		if(jt!=result.end())
		{
			jt->second=it->second;
		}
	}
	return result;
}

std::map<CRAD, double> smooth_residue_scores_along_sequence(const std::map<CRAD, double>& raw_scores, const unsigned int window)
{
	std::vector< std::pair<CRAD, double> > v(raw_scores.size());
	std::copy(raw_scores.begin(), raw_scores.end(), v.begin());
	std::vector< std::pair<CRAD, double> > sv=v;
	for(std::size_t i=0;i<v.size();i++)
	{
		const int start=std::max(0, (static_cast<int>(i)-static_cast<int>(window)));
		const int end=std::min(static_cast<int>(v.size())-1, (static_cast<int>(i)+static_cast<int>(window)));
		double sum_of_weighted_values=0.0;
		double sum_of_weights=0.0;
		for(int j=start;j<=end;j++)
		{
			if(v[i].first.chainID==v[j].first.chainID)
			{
				double ndist=fabs(static_cast<double>(static_cast<int>(i)-j))/static_cast<double>(window);
				double weight=(1.0-(ndist*ndist));
				sum_of_weights+=weight;
				sum_of_weighted_values+=v[j].second*weight;
			}
		}
		if(sum_of_weights>0.0)
		{
			sv[i].second=(sum_of_weighted_values/sum_of_weights);
		}
	}
	return std::map<CRAD, double>(sv.begin(), sv.end());
}

}

void score_contacts_potential(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--potential-file", "string", "file path to output potential values"));
		list_of_option_descriptions.push_back(OD("--solvent-factor", "number", "solvent factor value"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 area')\n";
			std::cerr << "stdout  ->  line of contact type area summaries (line format: 'annotation1 annotation2 area')\n";
			return;
		}
	}

	const std::string potential_file=poh.argument<std::string>("--potential-file", "");
	const double solvent_factor=poh.argument<double>("--solvent-factor", 1.0);

	std::map< std::pair<CRAD, CRAD>, double > map_of_considered_total_areas;
	std::map<CRAD, double> map_of_generalized_total_areas;
	double sum_of_all_areas=0.0;

	while(std::cin.good())
	{
		std::string line;
		std::getline(std::cin, line);
		if(!line.empty())
		{
			std::istringstream input(line);
			if(input.good())
			{
				std::pair<std::string, std::string> crads_strings;
				double area=0.0;
				input >> crads_strings.first >> crads_strings.second >> area;
				if(!input.fail() && !crads_strings.first.empty() && !crads_strings.second.empty())
				{
					const std::pair<CRAD, CRAD> crads(CRAD::from_str(crads_strings.first), CRAD::from_str(crads_strings.second));
					if(crads.first.valid() && crads.second.valid())
					{
						const std::pair<CRAD, CRAD> crads_without_numbering=modescommon::refine_pair_by_ordering(std::make_pair(crads.first.without_numbering(), crads.second.without_numbering()));
						map_of_considered_total_areas[crads_without_numbering]+=area;
						map_of_generalized_total_areas[crads_without_numbering.first]+=area;
						map_of_generalized_total_areas[crads_without_numbering.second]+=area;
						sum_of_all_areas+=area;
					}
				}
			}
		}
	}

	if(solvent_factor>0.0)
	{
		std::map<CRAD, double>::iterator it=map_of_generalized_total_areas.find(CRAD::solvent());
		if(it!=map_of_generalized_total_areas.end())
		{
			const double additional_area=solvent_factor*(it->second);
			it->second+=additional_area;
			sum_of_all_areas+=additional_area;
		}
	}

	std::map< std::pair<CRAD, CRAD>, std::pair<double, double> > result;
	for(std::map< std::pair<CRAD, CRAD>, double >::const_iterator it=map_of_considered_total_areas.begin();it!=map_of_considered_total_areas.end();++it)
	{
		const std::pair<CRAD, CRAD>& crads=it->first;
		const double ab=it->second;
		const double ax=map_of_generalized_total_areas[crads.first];
		const double bx=map_of_generalized_total_areas[crads.second];
		if(ab>0.0 && ax>0.0 && bx>0.0)
		{
			const double potential_value=(0.0-log((ab*sum_of_all_areas)/(ax*bx)));
			result[crads]=std::make_pair(potential_value, ab);
		}
	}

	if(!potential_file.empty())
	{
		std::ofstream foutput(potential_file.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map< std::pair<CRAD, CRAD>, std::pair<double, double> >::const_iterator it=result.begin();it!=result.end();++it)
			{
				const std::pair<CRAD, CRAD>& crads=it->first;
				foutput << crads.first.str() << " " << crads.second.str() << " " << it->second.first << "\n";
			}
		}
	}

	for(std::map< std::pair<CRAD, CRAD>, std::pair<double, double> >::const_iterator it=result.begin();it!=result.end();++it)
	{
		const std::pair<CRAD, CRAD>& crads=it->first;
		std::cout << crads.first.str() << " " << crads.second.str() << " " << it->second.second << "\n";
	}
}

void score_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--potential-file", "string", "file path to input potential values", true));
		list_of_option_descriptions.push_back(OD("--ignorable-max-seq-sep", "number", "maximum residue sequence separation for ignorable contacts"));
		list_of_option_descriptions.push_back(OD("--inter-atom-scores-file", "string", "file path to output inter-atom scores"));
		list_of_option_descriptions.push_back(OD("--inter-residue-scores-file", "string", "file path to output inter-residue scores"));
		list_of_option_descriptions.push_back(OD("--atom-scores-file", "string", "file path to output atom scores"));
		list_of_option_descriptions.push_back(OD("--residue-scores-file", "string", "file path to output residue scores"));
		list_of_option_descriptions.push_back(OD("--residue-atomic-scores-file", "string", "file path to output residue atom average scores"));
		list_of_option_descriptions.push_back(OD("--smoothing-window", "number", "window to smooth residue atom average scores along sequence"));
		list_of_option_descriptions.push_back(OD("--depth", "number", "neighborhood normalization depth"));
		list_of_option_descriptions.push_back(OD("--erf-mean", "number", "mean parameter for error function"));
		list_of_option_descriptions.push_back(OD("--erf-sd", "number", "sd parameter for error function"));
		list_of_option_descriptions.push_back(OD("--reference-sequence-file", "string", "file path to input reference sequence for normalization"));
		list_of_option_descriptions.push_back(OD("--residue-scores-to-ref-seq", "", "flag to align residue scores to reference sequence"));
		list_of_option_descriptions.push_back(OD("--detailed-output", "", "flag to enable detailed output"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 area')\n";
			std::cerr << "stdout  ->  global scores\n";
			return;
		}
	}

	const std::string potential_file=poh.argument<std::string>("--potential-file");
	const int ignorable_max_seq_sep=poh.argument<int>("--ignorable-max-seq-sep", 1);
	const std::string inter_atom_scores_file=poh.argument<std::string>("--inter-atom-scores-file", "");
	const std::string inter_residue_scores_file=poh.argument<std::string>("--inter-residue-scores-file", "");
	const std::string atom_scores_file=poh.argument<std::string>("--atom-scores-file", "");
	const std::string residue_scores_file=poh.argument<std::string>("--residue-scores-file", "");
	const std::string residue_atomic_scores_file=poh.argument<std::string>("--residue-atomic-scores-file", "");
	const unsigned int smoothing_window=poh.argument<unsigned int>("--smoothing-window", 0);
	const int depth=poh.argument<int>("--depth", 1);
	const double erf_mean=poh.argument<double>("--erf-mean", 0.4);
	const double erf_sd=poh.argument<double>("--erf-sd", 0.3);
	const std::string reference_sequence_file=poh.argument<std::string>("--reference-sequence-file", "");
	const bool residue_scores_to_ref_seq=poh.contains_option("--residue-scores-to-ref-seq");
	const bool detailed_output=poh.contains_option("--detailed-output");

	const EnergyScoreCalculationParameter escp(erf_mean, erf_sd);

	std::map< std::pair<CRAD, CRAD>, double > map_of_contacts;
	{
		auxiliaries::read_lines_to_container(std::cin, modescommon::add_chain_residue_atom_descriptors_pair_value_from_stream_to_map, map_of_contacts);
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
				auxiliaries::read_lines_to_container(finput, modescommon::add_chain_residue_atom_descriptors_pair_value_from_stream_to_map, map_of_potential_values);
			}
		}
		if(map_of_potential_values.empty())
		{
			throw std::runtime_error("No potential values input.");
		}
	}

	const std::string reference_sequence=modescommon::read_sequence_from_file(reference_sequence_file);

	std::map< std::pair<CRAD, CRAD>, EnergyDescriptor > inter_atom_energy_descriptors;
	{
		for(std::map< std::pair<CRAD, CRAD>, double >::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			EnergyDescriptor& ed=inter_atom_energy_descriptors[crads];
			if(!CRAD::match_with_sequence_separation_interval(crads.first, crads.second, 0, ignorable_max_seq_sep, false))
			{
				ed.total_area=it->second;
				ed.contacts_count=1;
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
		}
		print_pair_scores_to_file(inter_atom_energy_descriptors, escp, detailed_output, inter_atom_scores_file);
	}

	std::map< std::pair<CRAD, CRAD>, EnergyDescriptor > inter_residue_energy_descriptors;
	{
		for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
		{
			const std::pair<CRAD, CRAD>& crads=it->first;
			inter_residue_energy_descriptors[modescommon::refine_pair_by_ordering(std::make_pair(crads.first.without_atom(), crads.second.without_atom()))].add(it->second);
		}
		print_pair_scores_to_file(inter_residue_energy_descriptors, escp, detailed_output, inter_residue_scores_file);
	}

	const std::map< CRAD, std::set<CRAD> > atom_graph=modescommon::construct_graph_from_pair_mapping_of_descriptors(inter_atom_energy_descriptors, depth);
	const std::map<CRAD, EnergyDescriptor> atom_energy_descriptors=modescommon::construct_single_mapping_of_descriptors_from_pair_mapping_of_descriptors(inter_atom_energy_descriptors, atom_graph);
	print_single_scores_to_file(atom_energy_descriptors, escp, detailed_output, atom_scores_file);

	const std::map< CRAD, std::set<CRAD> > residue_graph=modescommon::construct_graph_from_pair_mapping_of_descriptors(inter_residue_energy_descriptors, depth);
	const std::map<CRAD, EnergyDescriptor> residue_energy_descriptors=modescommon::construct_single_mapping_of_descriptors_from_pair_mapping_of_descriptors(inter_residue_energy_descriptors, residue_graph);
	if(residue_scores_to_ref_seq && !reference_sequence.empty())
	{
		print_single_scores_to_file(inject_residue_scores_into_target_sequence(reference_sequence, residue_energy_descriptors), escp, detailed_output, residue_scores_file);
	}
	else
	{
		print_single_scores_to_file(residue_energy_descriptors, escp, detailed_output, residue_scores_file);
	}

	if(!residue_atomic_scores_file.empty())
	{
		std::ofstream foutput(residue_atomic_scores_file.c_str(), std::ios::out);
		if(foutput.good())
		{
			std::map<CRAD, std::pair<int, double> > residue_atom_summed_scores;
			for(std::map<CRAD, EnergyDescriptor>::const_iterator it=atom_energy_descriptors.begin();it!=atom_energy_descriptors.end();++it)
			{
				std::pair<int, double>& residue_value=residue_atom_summed_scores[it->first.without_atom()];
				residue_value.first++;
				residue_value.second+=calculate_energy_score_from_energy_descriptor(it->second, escp).quality_score;
			}
			if(residue_scores_to_ref_seq && !reference_sequence.empty())
			{
				residue_atom_summed_scores=inject_residue_scores_into_target_sequence(reference_sequence, residue_atom_summed_scores);
			}
			std::map<CRAD, double> residue_atomic_scores;
			for(std::map<CRAD, std::pair<int, double> >::const_iterator it=residue_atom_summed_scores.begin();it!=residue_atom_summed_scores.end();++it)
			{
				const std::pair<int, double>& residue_value=it->second;
				residue_atomic_scores[it->first]=((residue_value.first>0) ? (residue_value.second/static_cast<double>(residue_value.first)) : 0.0);
			}
			if(smoothing_window>0)
			{
				residue_atomic_scores=smooth_residue_scores_along_sequence(residue_atomic_scores, smoothing_window);
			}
			for(std::map<CRAD, double>::const_iterator it=residue_atomic_scores.begin();it!=residue_atomic_scores.end();++it)
			{
				foutput << it->first.str() << " " << it->second << "\n";
			}
		}
	}

	{
		EnergyDescriptor global_ed;
		for(std::map< std::pair<CRAD, CRAD>, EnergyDescriptor >::const_iterator it=inter_atom_energy_descriptors.begin();it!=inter_atom_energy_descriptors.end();++it)
		{
			global_ed.add(it->second);
		}
		print_score("global", global_ed, escp, detailed_output, std::cout);
	}

	print_single_scores_summary("atom_level_summary", atom_energy_descriptors, modescommon::count_atoms_from_sequence(reference_sequence), escp, detailed_output, std::cout);
	print_single_scores_summary("residue_level_summary", residue_energy_descriptors, reference_sequence.size(), escp, detailed_output, std::cout);
}

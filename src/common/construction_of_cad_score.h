#ifndef COMMON_CONSTRUCTION_OF_CAD_SCORE_H_
#define COMMON_CONSTRUCTION_OF_CAD_SCORE_H_

#include "chain_residue_atom_descriptor.h"

namespace voronota
{

namespace common
{

class ConstructionOfCADScore
{
public:
	typedef ChainResidueAtomDescriptor CRAD;
	typedef ChainResidueAtomDescriptorsPair CRADsPair;

	struct CADDescriptor
	{
		double target_area_sum;
		double model_area_sum;
		double raw_differences_sum;
		double constrained_differences_sum;
		double model_target_area_sum;

		CADDescriptor() : target_area_sum(0), model_area_sum(0), raw_differences_sum(), constrained_differences_sum(0), model_target_area_sum(0)
		{
		}

		static bool& detailed_output_switch()
		{
			static bool detailed_output=false;
			return detailed_output;
		}

		void add(const double target_area, const double model_area)
		{
			target_area_sum+=target_area;
			model_area_sum+=model_area;
			raw_differences_sum+=fabs(target_area-model_area);
			constrained_differences_sum+=std::min(fabs(target_area-model_area), target_area);
			model_target_area_sum+=(target_area>0.0 ? model_area : 0.0);
		}

		void add(const CADDescriptor& cadd)
		{
			target_area_sum+=cadd.target_area_sum;
			model_area_sum+=cadd.model_area_sum;
			raw_differences_sum+=cadd.raw_differences_sum;
			constrained_differences_sum+=cadd.constrained_differences_sum;
			model_target_area_sum+=cadd.model_target_area_sum;
		}

		double score() const
		{
			return ((target_area_sum>0.0) ? (1.0-(constrained_differences_sum/target_area_sum)) : -1.0);
		}
	};

	struct ParametersToConstructBundleOfCADScoreInformation
	{
		bool ignore_residue_names;
		bool remap_chains;
		bool remap_chains_logging;
		bool atom_level;
		bool residue_level;
		bool binarize;
		int depth;
		std::map<std::string, std::string> map_of_renamings;

		ParametersToConstructBundleOfCADScoreInformation() :
			ignore_residue_names(false),
			remap_chains(false),
			remap_chains_logging(false),
			atom_level(false),
			residue_level(true),
			binarize(false),
			depth(0)
		{
		}
	};

	struct BundleOfCADScoreInformation
	{
		ParametersToConstructBundleOfCADScoreInformation parameters_of_construction;

		std::map<CRADsPair, double> map_of_target_contacts;
		std::map<CRADsPair, double> map_of_contacts;

		std::map<std::string, std::string> map_of_renamings_from_remapping;

		std::map<CRADsPair, CADDescriptor> map_of_inter_atom_cad_descriptors;
		std::map<CRAD, CADDescriptor> map_of_atom_cad_descriptors;
		CADDescriptor atom_level_global_descriptor;
		double atom_average_local_score;

		std::map<CRADsPair, CADDescriptor> map_of_inter_residue_cad_descriptors;
		std::map<CRAD, CADDescriptor> map_of_residue_cad_descriptors;
		CADDescriptor residue_level_global_descriptor;
		double residue_average_local_score;

		BundleOfCADScoreInformation() :
			atom_average_local_score(0.0),
			residue_average_local_score(0.0)
		{
		}

		std::map<CRAD, double> residue_scores(const unsigned int smoothing_window) const
		{
			return ChainResidueAtomDescriptorsSequenceOperations::smooth_residue_scores_along_sequence(
					collect_scores_from_map_of_cad_descriptors(map_of_residue_cad_descriptors), smoothing_window);
		}

		std::map<CRAD, double> residue_scores() const
		{
			return residue_scores(0);
		}
	};

	static bool construct_bundle_of_cadscore_information(
			const ParametersToConstructBundleOfCADScoreInformation& parameters,
			const std::map<CRADsPair, double>& input_map_of_target_contacts,
			const std::map<CRADsPair, double>& input_map_of_contacts,
			BundleOfCADScoreInformation& bundle)
	{
		bundle=BundleOfCADScoreInformation();
		bundle.parameters_of_construction=parameters;

		if(input_map_of_target_contacts.empty() || input_map_of_contacts.empty())
		{
			return false;
		}

		bundle.map_of_target_contacts=input_map_of_target_contacts;
		bundle.map_of_contacts=input_map_of_contacts;

		if(parameters.map_of_renamings.empty())
		{
			bundle.map_of_contacts=input_map_of_contacts;
		}
		else
		{
			bundle.map_of_contacts=rename_chains_in_map_of_contacts(input_map_of_contacts, parameters.map_of_renamings);
		}

		if(parameters.remap_chains)
		{
			remap_chains_optimally(bundle.map_of_target_contacts, parameters.ignore_residue_names, parameters.binarize, parameters.remap_chains_logging, bundle.map_of_renamings_from_remapping, bundle.map_of_contacts);
		}

		if(parameters.atom_level)
		{
			bundle.map_of_inter_atom_cad_descriptors=construct_map_of_cad_descriptors(
					combine_two_pair_mappings_of_values(bundle.map_of_target_contacts, bundle.map_of_contacts, parameters.binarize));

			bundle.map_of_atom_cad_descriptors=filter_map_of_cad_descriptors_by_target_presence(
					common::ChainResidueAtomDescriptorsGraphOperations::accumulate_mapped_values_by_graph_neighbors(bundle.map_of_inter_atom_cad_descriptors, parameters.depth));

			bundle.atom_level_global_descriptor=construct_global_cad_descriptor(bundle.map_of_inter_atom_cad_descriptors);

			bundle.atom_average_local_score=calculate_average_score_from_map_of_cad_descriptors(bundle.map_of_atom_cad_descriptors);
		}

		if(parameters.residue_level)
		{
			bundle.map_of_inter_residue_cad_descriptors=construct_map_of_cad_descriptors(
					combine_two_pair_mappings_of_values(
							summarize_pair_mapping_of_values(bundle.map_of_target_contacts, parameters.ignore_residue_names),
							summarize_pair_mapping_of_values(bundle.map_of_contacts, parameters.ignore_residue_names),
							parameters.binarize));

			bundle.map_of_residue_cad_descriptors=filter_map_of_cad_descriptors_by_target_presence(
					common::ChainResidueAtomDescriptorsGraphOperations::accumulate_mapped_values_by_graph_neighbors(bundle.map_of_inter_residue_cad_descriptors, parameters.depth));

			bundle.residue_level_global_descriptor=construct_global_cad_descriptor(bundle.map_of_inter_residue_cad_descriptors);

			bundle.residue_average_local_score=calculate_average_score_from_map_of_cad_descriptors(bundle.map_of_residue_cad_descriptors);
		}

		return true;
	}

private:
	static std::map<CRADsPair, double> summarize_pair_mapping_of_values(const std::map<CRADsPair, double>& map, const bool ignore_residue_names)
	{
		std::map< CRADsPair, double > result;
		for(std::map< CRADsPair, double >::const_iterator it=map.begin();it!=map.end();++it)
		{
			const CRADsPair& crads=it->first;
			result[crads.without_some_info(true, true, false, ignore_residue_names)]+=it->second;
		}
		return result;
	}

	static std::map< CRADsPair, std::pair<double, double> > combine_two_pair_mappings_of_values(
			const std::map<CRADsPair, double>& map1,
			const std::map<CRADsPair, double>& map2,
			const bool binarize)
	{
		std::map< CRADsPair, std::pair<double, double> > result;
		for(std::map< CRADsPair, double >::const_iterator it=map1.begin();it!=map1.end();++it)
		{
			result[it->first].first=(binarize ? (it->second>0.0 ? 1.0 : 0.0) : it->second);
		}
		for(std::map< CRADsPair, double >::const_iterator it=map2.begin();it!=map2.end();++it)
		{
			result[it->first].second=(binarize ? (it->second>0.0 ? 1.0 : 0.0) : it->second);
		}
		return result;
	}

	static std::map<CRADsPair, CADDescriptor> construct_map_of_cad_descriptors(const std::map< CRADsPair, std::pair<double, double> >& map_of_value_pairs)
	{
		std::map< CRADsPair, CADDescriptor > result;
		for(std::map< CRADsPair, std::pair<double, double> >::const_iterator it=map_of_value_pairs.begin();it!=map_of_value_pairs.end();++it)
		{
			result[it->first].add(it->second.first, it->second.second);
		}
		return result;
	}

	static CADDescriptor construct_global_cad_descriptor(const std::map<CRADsPair, CADDescriptor>& map_of_descriptors)
	{
		CADDescriptor result;
		for(std::map< CRADsPair, CADDescriptor >::const_iterator it=map_of_descriptors.begin();it!=map_of_descriptors.end();++it)
		{
			result.add(it->second);
		}
		return result;
	}

	static CADDescriptor construct_global_cad_descriptor(const std::map<CRADsPair, double>& map_of_target_contacts, const std::map<CRADsPair, double>& map_of_contacts, const bool ignore_residue_names, const bool binarize)
	{
		return construct_global_cad_descriptor(
				construct_map_of_cad_descriptors(
						combine_two_pair_mappings_of_values(
								summarize_pair_mapping_of_values(map_of_target_contacts, ignore_residue_names),
								summarize_pair_mapping_of_values(map_of_contacts, ignore_residue_names),
								binarize)));
	}

	static std::map<CRAD, CADDescriptor> filter_map_of_cad_descriptors_by_target_presence(const std::map<CRAD, CADDescriptor>& input_map)
	{
		std::map<CRAD, CADDescriptor> result;
		for(std::map<CRAD, CADDescriptor>::const_iterator it=input_map.begin();it!=input_map.end();++it)
		{
			if(it->second.target_area_sum>0.0 && it->first.altLoc!="m" && it->first!=CRAD::any())
			{
				result[it->first]=it->second;
			}
		}
		return result;
	}

	static double calculate_average_score_from_map_of_cad_descriptors(const std::map<CRAD, CADDescriptor>& input_map)
	{
		double sum=0.0;
		for(std::map<CRAD, CADDescriptor>::const_iterator it=input_map.begin();it!=input_map.end();++it)
		{
			sum+=it->second.score();
		}
		return (sum/static_cast<double>(input_map.size()));
	}

	static std::map<CRAD, double> collect_scores_from_map_of_cad_descriptors(const std::map<CRAD, CADDescriptor>& input_map)
	{
		std::map<CRAD, double> result;
		for(std::map<CRAD, CADDescriptor>::const_iterator it=input_map.begin();it!=input_map.end();++it)
		{
			result[it->first]=it->second.score();
		}
		return result;
	}

	static std::string rename_by_map(const std::map<std::string, std::string>& map_of_renamings, const std::string& name)
	{
		std::map<std::string, std::string>::const_iterator it=map_of_renamings.find(name);
		if(it!=map_of_renamings.end())
		{
			return it->second;
		}
		else
		{
			return name;
		}
	}

	static std::map<CRADsPair, double> rename_chains_in_map_of_contacts(const std::map<CRADsPair, double>& map_of_contacts, const std::map<std::string, std::string>& map_of_renamings)
	{
		if(map_of_renamings.empty())
		{
			return map_of_contacts;
		}
		std::map<CRADsPair, double> result;
		for(std::map<CRADsPair, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			CRAD a=it->first.a;
			CRAD b=it->first.b;
			a.chainID=rename_by_map(map_of_renamings, a.chainID);
			b.chainID=rename_by_map(map_of_renamings, b.chainID);
			result[CRADsPair(a, b)]+=it->second;
		}
		return result;
	}

	static std::vector<std::string> get_sorted_chain_names_from_map_of_contacts(const std::map<CRADsPair, double>& map_of_contacts)
	{
		std::set<std::string> set_of_names;
		for(std::map<CRADsPair, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			if(it->first.a!=CRAD::solvent() && it->first.a!=CRAD::any())
			{
				set_of_names.insert(it->first.a.chainID);
			}
			if(it->first.b!=CRAD::solvent() && it->first.b!=CRAD::any())
			{
				set_of_names.insert(it->first.b.chainID);
			}
		}
		return std::vector<std::string>(set_of_names.begin(), set_of_names.end());
	}

	static std::map<std::string, std::string> generate_map_of_renamings_from_two_lists(const std::vector<std::string>& left, const std::vector<std::string>& right)
	{
		std::map<std::string, std::string> result;
		if(left.size()==right.size())
		{
			for(std::size_t i=0;i<left.size();i++)
			{
				result[left[i]]=right[i];
			}
		}
		return result;
	}

	static std::map<CRADsPair, double> select_contacts_with_defined_chain_names(const std::map<CRADsPair, double>& map_of_contacts)
	{
		std::map<CRADsPair, double> result;
		for(std::map<CRADsPair, double>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			if(!it->first.a.chainID.empty() || !it->first.b.chainID.empty())
			{
				result[it->first]=it->second;
			}
		}
		return result;
	}

	static void remap_chains_optimally(
			const std::map<CRADsPair, double>& map_of_target_contacts,
			const bool ignore_residue_names,
			const bool binarize,
			const bool write_log_to_stderr,
			std::map<std::string, std::string>& final_map_of_renamings,
			std::map<CRADsPair, double>& map_of_contacts)
	{
		const std::vector<std::string> chain_names=get_sorted_chain_names_from_map_of_contacts(map_of_contacts);
		if(chain_names.size()<2)
		{
			return;
		}
		if(chain_names.size()<=5)
		{
			std::pair<std::map<std::string, std::string>, double> best_renaming(generate_map_of_renamings_from_two_lists(chain_names, chain_names), 0.0);
			std::vector<std::string> permutated_chain_names=chain_names;
			do
			{
				const std::map<std::string, std::string> map_of_renamings=generate_map_of_renamings_from_two_lists(chain_names, permutated_chain_names);
				const double score=construct_global_cad_descriptor(
						map_of_target_contacts,
						rename_chains_in_map_of_contacts(map_of_contacts, map_of_renamings),
						ignore_residue_names,
						binarize).score();
				if(score>best_renaming.second)
				{
					best_renaming.first=map_of_renamings;
					best_renaming.second=score;
				}
				if(write_log_to_stderr)
				{
					for(std::size_t i=0;i<permutated_chain_names.size();i++)
					{
						std::cerr << permutated_chain_names[i] << " ";
					}
					std::cerr << " " << score << "\n";
				}
			}
			while(std::next_permutation(permutated_chain_names.begin(), permutated_chain_names.end()));
			map_of_contacts=rename_chains_in_map_of_contacts(map_of_contacts, best_renaming.first);
			if(write_log_to_stderr)
			{
				std::cerr << "remapping:\n";
				auxiliaries::IOUtilities().write_map(best_renaming.first, std::cerr);
			}
			final_map_of_renamings.swap(best_renaming.first);
		}
		else
		{
			std::map<std::string, std::string> map_of_renamings;
			std::map<std::string, std::string> map_of_renamings_in_target;
			for(std::size_t i=0;i<chain_names.size();i++)
			{
				map_of_renamings[chain_names[i]]=std::string();
				map_of_renamings_in_target[chain_names[i]]=std::string();
			}
			std::set<std::string> set_of_free_chains_left(chain_names.begin(), chain_names.end());
			std::set<std::string> set_of_free_chains_right(chain_names.begin(), chain_names.end());
			while(!set_of_free_chains_left.empty())
			{
				std::pair<std::string, std::string> best_pair(*set_of_free_chains_left.begin(), *set_of_free_chains_right.begin());
				double best_score=0.0;
				for(std::set<std::string>::const_iterator it_left=set_of_free_chains_left.begin();it_left!=set_of_free_chains_left.end();++it_left)
				{
					for(std::set<std::string>::const_iterator it_right=set_of_free_chains_right.begin();it_right!=set_of_free_chains_right.end();++it_right)
					{
						std::map<std::string, std::string> new_map_of_renamings=map_of_renamings;
						std::map<std::string, std::string> new_map_of_renamings_in_target=map_of_renamings_in_target;
						new_map_of_renamings[*it_left]=(*it_right);
						new_map_of_renamings_in_target[*it_right]=(*it_right);
						const CADDescriptor cad_descriptor=construct_global_cad_descriptor(
								select_contacts_with_defined_chain_names(rename_chains_in_map_of_contacts(map_of_target_contacts, new_map_of_renamings_in_target)),
								select_contacts_with_defined_chain_names(rename_chains_in_map_of_contacts(map_of_contacts, new_map_of_renamings)),
								ignore_residue_names,
								binarize);
						const double score=cad_descriptor.score()*cad_descriptor.target_area_sum;
						if(score>best_score)
						{
							best_pair=std::make_pair(*it_left, *it_right);
							best_score=score;
						}
						if(write_log_to_stderr)
						{
							std::cerr << (*it_left) << " " << (*it_right) << "  " << score << "\n";
						}
					}
				}
				if(write_log_to_stderr)
				{
					std::cerr << best_pair.first << " " << best_pair.second << "  " << best_score << " fixed\n";
				}
				map_of_renamings[best_pair.first]=best_pair.second;
				map_of_renamings_in_target[best_pair.second]=best_pair.second;
				set_of_free_chains_left.erase(best_pair.first);
				set_of_free_chains_right.erase(best_pair.second);
			}
			map_of_contacts=rename_chains_in_map_of_contacts(map_of_contacts, map_of_renamings);
			if(write_log_to_stderr)
			{
				std::cerr << "remapping:\n";
				auxiliaries::IOUtilities().write_map(map_of_renamings, std::cerr);
			}
			final_map_of_renamings.swap(map_of_renamings);
		}
	}
};

inline std::ostream& operator<<(std::ostream& output, const ConstructionOfCADScore::CADDescriptor& cadd)
{
	output << cadd.score();
	if(ConstructionOfCADScore::CADDescriptor::detailed_output_switch())
	{
		output << " " << cadd.target_area_sum
				<< " " << cadd.model_area_sum
				<< " " << cadd.raw_differences_sum
				<< " " << cadd.constrained_differences_sum
				<< " " << cadd.model_target_area_sum;
	}
	return output;
}

}

}

#endif /* COMMON_CONSTRUCTION_OF_CAD_SCORE_H_ */

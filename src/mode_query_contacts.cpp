#include <iostream>
#include <stdexcept>
#include <fstream>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/atoms_io.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"
#include "auxiliaries/opengl_printer.h"

#include "modescommon/contact_value.h"
#include "modescommon/matching_utilities.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

std::set<CRADsPair> init_set_of_hbplus_crad_pairs(const std::string& hbplus_file_name, const bool inter_residue_hbplus_tags)
{
	std::set<CRADsPair> set_of_hbplus_crad_pairs;
	if(!hbplus_file_name.empty())
	{
		std::ifstream input_file(hbplus_file_name.c_str(), std::ios::in);
		auxiliaries::AtomsIO::HBPlusReader::Data hbplus_file_data=auxiliaries::AtomsIO::HBPlusReader::read_data_from_file_stream(input_file);
		if(!hbplus_file_data.hbplus_records.empty())
		{
			for(std::vector<auxiliaries::AtomsIO::HBPlusReader::HBPlusRecord>::const_iterator it=hbplus_file_data.hbplus_records.begin();it!=hbplus_file_data.hbplus_records.end();++it)
			{
				const auxiliaries::AtomsIO::HBPlusReader::ShortAtomDescriptor& a=it->first;
				const auxiliaries::AtomsIO::HBPlusReader::ShortAtomDescriptor& b=it->second;
				const CRADsPair crads_pair(CRAD(CRAD::null_num(), a.chainID, a.resSeq, a.resName, a.name, "", ""), CRAD(CRAD::null_num(), b.chainID, b.resSeq, b.resName, b.name, "", ""));
				set_of_hbplus_crad_pairs.insert(inter_residue_hbplus_tags ? CRADsPair(crads_pair.a.without_atom(), crads_pair.b.without_atom()) : crads_pair);
			}
		}
	}
	return set_of_hbplus_crad_pairs;
}

unsigned int calc_string_color_integer(const std::string& str)
{
	const long generator=123456789;
	const long limiter=0xFFFFFF;
	long hash=generator;
	for(std::size_t i=0;i<str.size();i++)
	{
		hash+=static_cast<long>(str[i]+1)*static_cast<long>(i+1)*generator;
	}
	return static_cast<unsigned int>(hash%limiter);
}

unsigned int calc_two_crads_color_integer(const CRAD& a, const CRAD& b)
{
	return calc_string_color_integer(a<b ? (a.str()+b.str()) : (b.str()+a.str()));
}

}

void query_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--match-first", "string", "selection for first contacting group"));
		ods.push_back(OD("--match-first-not", "string", "negative selection for first contacting group"));
		ods.push_back(OD("--match-second", "string", "selection for second contacting group"));
		ods.push_back(OD("--match-second-not", "string", "negative selection for second contacting group"));
		ods.push_back(OD("--match-min-seq-sep", "number", "minimum residue sequence separation"));
		ods.push_back(OD("--match-max-seq-sep", "number", "maximum residue sequence separation"));
		ods.push_back(OD("--match-min-area", "number", "minimum contact area"));
		ods.push_back(OD("--match-max-area", "number", "maximum contact area"));
		ods.push_back(OD("--match-min-dist", "number", "minimum distance"));
		ods.push_back(OD("--match-max-dist", "number", "maximum distance"));
		ods.push_back(OD("--match-tags", "string", "tags to match"));
		ods.push_back(OD("--match-tags-not", "string", "tags to not match"));
		ods.push_back(OD("--match-adjuncts", "string", "adjuncts intervals to match"));
		ods.push_back(OD("--match-adjuncts-not", "string", "adjuncts intervals to not match"));
		ods.push_back(OD("--match-external-first", "string", "file path to input matchable annotations"));
		ods.push_back(OD("--match-external-second", "string", "file path to input matchable annotations"));
		ods.push_back(OD("--match-external-pairs", "string", "file path to input matchable annotation pairs"));
		ods.push_back(OD("--no-solvent", "", "flag to not include solvent accessible areas"));
		ods.push_back(OD("--no-same-chain", "", "flag to not include same chain contacts"));
		ods.push_back(OD("--invert", "", "flag to invert selection"));
		ods.push_back(OD("--drop-tags", "", "flag to drop all tags from input"));
		ods.push_back(OD("--drop-adjuncts", "", "flag to drop all adjuncts from input"));
		ods.push_back(OD("--set-tags", "string", "set tags instead of filtering"));
		ods.push_back(OD("--set-hbplus-tags", "string", "file path to input HBPLUS file"));
		ods.push_back(OD("--set-distance-bins-tags", "string", "list of distance thresholds"));
		ods.push_back(OD("--inter-residue-hbplus-tags", "", "flag to set inter-residue H-bond tags"));
		ods.push_back(OD("--set-adjuncts", "string", "set adjuncts instead of filtering"));
		ods.push_back(OD("--set-external-adjuncts", "string", "file path to input external adjuncts"));
		ods.push_back(OD("--set-external-adjuncts-name", "string", "name for external adjuncts"));
		ods.push_back(OD("--inter-residue", "", "flag to convert input to inter-residue contacts"));
		ods.push_back(OD("--summarize", "", "flag to output only summary of contacts"));
		ods.push_back(OD("--preserve-graphics", "", "flag to preserve graphics in output"));
		ods.push_back(OD("--drawing-for-pymol", "string", "file path to output drawing as pymol script"));
		ods.push_back(OD("--drawing-for-jmol", "string", "file path to output drawing as jmol script"));
		ods.push_back(OD("--drawing-for-scenejs", "string", "file path to output drawing as scenejs script"));
		ods.push_back(OD("--drawing-name", "string", "graphics object name for drawing output"));
		ods.push_back(OD("--drawing-color", "string", "color for drawing output, in hex format, white is 0xFFFFFF"));
		ods.push_back(OD("--drawing-adjunct-gradient", "string", "adjunct name to use for gradient-based coloring"));
		ods.push_back(OD("--drawing-reverse-gradient", "", "flag to use reversed gradient for drawing"));
		ods.push_back(OD("--drawing-random-colors", "", "flag to use random color for each drawn contact"));
		ods.push_back(OD("--drawing-alpha", "number", "alpha opacity value for drawing output"));
		ods.push_back(OD("--drawing-labels", "", "flag to use labels in drawing if possible"));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts [graphics]')");
			poh.print_io_description("stdout", false, true, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts [graphics]')");
			return;
		}
	}

	const std::string match_first=poh.argument<std::string>("--match-first", "");
	const std::string match_first_not=poh.argument<std::string>("--match-first-not", "");
	const std::string match_second=poh.argument<std::string>("--match-second", "");
	const std::string match_second_not=poh.argument<std::string>("--match-second-not", "");
	const int match_min_sequence_separation=poh.argument<int>("--match-min-seq-sep", CRAD::null_num());
	const int match_max_sequence_separation=poh.argument<int>("--match-max-seq-sep", CRAD::null_num());
	const double match_min_area=poh.argument<double>("--match-min-area", std::numeric_limits<double>::min());
	const double match_max_area=poh.argument<double>("--match-max-area", std::numeric_limits<double>::max());
	const double match_min_dist=poh.argument<double>("--match-min-dist", std::numeric_limits<double>::min());
	const double match_max_dist=poh.argument<double>("--match-max-dist", std::numeric_limits<double>::max());
	const std::string match_tags=poh.argument<std::string>("--match-tags", "");
	const std::string match_tags_not=poh.argument<std::string>("--match-tags-not", "");
	const std::string match_adjuncts=poh.argument<std::string>("--match-adjuncts", "");
	const std::string match_adjuncts_not=poh.argument<std::string>("--match-adjuncts-not", "");
	const std::string match_external_first=poh.argument<std::string>("--match-external-first", "");
	const std::string match_external_second=poh.argument<std::string>("--match-external-second", "");
	const std::string match_external_pairs=poh.argument<std::string>("--match-external-pairs", "");
	const bool no_solvent=poh.contains_option("--no-solvent");
	const bool no_same_chain=poh.contains_option("--no-same-chain");
	const bool invert=poh.contains_option("--invert");
	const bool drop_tags=poh.contains_option("--drop-tags");
	const bool drop_adjuncts=poh.contains_option("--drop-adjuncts");
	const std::string set_tags=poh.argument<std::string>("--set-tags", "");
	const std::string set_hbplus_tags=poh.argument<std::string>("--set-hbplus-tags", "");
	const std::string set_distance_bins_tags=poh.argument<std::string>("--set-distance-bins-tags", "");
	const bool inter_residue_hbplus_tags=poh.contains_option("--inter-residue-hbplus-tags");
	const std::string set_adjuncts=poh.argument<std::string>("--set-adjuncts", "");
	const std::string set_external_adjuncts=poh.argument<std::string>("--set-external-adjuncts", "");
	const std::string set_external_adjuncts_name=poh.argument<std::string>("--set-external-adjuncts-name", "ex");
	const bool inter_residue=poh.contains_option("--inter-residue");
	const bool summarize=poh.contains_option("--summarize");
	const std::string drawing_for_pymol=poh.argument<std::string>("--drawing-for-pymol", "");
	const std::string drawing_for_jmol=poh.argument<std::string>("--drawing-for-jmol", "");
	const std::string drawing_for_scenejs=poh.argument<std::string>("--drawing-for-scenejs", "");
	const std::string drawing_name=poh.argument<std::string>("--drawing-name", "contacts");
	const unsigned int drawing_color=auxiliaries::ProgramOptionsHandler::convert_hex_string_to_integer<unsigned int>(poh.argument<std::string>("--drawing-color", "0xFFFFFF"));
	const std::string drawing_adjunct_gradient=poh.argument<std::string>("--drawing-adjunct-gradient", "");
	const bool drawing_reverse_gradient=poh.contains_option("--drawing-reverse-gradient");
	const bool drawing_random_colors=poh.contains_option("--drawing-random-colors");
	const double drawing_alpha=poh.argument<double>("--drawing-alpha", 1.0);
	const bool drawing_labels=poh.contains_option("--drawing-labels");
	enabled_output_of_ContactValue_graphics()=poh.contains_option("--preserve-graphics");

	std::map<CRADsPair, ContactValue> map_of_contacts;
	auxiliaries::IOUtilities().read_lines_to_map(std::cin, map_of_contacts);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	if(inter_residue)
	{
		std::map< CRADsPair, ContactValue > map_of_reduced_contacts;
		for(std::map< CRADsPair, ContactValue >::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const CRADsPair crads(it->first.a.without_atom(), it->first.b.without_atom());
			if(!(crads.a==crads.b))
			{
				map_of_reduced_contacts[crads].add(it->second);
			}
		}
		map_of_contacts=map_of_reduced_contacts;
	}

	std::map<CRADsPair, std::map<CRADsPair, ContactValue>::iterator> selected_contacts;
	{
		const std::set<CRAD> matchable_external_first_set_of_crads=auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRAD> >(match_external_first);
		const std::set<CRAD> matchable_external_second_set_of_crads=auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRAD> >(match_external_second);
		const std::set<CRADsPair> matchable_external_set_of_crad_pairs=auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRADsPair> >(match_external_pairs);

		for(std::map< CRADsPair, ContactValue >::iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
		{
			const CRADsPair& crads=it->first;
			const ContactValue& value=it->second;
			bool passed=false;
			if(
					value.area>=match_min_area && value.area<=match_max_area &&
					value.dist>=match_min_dist && value.dist<=match_max_dist &&
					(!no_solvent || !(crads.a==CRAD::solvent() || crads.b==CRAD::solvent())) &&
					(!no_same_chain || crads.a.chainID!=crads.b.chainID) &&
					CRAD::match_with_sequence_separation_interval(crads.a, crads.b, match_min_sequence_separation, match_max_sequence_separation, true) &&
					MatchingUtilities::match_set_of_tags(value.props.tags, match_tags, match_tags_not) &&
					MatchingUtilities::match_map_of_adjuncts(value.props.adjuncts, match_adjuncts, match_adjuncts_not) &&
					(match_external_pairs.empty() || MatchingUtilities::match_crads_pair_with_set_of_crads_pairs(crads, matchable_external_set_of_crad_pairs))
			)
			{
				const bool matched_first_second=(
						MatchingUtilities::match_crad(crads.a, match_first, match_first_not) &&
						MatchingUtilities::match_crad(crads.b, match_second, match_second_not) &&
						(match_external_first.empty() || MatchingUtilities::match_crad_with_set_of_crads(crads.a, matchable_external_first_set_of_crads)) &&
						(match_external_second.empty() || MatchingUtilities::match_crad_with_set_of_crads(crads.b, matchable_external_second_set_of_crads)));
				const bool matched_second_first=matched_first_second || (
						MatchingUtilities::match_crad(crads.b, match_first, match_first_not) &&
						MatchingUtilities::match_crad(crads.a, match_second, match_second_not) &&
						(match_external_first.empty() || MatchingUtilities::match_crad_with_set_of_crads(crads.b, matchable_external_first_set_of_crads)) &&
						(match_external_second.empty() || MatchingUtilities::match_crad_with_set_of_crads(crads.a, matchable_external_second_set_of_crads)));
				passed=(matched_first_second || matched_second_first);
				if(passed && !invert)
				{
					selected_contacts.insert(selected_contacts.end(), std::make_pair(CRADsPair(crads.a, crads.b, !matched_first_second), it));
				}
			}
			if(!passed && invert)
			{
				selected_contacts.insert(selected_contacts.end(), std::make_pair(crads, it));
			}
		}
	}

	const bool update_mode=(drop_tags || drop_adjuncts || !set_tags.empty() || !set_adjuncts.empty() || !set_external_adjuncts.empty() || !set_hbplus_tags.empty() || !set_distance_bins_tags.empty());
	if(update_mode && !selected_contacts.empty())
	{
		const std::map<CRADsPair, double> map_of_external_adjunct_values=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRADsPair, double> >(set_external_adjuncts);
		const std::set<CRADsPair> set_of_hbplus_crad_pairs=init_set_of_hbplus_crad_pairs(set_hbplus_tags, inter_residue_hbplus_tags);
		const std::set<double> distance_thresholds_for_bins=(set_distance_bins_tags.empty() ? std::set<double>() : auxiliaries::IOUtilities(';').read_string_lines_to_set< std::set<double> >(set_distance_bins_tags));

		for(std::map<CRADsPair, std::map<CRADsPair, ContactValue>::iterator>::iterator selected_map_it=selected_contacts.begin();selected_map_it!=selected_contacts.end();++selected_map_it)
		{
			std::map<CRADsPair, ContactValue>::iterator it=selected_map_it->second;
			if(it!=map_of_contacts.end())
			{
				if(drop_tags)
				{
					it->second.props.tags.clear();
				}
				if(drop_adjuncts)
				{
					it->second.props.adjuncts.clear();
				}
				if(!set_tags.empty())
				{
					it->second.props.update_tags(set_tags);
				}
				if(!set_adjuncts.empty())
				{
					it->second.props.update_adjuncts(set_adjuncts);
				}
				if(!map_of_external_adjunct_values.empty())
				{
					std::map< CRADsPair, double >::const_iterator adjunct_value_it=map_of_external_adjunct_values.find(it->first);
					if(adjunct_value_it!=map_of_external_adjunct_values.end())
					{
						it->second.props.adjuncts[set_external_adjuncts_name]=adjunct_value_it->second;
					}
				}
				if(!set_of_hbplus_crad_pairs.empty())
				{
					if(MatchingUtilities::match_crads_pair_with_set_of_crads_pairs(it->first, set_of_hbplus_crad_pairs))
					{
						it->second.props.tags.insert(inter_residue_hbplus_tags ? "rhb" : "hb");
					}
				}
				if(!distance_thresholds_for_bins.empty())
				{
					std::string bin_tag="d0";
					std::set<double>::const_iterator it_a=distance_thresholds_for_bins.begin();
					if(it->second.dist>=(*it_a))
					{
						bool found=false;
						int bin=1;
						while(it_a!=distance_thresholds_for_bins.end() && !found)
						{
							std::set<double>::const_iterator it_b=it_a; ++it_b;
							if(it->second.dist>=(*it_a) && (it_b==distance_thresholds_for_bins.end() || it->second.dist<(*it_b)))
							{
								std::ostringstream bin_tag_output;
								bin_tag_output << "d" << bin;
								bin_tag=bin_tag_output.str();
								found=true;
							}
							++it_a;
							bin++;
						}
					}
					it->second.props.tags.insert(bin_tag);
				}
			}
		}
	}

	if(summarize)
	{
		ContactValue summary;
		for(std::map<CRADsPair, std::map<CRADsPair, ContactValue>::iterator>::const_iterator it=selected_contacts.begin();it!=selected_contacts.end();++it)
		{
			summary.add(it->second->second);
			summary.graphics.clear();
		}
		std::cout << CRADsPair(CRAD("any"), CRAD("any")) << " " << summary << "\n";
	}
	else
	{
		if(update_mode)
		{
			auxiliaries::IOUtilities().write_map(map_of_contacts, std::cout);
		}
		else
		{
			for(std::map<CRADsPair, std::map<CRADsPair, ContactValue>::iterator>::const_iterator it=selected_contacts.begin();it!=selected_contacts.end();++it)
			{
				std::cout << it->first << " " << it->second->second << "\n";
			}
		}
	}

	if(!selected_contacts.empty() && !(drawing_for_pymol.empty() && drawing_for_jmol.empty() && drawing_for_scenejs.empty()))
	{
		auxiliaries::OpenGLPrinter opengl_printer;
		opengl_printer.add_color(drawing_color);
		opengl_printer.add_alpha(drawing_alpha);
		for(std::map<CRADsPair, std::map<CRADsPair, ContactValue>::iterator>::const_iterator it=selected_contacts.begin();it!=selected_contacts.end();++it)
		{
			const CRADsPair& crads=it->first;
			const ContactValue& value=it->second->second;
			if(!value.graphics.empty())
			{
				if(drawing_labels)
				{
					std::string label=("("+crads.a.str()+")&("+crads.b.str()+")");
					std::replace(label.begin(), label.end(), '<', '[');
					std::replace(label.begin(), label.end(), '>', ']');
					opengl_printer.add_label(label);
				}

				if(!drawing_adjunct_gradient.empty())
				{
					if(value.props.adjuncts.count(drawing_adjunct_gradient)>0)
					{
						const double gradient_value=value.props.adjuncts.find(drawing_adjunct_gradient)->second;
						opengl_printer.add_color_from_blue_white_red_gradient(drawing_reverse_gradient ? (1.0-gradient_value) : gradient_value);
					}
					else
					{
						opengl_printer.add_color(drawing_color);
					}
				}
				else if(drawing_random_colors)
				{
					opengl_printer.add_color(calc_two_crads_color_integer(crads.a, crads.b));
				}

				opengl_printer.add(value.graphics);
			}
		}
		if(!drawing_for_pymol.empty())
		{
			std::ofstream foutput(drawing_for_pymol.c_str(), std::ios::out);
			if(foutput.good())
			{
				opengl_printer.print_pymol_script(drawing_name, true, foutput);
			}
		}
		if(!drawing_for_jmol.empty())
		{
			std::ofstream foutput(drawing_for_jmol.c_str(), std::ios::out);
			if(foutput.good())
			{
				opengl_printer.print_jmol_script(drawing_name, foutput);
			}
		}
		if(!drawing_for_scenejs.empty())
		{
			std::ofstream foutput(drawing_for_scenejs.c_str(), std::ios::out);
			if(foutput.good())
			{
				opengl_printer.print_scenejs_script(drawing_name, true, foutput);
			}
		}
	}
}

void query_contacts_depth_values(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2')");
			poh.print_io_description("stdout", false, true, "list of depth values (line format: 'annotation depth')");
			return;
		}
	}

	const std::set<CRADsPair> set_of_contacts=auxiliaries::IOUtilities().read_lines_to_set< std::set<CRADsPair> >(std::cin);
	if(set_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	std::map<CRAD, int> map_crad_to_depth;
	int level_count=0;

	for(std::set<CRADsPair>::const_iterator it=set_of_contacts.begin();it!=set_of_contacts.end();++it)
	{
		const CRADsPair& crads=(*it);
		if(crads.b==CRAD::solvent())
		{
			map_crad_to_depth[crads.a]=1;
			level_count++;
		}
	}

	for(int depth=1;level_count>0;depth++)
	{
		level_count=0;
		for(std::set<CRADsPair>::const_iterator it=set_of_contacts.begin();it!=set_of_contacts.end();++it)
		{
			const CRADsPair& crads=(*it);
			if(!crads.contains(CRAD::solvent()))
			{
				const int depth_a=map_crad_to_depth[crads.a];
				const int depth_b=map_crad_to_depth[crads.b];
				if(depth_a==depth && depth_b==0)
				{
					map_crad_to_depth[crads.b]=(depth+1);
					level_count++;
				}
				else if(depth_b==depth && depth_a==0)
				{
					map_crad_to_depth[crads.a]=(depth+1);
					level_count++;
				}
			}
		}
	}

	auxiliaries::IOUtilities().write_map(map_crad_to_depth, std::cout);
}

void query_contacts_simulating_unfolding(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--max-seq-sep", "number", "maximum untouchable residue sequence separation", true));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts [graphics]')");
			poh.print_io_description("stdout", false, true, "list of contacts (line format: 'annotation1 annotation2 area distance tags adjuncts')");
			return;
		}
	}

	enabled_output_of_ContactValue_graphics()=false;

	const int match_max_sequence_separation=poh.argument<int>("--max-seq-sep");

	const std::map<CRADsPair, ContactValue> map_of_contacts=auxiliaries::IOUtilities().read_lines_to_map< std::map<CRADsPair, ContactValue> >(std::cin);
	if(map_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	std::map<CRADsPair, ContactValue> result;
	for(std::map< CRADsPair, ContactValue >::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
	{
		const CRADsPair& crads=it->first;
		const ContactValue& value=it->second;
		if(CRAD::match_with_sequence_separation_interval(crads.a, crads.b, 0, match_max_sequence_separation, true) && !crads.contains(CRAD::solvent()))
		{
			result[crads]=value;
		}
		else
		{
			const CRAD* crads_components[2]={&crads.a, &crads.b};
			for(int i=0;i<2;i++)
			{
				const CRAD& crad=(*(crads_components[i]));
				if(crad!=CRAD::solvent())
				{
					ContactValue& solvent_value=result[CRADsPair(crad, CRAD::solvent())];
					solvent_value.area+=value.area;
					solvent_value.dist=std::max(solvent_value.dist, value.dist);
				}
			}
		}
	}

	auxiliaries::IOUtilities().write_map(result, std::cout);
}

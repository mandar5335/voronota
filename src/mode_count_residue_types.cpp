#include <iostream>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

#include "modescommon/sequence_utilities.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

inline bool read_and_accumulate_to_map_of_counts(std::istream& input, std::map<CRAD, long>& map_of_counts)
{
	CRAD crad;
	long count;
	input >> crad >> count;
	if(!input.fail())
	{
		map_of_counts[crad]+=count;
		return true;
	}
	return false;
}

}

void count_residue_types(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--input-pre-counted", "", "flag to input types with counts"));
		ods.push_back(OD("--input-sequence", "", "flag to input sequences"));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false,
					"list of ids, or list of types with counts, or sequence\n(default mode line format: 'id')\n(pre-counted mode line format: 'type count')\n(sequence mode format: plain or FASTA)");
			poh.print_io_description("stdout", false, true,
					"list of types with counts (line format: 'type count')");
			return;
		}
	}

	const bool input_pre_counted=poh.contains_option("--input-pre-counted");
	const bool input_sequence=poh.contains_option("--input-sequence");

	std::map<CRAD, long> result;

	if(input_pre_counted)
	{
		auxiliaries::IOUtilities().read_lines_to_container(std::cin, read_and_accumulate_to_map_of_counts, result);
	}
	else if(input_sequence)
	{
		const std::string sequence=SequenceUtilities::read_sequence_from_stream(std::cin);
		for(std::size_t i=0;i<sequence.size();i++)
		{
			CRAD crad;
			crad.resName=auxiliaries::ResidueLettersCoding::convert_residue_code_small_to_big(sequence.substr(i, 1));
			result[crad]++;
		}
	}
	else
	{
		const std::vector<CRAD> crads=auxiliaries::IOUtilities().read_lines_to_set< std::vector<CRAD> >(std::cin);
		std::set<CRAD> residue_crads;
		for(std::size_t i=0;i<crads.size();i++)
		{
			residue_crads.insert(crads[i].without_atom());
		}
		for(std::set<CRAD>::const_iterator it=residue_crads.begin();it!=residue_crads.end();++it)
		{
			CRAD crad;
			crad.resName=it->resName;
			result[crad]++;
		}
	}

	auxiliaries::IOUtilities().write_map(result, std::cout);
}

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

#include "modescommon/vectorization_utilities.h"
#include "modescommon/filepath_utilities.h"

namespace
{

class CRADsPair : public auxiliaries::ChainResidueAtomDescriptorsPair
{
};

inline std::ostream& operator<<(std::ostream& output, const CRADsPair& crads_pair)
{
	if(crads_pair.reversed_display)
	{
		output << crads_pair.b << "__" << crads_pair.a;
	}
	else
	{
		output << crads_pair.a << "__" << crads_pair.b;
	}
	return output;
}

typedef VectorizationUtilities<std::string, CRADsPair, double> Vectorizer;

Vectorizer::MapOfMaps read_maps_of_contacts()
{
	Vectorizer::MapOfMaps maps_of_contacts;
	const std::set<std::string> input_files=auxiliaries::IOUtilities().read_lines_to_set< std::set<std::string> >(std::cin);
	const std::size_t common_path_start_length=calc_common_path_start_length(input_files);
	for(std::set<std::string>::const_iterator it=input_files.begin();it!=input_files.end();++it)
	{
		const std::string& filename=(*it);
		const Vectorizer::Map raw_map_of_contacts=auxiliaries::IOUtilities().read_file_lines_to_map<Vectorizer::Map>(filename);
		if(!raw_map_of_contacts.empty())
		{
			maps_of_contacts[filename.substr(common_path_start_length)]=raw_map_of_contacts;
		}
	}
	return maps_of_contacts;
}

double calc_configurable_cadscore_of_two_vectors(const std::vector<double>& a, const std::vector<double>& b, const bool symmetric)
{
	if(a.size()!=b.size())
	{
		throw std::runtime_error("Vector sizes are not equal.");
	}
	double sum_diffs=0.0;
	double sum_all=0.0;
	for(std::size_t i=0;i<a.size();i++)
	{
		const double ref=(symmetric ? std::max(a[i], b[i]) : a[i]);
		sum_diffs+=std::min(fabs(a[i]-b[i]), ref);
		sum_all+=ref;
	}
	return (1.0-(sum_diffs/sum_all));
}

double calc_cadscore_of_two_vectors(const std::vector<double>& a, const std::vector<double>& b)
{
	return calc_configurable_cadscore_of_two_vectors(a, b, true);
}

double calc_euclidean_distance_of_two_vectors(const std::vector<double>& a, const std::vector<double>& b)
{
	if(a.size()!=b.size())
	{
		throw std::runtime_error("Vector sizes are not equal.");
	}
	double sum=0.0;
	for(std::size_t i=0;i<a.size();i++)
	{
		sum+=((a[i]-b[i])*(a[i]-b[i]));
	}
	return sqrt(sum);
}

}

void vectorize_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of contacts files");
	pohw.describe_io("stdout", false, true, "table of contacts vectors");

	const bool transpose=poh.contains_option(pohw.describe_option("--transpose", "", "flag to transpose output table"));
	const std::string cadscore_matrix_file=poh.argument<std::string>(pohw.describe_option("--CAD-score-matrix", "string", "file path to output CAD-score matrix"), "");
	const std::string distance_matrix_file=poh.argument<std::string>(pohw.describe_option("--distance-matrix", "string", "file path to output euclidean distance matrix"), "");
	const std::string consensus_list_file=poh.argument<std::string>(pohw.describe_option("--consensus-list", "string", "file path to output ordered similarities to consensus (average contacts)"), "");
	const std::string clustering_output_file=poh.argument<std::string>(pohw.describe_option("--clustering-output", "string", "file path to output clusters"), "");
	const double clustering_threshold=poh.argument<double>(pohw.describe_option("--clustering-threshold", "string", "clustering threshold value"), 0.5);

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const Vectorizer::MapOfMaps maps_of_contacts=read_maps_of_contacts();
	if(maps_of_contacts.empty())
	{
		throw std::runtime_error("No input.");
	}

	const Vectorizer::MapKeysIDs crads_ids=Vectorizer::collect_map_keys_ids(maps_of_contacts);
	const Vectorizer::MapOfVectors map_of_areas_vectors=Vectorizer::collect_map_of_vectors(maps_of_contacts, crads_ids);
	const Vectorizer::IteratorsOfMapOfVectors iterators_of_map_of_areas_vectors=Vectorizer::collect_iterators_of_map_of_vectors(map_of_areas_vectors);

	Vectorizer::print_similarity_matrix(map_of_areas_vectors, cadscore_matrix_file, calc_cadscore_of_two_vectors);
	Vectorizer::print_similarity_matrix(map_of_areas_vectors, distance_matrix_file, calc_euclidean_distance_of_two_vectors);

	if(!consensus_list_file.empty())
	{
		Vectorizer::print_consensus_similarities(
				iterators_of_map_of_areas_vectors,
				Vectorizer::calc_consensus_similarities(iterators_of_map_of_areas_vectors, calc_cadscore_of_two_vectors),
				consensus_list_file);
	}

	if(!clustering_output_file.empty())
	{
		Vectorizer::print_clusters(
				iterators_of_map_of_areas_vectors,
				Vectorizer::calc_clusters(iterators_of_map_of_areas_vectors, calc_cadscore_of_two_vectors, clustering_threshold, true),
				clustering_output_file);
	}

	if(transpose)
	{
		Vectorizer::print_map_of_areas_vectors_transposed(crads_ids, map_of_areas_vectors);
	}
	else
	{
		Vectorizer::print_map_of_areas_vectors(crads_ids, map_of_areas_vectors);
	}
}

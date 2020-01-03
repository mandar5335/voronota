#ifndef SCRIPTING_OPERATORS_EXPORT_SELECTION_OF_ATOMS_H_
#define SCRIPTING_OPERATORS_EXPORT_SELECTION_OF_ATOMS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class ExportSelectionOfAtoms
{
public:
	struct Result
	{
		std::string file;
		std::string dump;
		SummaryOfAtoms atoms_summary;
		std::size_t number_of_descriptors_written;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			if(!dump.empty())
			{
				heterostorage.variant_object.value("dump")=dump;
			}
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.variant_object.value("number_of_descriptors_written")=number_of_descriptors_written;
			return (*this);
		}
	};

	std::string file;
	SelectionManager::Query parameters_for_selecting;
	bool no_serial;
	bool no_name;
	bool no_resSeq;
	bool no_resName;

	ExportSelectionOfAtoms() : file(""), no_serial(false), no_name(false), no_resSeq(false), no_resName(false)
	{
	}

	ExportSelectionOfAtoms& init(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		no_serial=input.get_flag("no-serial");
		no_name=input.get_flag("no-name");
		no_resSeq=input.get_flag("no-resSeq");
		no_resName=input.get_flag("no-resName");
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_file_name_input(file, false);

		const std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		assert_io_stream(file, output);

		std::set<common::ChainResidueAtomDescriptor> set_of_crads;

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			set_of_crads.insert(data_manager.atoms()[*it].crad.without_some_info(no_serial, no_name, no_resSeq, no_resName));
		}

		auxiliaries::IOUtilities().write_set(set_of_crads, output);

		Result result;
		result.file=file;
		if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
		{
			result.dump=output_selector.str();
		}
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);
		result.number_of_descriptors_written=set_of_crads.size();

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_SELECTION_OF_ATOMS_H_ */


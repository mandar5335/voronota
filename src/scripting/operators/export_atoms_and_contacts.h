#ifndef SCRIPTING_OPERATORS_EXPORT_ATOMS_AND_CONTACTS_H_
#define SCRIPTING_OPERATORS_EXPORT_ATOMS_AND_CONTACTS_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class ExportAtomsAndContacts
{
public:
	struct Result
	{
		std::string file;
		std::string dump;
		SummaryOfAtoms atoms_summary;
		SummaryOfContacts contacts_summary;

		Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			if(!dump.empty())
			{
				heterostorage.variant_object.value("dump")=dump;
			}
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			return (*this);
		}
	};

	std::string file;
	bool no_graphics;

	ExportAtomsAndContacts() : no_graphics(false)
	{
	}

	ExportAtomsAndContacts& init(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
		no_graphics=input.get_flag("no-graphics");
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();
		data_manager.assert_contacts_availability();

		assert_file_name_input(file, false);

		OutputSelector output_selector(file);

		{
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);
			auxiliaries::IOUtilities().write_set(data_manager.atoms(), output);
			output << "_end_atoms\n";
			common::enabled_output_of_ContactValue_graphics()=!no_graphics;
			auxiliaries::IOUtilities().write_set(data_manager.contacts(), output);
			output << "_end_contacts\n";
		}

		Result result;
		result.file=file;
		if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
		{
			result.dump=output_selector.str();
		}
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms());
		result.contacts_summary=SummaryOfContacts(data_manager.contacts());

		return result;
	}
};

}

}




#endif /* SCRIPTING_OPERATORS_EXPORT_ATOMS_AND_CONTACTS_H_ */

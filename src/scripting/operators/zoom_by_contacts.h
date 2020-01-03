#ifndef SCRIPTING_OPERATORS_ZOOM_BY_CONTACTS_H_
#define SCRIPTING_OPERATORS_ZOOM_BY_CONTACTS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class ZoomByContacts
{
public:
	struct Result
	{
		SummaryOfAtoms summary_of_atoms;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.summaries_of_atoms["zoomed"]=summary_of_atoms;
			VariantSerialization::write(summary_of_atoms.bounding_box, heterostorage.variant_object.object("bounding_box"));
			return (*this);
		}
	};

	SelectionManager::Query parameters_for_selecting;

	ZoomByContacts()
	{
	}

	ZoomByContacts& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		const std::set<std::size_t> contacts_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);

		if(contacts_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		std::set<std::size_t> atoms_ids;
		for(std::set<std::size_t>::const_iterator it=contacts_ids.begin();it!=contacts_ids.end();++it)
		{
			atoms_ids.insert(data_manager.contacts()[*it].ids[0]);
			atoms_ids.insert(data_manager.contacts()[*it].ids[1]);
		}

		if(atoms_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		Result result;
		result.summary_of_atoms=SummaryOfAtoms(data_manager.atoms(), atoms_ids);

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_ZOOM_BY_CONTACTS_H_ */

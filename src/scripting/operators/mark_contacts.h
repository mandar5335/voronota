#ifndef SCRIPTING_OPERATORS_MARK_CONTACTS_H_
#define SCRIPTING_OPERATORS_MARK_CONTACTS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class MarkContacts
{
public:
	struct Result
	{
		SummaryOfContacts contacts_summary;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			return (*this);
		}
	};

	SelectionManager::Query parameters_for_selecting;

	MarkContacts()
	{
	}

	virtual ~MarkContacts()
	{
	}

	MarkContacts& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		data_manager.update_contacts_display_states(DataManager::DisplayStateUpdater().set_mark(positive()).set_unmark(!positive()), ids);

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);

		return result;
	}

protected:
	virtual bool positive() const
	{
		return true;
	}
};

class UnmarkContacts : public MarkContacts
{
protected:
	bool positive() const
	{
		return false;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_MARK_CONTACTS_H_ */

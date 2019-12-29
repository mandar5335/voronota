#ifndef SCRIPTING_OPERATORS_DELETE_TAGS_OF_CONTACTS_H_
#define SCRIPTING_OPERATORS_DELETE_TAGS_OF_CONTACTS_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class DeleteTagsOfContacts
{
public:
	struct Result
	{
		SummaryOfContacts contacts_summary;

		Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			return (*this);
		}
	};

	SelectionManager::Query parameters_for_selecting;
	bool all;
	std::vector<std::string> tags;

	DeleteTagsOfContacts() : all(false)
	{
	}

	DeleteTagsOfContacts& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		all=input.get_flag("all");
		tags=input.get_value_vector_or_default<std::string>("tags", std::vector<std::string>());
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		if(!all && tags.empty())
		{
			throw std::runtime_error(std::string("No tags specified."));
		}

		if(all && !tags.empty())
		{
			throw std::runtime_error(std::string("Conflicting specification of tags."));
		}

		std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			std::set<std::string>& contact_tags=data_manager.contact_tags_mutable(*it);
			if(all)
			{
				contact_tags.clear();
			}
			else
			{
				for(std::size_t i=0;i<tags.size();i++)
				{
					contact_tags.erase(tags[i]);
				}
			}
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_DELETE_TAGS_OF_CONTACTS_H_ */

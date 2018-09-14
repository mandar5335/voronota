#ifndef COMMON_SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_
#define COMMON_SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_

#include "command_input.h"
#include "data_manager.h"

namespace common
{

namespace scripting
{

class GenericCommandForDataManager
{
public:
	struct CommandRecord
	{
		CommandInput command_input;
		DataManager* data_manager_ptr;
		bool successful;
		bool changed_atoms;
		bool changed_contacts;
		bool changed_atoms_tags;
		bool changed_contacts_tags;
		bool changed_atoms_display_states;
		bool changed_contacts_display_states;
		std::string output_log;
		std::string output_error;
		std::string output_text;
		std::vector<std::size_t> output_set_of_atoms_ids;
		std::vector<std::size_t> output_set_of_contacts_ids;
		SummaryOfAtoms summary_of_atoms;
		SummaryOfContacts summary_of_contacts;
		std::map<std::string, VariantValue> extra_values;

		explicit CommandRecord(const CommandInput& command_input, DataManager& data_manager) :
			command_input(command_input),
			data_manager_ptr(&data_manager),
			successful(false),
			changed_atoms(false),
			changed_contacts(false),
			changed_atoms_tags(false),
			changed_contacts_tags(false),
			changed_atoms_display_states(false),
			changed_contacts_display_states(false)
		{
		}
	};

	GenericCommandForDataManager()
	{
	}

	virtual ~GenericCommandForDataManager()
	{
	}

	CommandRecord execute(const CommandInput& command_input, DataManager& data_manager)
	{
		CommandRecord record(command_input, data_manager);

		std::ostringstream output_for_log;
		std::ostringstream output_for_errors;
		std::ostringstream output_for_text;

		CommandArguments cargs(record.command_input, data_manager, output_for_log, output_for_text);

		try
		{
			data_manager.sync_selections_with_display_states_if_requested_in_string(record.command_input.get_canonical_input_command_string());
			run(cargs);
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			output_for_errors << e.what();
		}

		record.output_log=output_for_log.str();
		record.output_error=output_for_errors.str();
		record.output_text=output_for_text.str();

		record.output_set_of_atoms_ids=std::vector<std::size_t>(cargs.output_set_of_atoms_ids.begin(), cargs.output_set_of_atoms_ids.end());
		record.output_set_of_contacts_ids=std::vector<std::size_t>(cargs.output_set_of_contacts_ids.begin(), cargs.output_set_of_contacts_ids.end());

		record.summary_of_atoms=cargs.summary_of_atoms;
		record.summary_of_contacts=cargs.summary_of_contacts;

		record.extra_values.swap(cargs.extra_values);

		record.changed_atoms=cargs.changed_atoms;
		record.changed_contacts=(cargs.changed_contacts || record.changed_atoms);
		record.changed_atoms_tags=(cargs.changed_atoms_tags || record.changed_atoms);
		record.changed_contacts_tags=(cargs.changed_contacts_tags || record.changed_contacts);
		record.changed_atoms_display_states=(cargs.changed_atoms_display_states || record.changed_atoms);
		record.changed_contacts_display_states=(cargs.changed_contacts_display_states || record.changed_contacts);

		return record;
	}

	virtual bool allowed_to_work_on_multiple_data_managers(const CommandInput&) const
	{
		return false;
	}

protected:
	class CommandArguments
	{
	public:
		CommandInput& input;
		DataManager& data_manager;
		std::ostream& output_for_log;
		std::ostream& output_for_text;
		bool changed_atoms;
		bool changed_contacts;
		bool changed_atoms_tags;
		bool changed_contacts_tags;
		bool changed_atoms_display_states;
		bool changed_contacts_display_states;
		std::set<std::size_t> output_set_of_atoms_ids;
		std::set<std::size_t> output_set_of_contacts_ids;
		SummaryOfAtoms summary_of_atoms;
		SummaryOfContacts summary_of_contacts;
		std::map<std::string, VariantValue> extra_values;

		CommandArguments(
				CommandInput& input,
				DataManager& data_manager,
				std::ostream& output_for_log,
				std::ostream& output_for_text) :
					input(input),
					data_manager(data_manager),
					output_for_log(output_for_log),
					output_for_text(output_for_text),
					changed_atoms(false),
					changed_contacts(false),
					changed_atoms_tags(false),
					changed_contacts_tags(false),
					changed_atoms_display_states(false),
					changed_contacts_display_states(false)
		{
		}
	};

	virtual void run(CommandArguments& /*cargs*/)
	{
	}
};

}

}

#endif /* COMMON_SCRIPTING_GENERIC_COMMAND_FOR_DATA_MANAGER_H_ */

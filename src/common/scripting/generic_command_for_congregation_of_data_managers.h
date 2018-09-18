#ifndef COMMON_SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_
#define COMMON_SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_

#include "command_input.h"
#include "congregation_of_data_managers.h"
#include "change_indicator_for_congregation_of_data_managers.h"
#include "heterogeneous_storage.h"

namespace common
{

namespace scripting
{

class GenericCommandForCongregationOfDataManagers
{
public:
	struct CommandRecord
	{
		CommandInput command_input;
		CongregationOfDataManagers* congregation_of_data_managers;
		bool successful;
		ChangeIndicatorForCongregationOfDataManagers change_indicator;
		HeterogeneousStorage heterostorage;

		explicit CommandRecord(const CommandInput& command_input, CongregationOfDataManagers& congregation_of_data_managers) :
			command_input(command_input),
			congregation_of_data_managers(&congregation_of_data_managers),
			successful(false)
		{
		}
	};

	GenericCommandForCongregationOfDataManagers()
	{
	}

	virtual ~GenericCommandForCongregationOfDataManagers()
	{
	}

	CommandRecord execute(const CommandInput& command_input, CongregationOfDataManagers& congregation_of_data_managers)
	{
		CommandRecord record(command_input, congregation_of_data_managers);

		std::ostringstream output_for_log;
		std::ostringstream output_for_errors;

		CommandArguments cargs(record.command_input, congregation_of_data_managers, record.change_indicator, output_for_log, record.heterostorage);

		try
		{
			run(cargs);
			record.successful=true;
		}
		catch(const std::exception& e)
		{
			output_for_errors << e.what();
		}

		record.heterostorage.log+=output_for_log.str();
		record.heterostorage.error+=output_for_errors.str();

		record.change_indicator.ensure_correctness();

		return record;
	}

protected:
	class CommandArguments
	{
	public:
		CommandInput& input;
		CongregationOfDataManagers& congregation_of_data_managers;
		ChangeIndicatorForCongregationOfDataManagers& change_indicator;
		std::ostream& output_for_log;
		HeterogeneousStorage& heterostorage;

		CommandArguments(
				CommandInput& input,
				CongregationOfDataManagers& congregation_of_data_managers,
				ChangeIndicatorForCongregationOfDataManagers& change_indicator,
				std::ostream& output_for_log,
				HeterogeneousStorage& heterostorage) :
					input(input),
					congregation_of_data_managers(congregation_of_data_managers),
					change_indicator(change_indicator),
					output_for_log(output_for_log),
					heterostorage(heterostorage)
		{
		}
	};

	virtual void run(CommandArguments& /*cargs*/)
	{
	}
};

}

}

#endif /* COMMON_SCRIPTING_GENERIC_COMMAND_FOR_CONGREGATION_OF_DATA_MANAGERS_H_ */

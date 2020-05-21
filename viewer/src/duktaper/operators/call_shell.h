#ifndef DUKTAPER_OPERATORS_CALL_SHELL_H_
#define DUKTAPER_OPERATORS_CALL_SHELL_H_

#include <redi/pstream.h>

#include "../operators_common.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class CallShell : public scripting::operators::OperatorBase<CallShell>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		int exit_status;
		std::string stdout;
		std::string stderr;

		Result() : exit_status(1)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("exit_status")=exit_status;
			heterostorage.variant_object.value("stdout")=stdout;
			heterostorage.variant_object.value("stderr")=stderr;
		}
	};

	std::string command_string;
	std::string input_data_file;
	std::string input_data_string;

	CallShell()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		command_string=input.get_value_or_first_unused_unnamed_value("command-string");
		input_data_file=input.get_value_or_default<std::string>("input-data-file", "");
		input_data_string=input.get_value_or_default<std::string>("input-data-string", "");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(scripting::CDOD("command-string", scripting::CDOD::DATATYPE_STRING, "command string"));
		doc.set_option_decription(scripting::CDOD("input-data-file", scripting::CDOD::DATATYPE_STRING, "path to input data file", ""));
		doc.set_option_decription(scripting::CDOD("input-data-string", scripting::CDOD::DATATYPE_STRING, "input data string", ""));
	}

	Result run(void*) const
	{
		if(command_string.empty())
		{
			throw std::runtime_error(std::string("Mo command string provided."));
		}

		if(!input_data_file.empty() && !input_data_string.empty())
		{
			throw std::runtime_error(std::string("More than one input data source provided."));
		}

		std::string input_data;

		if(!input_data_file.empty())
		{
			scripting::InputSelector finput_selector(input_data_file);
			std::istream& finput=finput_selector.stream();

			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to read file '")+input_data_file+"'.");
			}

			std::istreambuf_iterator<char> eos;
			std::string data(std::istreambuf_iterator<char>(finput), eos);

			input_data.swap(data);
		}
		else if(!input_data_string.empty())
		{
			input_data=input_data_string;
		}

		const std::string command=std::string("#!/bin/bash\n")+command_string;

		redi::pstream proc(command, input_data.empty() ? (redi::pstreams::pstdout|redi::pstreams::pstderr) : (redi::pstreams::pstdin|redi::pstreams::pstdout|redi::pstreams::pstderr));

		if(!input_data.empty())
		{
			proc << input_data;
			proc.rdbuf()->peof();
		}

		Result result;

		if(proc.out().good())
		{
			std::istreambuf_iterator<char> eos;
			result.stdout=std::string(std::istreambuf_iterator<char>(proc.out()), eos);
		}

		if(proc.err().good())
		{
			std::istreambuf_iterator<char> eos;
			result.stderr=std::string(std::istreambuf_iterator<char>(proc.err()), eos);
		}

		proc.close();

		if(proc.rdbuf()->exited())
		{
			result.exit_status=proc.rdbuf()->status();
		}
		else
		{
			result.exit_status=0;
		}

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_CALL_SHELL_H_ */

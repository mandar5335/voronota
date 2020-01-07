#ifndef SCRIPTING_OPERATORS_SET_ALIAS_H_
#define SCRIPTING_OPERATORS_SET_ALIAS_H_

#include "../operators_common.h"

#include "../script_partitioner.h"

namespace scripting
{

namespace operators
{

class SetAlias : public OperatorBase<SetAlias>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string alias;
		std::string script;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("alias")=alias;
			heterostorage.variant_object.value("script")=script;
			return (*this);
		}
	};

	std::string name;
	std::string script_template;

	SetAlias()
	{
	}

	SetAlias& init(CommandInput& input)
	{
		name=input.get_value_or_first_unused_unnamed_value("name");
		script_template=input.get_value_or_first_unused_unnamed_value("script-template");
		return (*this);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "alias name"));
		doc.set_option_decription(CDOD("script-template", CDOD::DATATYPE_STRING, "script template"));
	}

	Result run(ScriptPartitioner& script_partitioner) const
	{
		if(name.empty())
		{
			throw std::runtime_error(std::string("Empty alias name."));
		}

		if(script_template.empty())
		{
			throw std::runtime_error(std::string("Empty script template."));
		}

		if(name==script_template)
		{
			throw std::runtime_error(std::string("Alias name is the same as the script template."));
		}

		script_partitioner.set_alias(name, script_template);

		Result result;
		result.alias=name;
		result.script=script_template;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_SET_ALIAS_H_ */

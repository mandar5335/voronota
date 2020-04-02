#ifndef DUKTAPER_OPERATORS_TMALIGN_ALL_H_
#define DUKTAPER_OPERATORS_TMALIGN_ALL_H_

#include "tmalign.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class TMalignAll : public scripting::operators::OperatorBase<TMalign>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	std::string target_name;
	std::string target_selection;
	std::string model_selection;

	TMalignAll()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		target_name=input.get_value_or_first_unused_unnamed_value_or_default("target", "");
		target_selection=input.get_value_or_default<std::string>("target-sel", "");
		model_selection=input.get_value_or_default<std::string>("model-sel", "");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(scripting::CDOD("target", scripting::CDOD::DATATYPE_STRING, "name of target object", ""));
		doc.set_option_decription(scripting::CDOD("target-sel", scripting::CDOD::DATATYPE_STRING, "selection of atoms for target object", ""));
		doc.set_option_decription(scripting::CDOD("model-sel", scripting::CDOD::DATATYPE_STRING, "selection of atoms for model object", ""));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		const std::vector<scripting::DataManager*> objects=congregation_of_data_managers.get_objects();

		if(objects.size()<2)
		{
			throw std::runtime_error(std::string("Less than two objects available."));
		}

		const std::string target_name_to_use=(target_name.empty() ? congregation_of_data_managers.get_object_attributes(objects[0]).name : target_name);

		congregation_of_data_managers.assert_object_availability(target_name_to_use);

		for(std::size_t i=0;i<objects.size();i++)
		{
			const scripting::CongregationOfDataManagers::ObjectAttributes attributes=congregation_of_data_managers.get_object_attributes(objects[i]);
			if(attributes.name!=target_name_to_use)
			{
				std::ostringstream args;
				args << "-target " << target_name_to_use << " -model " << attributes.name;
				if(!target_selection.empty())
				{
					args << " -target-sel " << target_selection;
				}
				if(!model_selection.empty())
				{
					args << " -model-sel " << model_selection;
				}
				TMalign().init(args.str()).run(congregation_of_data_managers);
			}
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_TMALIGN_ALL_H_ */

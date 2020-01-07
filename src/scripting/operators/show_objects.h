#ifndef SCRIPTING_OPERATORS_SHOW_OBJECTS_H_
#define SCRIPTING_OPERATORS_SHOW_OBJECTS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class ShowObjects : public OperatorBase<ShowObjects>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		const Result& write(HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	CongregationOfDataManagers::ObjectQuery query;

	ShowObjects()
	{
	}

	virtual ~ShowObjects()
	{
	}

	ShowObjects& init(CommandInput& input)
	{
		query=Utilities::read_congregation_of_data_managers_object_query(input);
		return (*this);
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		const std::vector<DataManager*> objects=congregation_of_data_managers.get_objects(query);
		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		for(std::size_t i=0;i<objects.size();i++)
		{
			congregation_of_data_managers.set_object_visible(objects[i], positive());
		}

		Result result;

		return result;
	}

protected:
	virtual bool positive() const
	{
		return true;
	}
};

class HideObjects : public ShowObjects
{
protected:
	bool positive() const
	{
		return false;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_SHOW_OBJECTS_H_ */

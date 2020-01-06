#ifndef SCRIPTING_OPERATORS_ECHO_H_
#define SCRIPTING_OPERATORS_ECHO_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class Echo
{
public:
	struct Result
	{
		std::vector<std::string> strings;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			for(std::size_t i=0;i<strings.size();i++)
			{
				heterostorage.variant_object.values_array("lines").push_back(VariantValue(strings[i]));
			}
			return (*this);
		}
	};

	std::vector<std::string> strings;

	Echo()
	{
	}

	Echo& init(CommandInput& input)
	{
		strings=input.get_value_vector_or_all_unnamed_values("strings");
		return (*this);
	}

	Result run(void*&) const
	{
		Result result;
		result.strings=strings;
		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_ECHO_H_ */

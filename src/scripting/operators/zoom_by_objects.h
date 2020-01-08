#ifndef SCRIPTING_OPERATORS_ZOOM_BY_OBJECTS_H_
#define SCRIPTING_OPERATORS_ZOOM_BY_OBJECTS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class ZoomByObjects : public OperatorBase<ZoomByObjects>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms summary_of_atoms;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			if(summary_of_atoms.bounding_box.filled)
			{
				heterostorage.summaries_of_atoms["zoomed"]=summary_of_atoms;
				VariantSerialization::write(summary_of_atoms.bounding_box, heterostorage.variant_object.object("bounding_box"));
			}
			return (*this);
		}
	};

	CongregationOfDataManagers::ObjectQuery query;

	ZoomByObjects()
	{
	}

	ZoomByObjects& init(CommandInput& input)
	{
		query=Utilities::read_congregation_of_data_managers_object_query(input);
		return (*this);
	}

	void document(CommandDocumentation& doc) const
	{
		Utilities::document_read_congregation_of_data_managers_object_query(doc);
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		const std::vector<DataManager*> objects=congregation_of_data_managers.get_objects(query);
		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		Result result;

		for(std::size_t i=0;i<objects.size();i++)
		{
			result.summary_of_atoms.feed(SummaryOfAtoms(objects[i]->atoms()));
		}

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_ZOOM_BY_OBJECTS_H_ */

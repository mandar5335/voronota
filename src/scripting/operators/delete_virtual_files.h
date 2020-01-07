#ifndef SCRIPTING_OPERATORS_DELETE_VIRTUAL_FILES_H_
#define SCRIPTING_OPERATORS_DELETE_VIRTUAL_FILES_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class DeleteVirtualFiles : public OperatorBase<DeleteVirtualFiles>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		const Result& write(HeterogeneousStorage&) const
		{
			return (*this);
		}
	};

	std::vector<std::string> filenames;

	DeleteVirtualFiles()
	{
	}

	DeleteVirtualFiles& init(CommandInput& input)
	{
		filenames=input.get_value_vector_or_all_unnamed_values("filenames");
		return (*this);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("filenames", CDOD::DATATYPE_STRING_ARRAY, "file names", ""));
	}

	Result run(void*&) const
	{
		if(filenames.empty())
		{
			VirtualFileStorage::clear();
		}
		else
		{
			for(std::size_t i=0;i<filenames.size();i++)
			{
				VirtualFileStorage::assert_file_exists(filenames[i]);
			}

			for(std::size_t i=0;i<filenames.size();i++)
			{
				VirtualFileStorage::delete_file(filenames[i]);
			}
		}

		Result result;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_DELETE_VIRTUAL_FILES_H_ */

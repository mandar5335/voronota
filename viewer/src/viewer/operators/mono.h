#ifndef VIEWER_OPERATORS_MONO_H_
#define VIEWER_OPERATORS_MONO_H_

#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Mono : public scripting::operators::OperatorBase<Mono>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	Mono()
	{
	}

	void initialize(scripting::CommandInput&)
	{
	}

	void document(scripting::CommandDocumentation&) const
	{
	}

	Result run(void*&) const
	{
		uv::ViewerApplication::instance().set_rendering_mode_to_simple();
		Result result;
		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_MONO_H_ */

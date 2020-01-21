#ifndef VIEWER_OPERATORS_GRID_H_
#define VIEWER_OPERATORS_GRID_H_

#include "../operators_common.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Grid : public scripting::operators::OperatorBase<Grid>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		void store(scripting::HeterogeneousStorage&) const
		{
		}
	};

	Grid(const int grid_variant_value, int& grid_variant) : grid_variant_value_(grid_variant_value), grid_variant_ptr_(&grid_variant)
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
		uv::ViewerApplication::instance().set_rendering_mode_to_grid();
		(*grid_variant_ptr_)=grid_variant_value_;
		Result result;
		return result;
	}

private:
	int grid_variant_value_;
	int* grid_variant_ptr_;
};

}

}

}

#endif /* VIEWER_OPERATORS_GRID_H_ */

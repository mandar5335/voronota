#ifndef UV_TRANSFORMATION_MATRIX_CONTROLLER_H_
#define UV_TRANSFORMATION_MATRIX_CONTROLLER_H_

#include "common.h"

namespace uv
{

class TransformationMatrixController
{
public:
	TransformationMatrixController() : matrix_(1.0f)
	{
	}

	static TransformationMatrixController create_projection_ortho(const int width, const int height, const float z_near, const float z_far)
	{
		TransformationMatrixController tmc;
		float side=static_cast<float>(std::min(width, height));
		float width_range=static_cast<float>(width)/side;
		float height_range=static_cast<float>(height)/side;
		tmc.matrix_=glm::ortho(-width_range, +width_range, -height_range, +height_range, z_near, z_far);
		return tmc;
	}

	static TransformationMatrixController create_projection_perspective(const int width, const int height)
	{
		TransformationMatrixController tmc;
		float aspect_ratio=static_cast<float>(width)/static_cast<float>(height);
		tmc.matrix_=glm::perspective(45.0f, aspect_ratio, 0.1f, 100.0f);
		return tmc;
	}

	static TransformationMatrixController create_viewtransform_simple(const double zoom_value)
	{
		TransformationMatrixController tmc;
		tmc.add_scaling(zoom_value);
		return tmc;
	}

	static TransformationMatrixController create_viewtransform_simple_stereo(const double zoom_value, const float stereo_angle, const float stereo_offset, const int index)
	{
		TransformationMatrixController tmc;
		tmc.add_scaling(zoom_value);
		tmc.add_rotation((index==0 ? stereo_angle : -stereo_angle), glm::vec3(0.0f, 1.0f, 0.0f));
		tmc.add_translation(glm::vec3((index==0 ? -stereo_offset : stereo_offset), 0.0f, 0.0f));
		return tmc;
	}

	static TransformationMatrixController create_viewtransform_look_at(const double zoom_value)
	{
		TransformationMatrixController tmc;
		float dist=static_cast<float>(50.0/zoom_value);
		tmc.matrix_=glm::lookAt(glm::vec3(0.0f, 0.0f, dist), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		return tmc;
	}

	const glm::mat4& matrix() const
	{
		return matrix_;
	}

	const GLfloat* matrix_data() const
	{
		return (&matrix_[0][0]);
	}

	void add_rotation(const float angle_in_radians, const glm::vec3& axis)
	{
		matrix_=glm::rotate(glm::mat4(1.0f), angle_in_radians, axis)*matrix_;
	}

	void add_translation(const glm::vec3& offset)
	{
		matrix_=glm::translate(glm::mat4(1.0f), offset)*matrix_;
	}

	void add_scaling(const float factor)
	{
		matrix_=glm::scale(glm::mat4(1.0f), glm::vec3(factor, factor, factor))*matrix_;
	}

	void add_scaling(const float factor_x, const float factor_y, const float factor_z)
	{
		matrix_=glm::scale(glm::mat4(1.0f), glm::vec3(factor_x, factor_y, factor_z))*matrix_;
	}

	void reset()
	{
		matrix_=glm::mat4(1.0f);
	}

	void center(const glm::vec3& pos)
	{
		add_translation(glm::vec3(matrix_*glm::vec4(pos, 1.0f))*(-1.0f));
	}

private:
	glm::mat4 matrix_;
};

}

#endif /* UV_TRANSFORMATION_MATRIX_CONTROLLER_H_ */

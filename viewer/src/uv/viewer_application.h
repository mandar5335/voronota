#ifndef UV_VIEWER_APPLICATION_H_
#define UV_VIEWER_APPLICATION_H_

#include "common.h"
#include "shading_controller.h"
#include "transformation_matrix_controller.h"
#include "zoom_calculator.h"

#include <GLFW/glfw3.h>

namespace uv
{

class ViewerApplication : private Noncopyable
{
public:
	struct InitializationParameters
	{
		int suggested_window_width;
		int suggested_window_height;
		std::string title;
		std::string shader_vertex;
		std::string shader_vertex_with_instancing;
		std::string shader_fragment;

		InitializationParameters() :
			suggested_window_width(800),
			suggested_window_height(600)
		{
		}
	};

	ViewerApplication() :
		good_(false),
		window_(0),
		minimum_window_width_(default_minimum_window_width()),
		minimum_window_height_(default_minimum_window_height()),
		window_width_(0),
		window_height_(0),
		margin_left_(0),
		margin_right_(0),
		margin_bottom_(0),
		margin_top_(0),
		zoom_value_(1.0),
		zoom_value_step_(1.05),
		ortho_z_near_(-2.0f),
		ortho_z_far_(2.0f),
		ortho_z_step_(1.05),
		mouse_button_left_down_(false),
		mouse_button_right_down_(false),
		mouse_button_left_click_possible_(false),
		mouse_button_right_click_possible_(false),
		mouse_cursor_x_(0.0),
		mouse_cursor_y_(0.0),
		mouse_cursor_prev_x_(0.0),
		mouse_cursor_prev_y_(0.0),
		call_for_selection_(0),
		stereo_angle_(0.09),
		stereo_offset_(0.0),
		grid_size_(1),
		rendering_mode_(RenderingMode::simple)
	{
		Utilities::calculate_color_from_integer(0, background_color_);
	}

	virtual ~ViewerApplication()
	{
		if(good())
		{
			glfwTerminate();
		}
	}

	static void render(void* state)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(state);
		app->render();
	}

	bool init(const InitializationParameters& parameters)
	{
		if(good())
		{
			return false;
		}

		if(!glfwInit())
		{
			std::cout << "Error: failed to init GLFW3" << std::endl;
			return false;
		}

		window_width_=std::max(minimum_window_width_, parameters.suggested_window_width);
		window_height_=std::max(minimum_window_height_, parameters.suggested_window_height);

		glfwWindowHint(GLFW_SAMPLES, 4);

		window_=glfwCreateWindow(window_width_, window_height_, parameters.title.c_str(), 0, 0);
		if(!window_)
		{
			std::cout << "Error: failed to create window with GLFW3" << std::endl;
			glfwTerminate();
			return false;
		}

		//glfwSetWindowSizeLimits(window_, minimum_window_width_, minimum_window_height_, GLFW_DONT_CARE, GLFW_DONT_CARE);

		glfwSetWindowUserPointer(window_, this);

		glfwSetWindowSizeCallback(window_, callback_on_window_resized);
		glfwSetScrollCallback(window_, callback_on_window_scrolled);
		glfwSetMouseButtonCallback(window_, callback_on_mouse_button_used);
		glfwSetCursorPosCallback(window_, callback_on_mouse_cursor_moved);
		glfwSetKeyCallback(window_, callback_on_key_used);
		glfwSetCharCallback(window_, callback_on_character_used);

		glfwMakeContextCurrent(window_);

		glewExperimental=GL_TRUE;
		glewInit();

		{
			const GLubyte* renderer;
			renderer=glGetString(GL_RENDERER);
			std::cout << "Renderer: " << renderer << std::endl;
		}

		{
			const GLubyte* version;
			version=glGetString(GL_VERSION);
			std::cout << "Version: " << version << std::endl;
		}

		//glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		if(!shading_simple_.init(parameters.shader_vertex, parameters.shader_fragment))
		{
			std::cout << "Error: failed to init shading simple." << std::endl;
			glfwTerminate();
			return false;
		}

		if(!shading_with_instancing_.init(parameters.shader_vertex_with_instancing, parameters.shader_fragment))
		{
			std::cout << "Error: failed to init shading with instancing." << std::endl;
			glfwTerminate();
			return false;
		}

		modeltransform_matrix_=TransformationMatrixController();

		refresh_shading_projection();
		refresh_shading_viewtransform();
		refresh_shading_modeltransform();

		good_=true;

		return true;
	}

	void run_loop()
	{
		while(good() && !glfwWindowShouldClose(window_))
		{
			render();
		}
	}

	bool good() const
	{
		return good_;
	}

	GLFWwindow* window()
	{
		return window_;
	}

	int window_width() const
	{
		return window_width_;
	}

	int window_height() const
	{
		return window_height_;
	}

	const float* background_color() const
	{
		return &background_color_[0];
	}

	bool rendering_mode_is_simple() const
	{
		return (rendering_mode_==RenderingMode::simple);
	}

	bool rendering_mode_is_stereo() const
	{
		return (rendering_mode_==RenderingMode::stereo);
	}

	bool rendering_mode_is_grid() const
	{
		return (rendering_mode_==RenderingMode::grid);
	}

	float stereo_angle() const
	{
		return stereo_angle_;
	}

	float stereo_offset() const
	{
		return stereo_offset_;
	}

	int grid_size() const
	{
		return grid_size_;
	}

	void close()
	{
		if(good())
		{
			glfwSetWindowShouldClose(window_, 1);
		}
	}

	void zoom(const ZoomCalculator& zoom_calculator)
	{
		if(!good())
		{
			return;
		}

		zoom_value_=zoom_calculator.get_zoom_factor();
		modeltransform_matrix_.center(zoom_calculator.get_center_position());
		refresh_shading_viewtransform();
		refresh_shading_modeltransform();
	}

	void rotate(const glm::vec3& axis, const float angle)
	{
		if(!good())
		{
			return;
		}

		if(angle!=0.0f && glm::length(axis)>0.0)
		{
			modeltransform_matrix_.add_rotation(angle, glm::normalize(axis));
			refresh_shading_modeltransform();
		}
	}

	void set_window_size(const int width, const int height)
	{
		glfwSetWindowSize(window_, width, height);
	}

	void set_background_color(unsigned int rgb)
	{
		Utilities::calculate_color_from_integer(rgb, background_color_);
	}

	void set_rendering_mode_to_simple()
	{
		rendering_mode_=RenderingMode::simple;
	}

	void set_rendering_mode_to_stereo()
	{
		rendering_mode_=RenderingMode::stereo;
	}

	void set_rendering_mode_to_stereo(const float stereo_angle, const float stereo_offset)
	{
		set_stereo_angle(stereo_angle);
		set_stereo_offset(stereo_offset);
		set_rendering_mode_to_stereo();
	}

	void set_rendering_mode_to_grid()
	{
		rendering_mode_=RenderingMode::grid;
	}

	void set_rendering_mode_to_grid(const int grid_size)
	{
		set_grid_size(grid_size);
		set_rendering_mode_to_grid();
	}

	void set_fog_enabled(const bool enabled)
	{
		if(!good())
		{
			return;
		}

		shading_simple_.set_fog_enabled(enabled);
		shading_with_instancing_.set_fog_enabled(enabled);
	}

	void set_stereo_angle(const float stereo_angle)
	{
		stereo_angle_=stereo_angle;
	}

	void set_stereo_offset(const float stereo_offset)
	{
		stereo_offset_=stereo_offset;
	}

	void set_grid_size(const int grid_size)
	{
		grid_size_=(grid_size>=1 ? grid_size : 1);
	}

	bool read_pixels(int& image_width, int& image_height, std::vector<char>& image_data)
	{
		image_width=full_rendering_area_width();
		image_height=full_rendering_area_height();
		image_data.clear();
		image_data.resize(image_width*image_height*3);

		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_PACK_SKIP_ROWS, 0);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(margin_left_, margin_bottom_, image_width, image_height, GL_RGB, GL_UNSIGNED_BYTE, &image_data[0]);

		return true;
	}

protected:
	bool set_margins(const int margin_left, const int margin_right, const int margin_bottom, const int margin_top)
	{
		if(margin_left>=0 && margin_right>=0 && margin_top>=0 && margin_bottom>=0)
		{
			margin_left_=margin_left;
			margin_right_=margin_right;
			margin_bottom_=margin_bottom;
			margin_top_=margin_top;
			minimum_window_width_=margin_left_+margin_right_+default_minimum_window_width();
			minimum_window_height_=margin_top_+margin_bottom_+default_minimum_window_height();
//			glfwSetWindowSizeLimits(window_, minimum_window_width_, minimum_window_height_, GLFW_DONT_CARE, GLFW_DONT_CARE);
			refresh_shading_projection();
			return true;
		}
		return false;
	}

	virtual void on_window_resized(int width, int height)
	{
	}

	virtual void on_window_scrolled(double xoffset, double yoffset)
	{
	}

	virtual void on_mouse_button_used(int button, int action, int mods)
	{
	}

	virtual void on_mouse_cursor_moved(double xpos, double ypos)
	{
	}

	virtual void on_key_used(int key, int scancode, int action, int mods)
	{
	}

	virtual void on_character_used(unsigned int codepoint)
	{
	}

	virtual void on_draw_simple(const int grid_id)
	{
	}

	virtual void on_draw_with_instancing(const int grid_id)
	{
	}

	virtual void on_draw_overlay()
	{
	}

	virtual void on_before_rendered_frame()
	{
	}

	virtual void on_after_rendered_frame()
	{
	}

	virtual void on_selection(const unsigned int color, const int button_code, const bool mod_ctrl, const bool mod_shift)
	{
	}

private:
	class ModKeysStatusController
	{
	public:
		bool shift_left;
		bool shift_right;
		bool ctrl_left;
		bool ctrl_right;

		ModKeysStatusController() :
			shift_left(false),
			shift_right(false),
			ctrl_left(false),
			ctrl_right(false)
		{
		}

		bool shift_any() const
		{
			return (shift_left || shift_right);
		}

		bool ctrl_any() const
		{
			return (ctrl_left || ctrl_right);
		}

		void update(int key, int scancode, int action, int mods)
		{
			if(key==GLFW_KEY_LEFT_SHIFT)
			{
				update(action, shift_left);
			}
			else if(key==GLFW_KEY_RIGHT_SHIFT)
			{
				update(action, shift_right);
			}
			else if(key==GLFW_KEY_LEFT_CONTROL)
			{
				update(action, ctrl_left);
			}
			else if(key==GLFW_KEY_RIGHT_CONTROL)
			{
				update(action, ctrl_right);
			}
		}

	private:
		static void update(const int action, bool& value)
		{
			value=((action==GLFW_PRESS) || (value && action!=GLFW_RELEASE));
		}
	};

	struct ShadingMode
	{
		enum Mode
		{
			simple,
			with_instancing
		};
	};

	struct RenderingMode
	{
		enum Mode
		{
			simple,
			stereo,
			grid
		};
	};

	static double calculate_stepped_value(const double value, const double value_step, const bool step_up, const bool step_down)
	{
		const double new_value=value*(step_up ? value_step : (step_down ? 1.0/value_step : 1.0));
		if(new_value!=0.0)
		{
			return new_value;
		}
		return value;
	}

	static void callback_on_window_resized(GLFWwindow* window, int width, int height)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(glfwGetWindowUserPointer(window));
		app->callback_on_window_resized(width, height);
	}

	static void callback_on_window_scrolled(GLFWwindow* window, double xoffset, double yoffset)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(glfwGetWindowUserPointer(window));
		app->callback_on_window_scrolled(xoffset, yoffset);
	}

	static void callback_on_mouse_button_used(GLFWwindow* window, int button, int action, int mods)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(glfwGetWindowUserPointer(window));
		app->callback_on_mouse_button_used(button, action, mods);
	}

	static void callback_on_mouse_cursor_moved(GLFWwindow* window, double xpos, double ypos)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(glfwGetWindowUserPointer(window));
		app->callback_on_mouse_cursor_moved(xpos, ypos);
	}

	static void callback_on_key_used(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(glfwGetWindowUserPointer(window));
		app->callback_on_key_used(key, scancode, action, mods);
	}

	static void callback_on_character_used(GLFWwindow* window, unsigned int codepoint)
	{
		ViewerApplication* app=static_cast<ViewerApplication*>(glfwGetWindowUserPointer(window));
		app->callback_on_character_used(codepoint);
	}

	void callback_on_window_resized(int width, int height)
	{
		window_width_=width;
		window_height_=height;
		refresh_shading_projection();
		on_window_resized(width, height);
	}

	void callback_on_window_scrolled(double xoffset, double yoffset)
	{
		const bool step_up=(yoffset<-0.1);
		const bool step_down=(yoffset>0.1);
		zoom_value_=calculate_stepped_value(zoom_value_, zoom_value_step_, step_up, step_down);
		refresh_shading_viewtransform();
		on_window_scrolled(xoffset, yoffset);
	}

	void callback_on_mouse_button_used(int button, int action, int mods)
	{
		if(button==GLFW_MOUSE_BUTTON_LEFT)
		{
			if(action==GLFW_PRESS)
			{
				mouse_button_left_down_=true;
				mouse_button_left_click_possible_=true;
			}
			else if(action==GLFW_RELEASE)
			{
				mouse_button_left_down_=false;
				if(mouse_button_left_click_possible_)
				{
					call_for_selection_=1;
				}
				mouse_button_left_click_possible_=false;
			}
		}
		if(button==GLFW_MOUSE_BUTTON_RIGHT)
		{
			if(action==GLFW_PRESS)
			{
				mouse_button_right_down_=true;
				mouse_button_right_click_possible_=true;
			}
			else if(action==GLFW_RELEASE)
			{
				mouse_button_right_down_=false;
				if(mouse_button_right_click_possible_)
				{
					call_for_selection_=2;
				}
				mouse_button_right_click_possible_=false;
			}
		}
		on_mouse_button_used(button, action, mods);
	}

	void callback_on_mouse_cursor_moved(double xpos, double ypos)
	{
		mouse_cursor_prev_x_=mouse_cursor_x_;
		mouse_cursor_prev_y_=mouse_cursor_y_;
		mouse_cursor_x_=xpos;
		mouse_cursor_y_=ypos;
		mouse_button_left_click_possible_=false;
		mouse_button_right_click_possible_=false;
		perform_trackball_operation(
				mouse_button_left_down_,
				(mouse_button_right_down_ && !modkeys_status_.shift_any() && !modkeys_status_.ctrl_any()),
				(mouse_button_right_down_ && modkeys_status_.shift_any() && !modkeys_status_.ctrl_any()),
				(mouse_button_right_down_ && !modkeys_status_.shift_any() && modkeys_status_.ctrl_any()));
		on_mouse_cursor_moved(xpos, ypos);
	}

	void callback_on_key_used(int key, int scancode, int action, int mods)
	{
		modkeys_status_.update(key, scancode, action, mods);
		on_key_used(key, scancode, action, mods);
	}

	void callback_on_character_used(unsigned int codepoint)
	{
		on_character_used(codepoint);
	}

	bool mouse_cursor_not_on_margin() const
	{
		const int x=static_cast<int>(mouse_cursor_x_);
		const int y=window_height_-static_cast<int>(mouse_cursor_y_);
		return (x>margin_left_ && x<(window_width_-margin_right_) && y>margin_bottom_ && y<(window_height_-margin_top_));
	}

	int full_rendering_area_width() const
	{
		return (window_width_-margin_left_-margin_right_);
	}

	int full_rendering_area_height() const
	{
		return (window_height_-margin_bottom_-margin_top_);
	}

	void render()
	{
		if(!good())
		{
			return;
		}

		on_before_rendered_frame();

		glfwPollEvents();

		glViewport(0, 0, window_width_, window_height_);
		glClearColor(background_color_[0], background_color_[1], background_color_[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport(margin_left_, margin_bottom_, full_rendering_area_width(), full_rendering_area_height());

		if(call_for_selection_>0 && mouse_cursor_not_on_margin())
		{
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			{
				//glDisable(GL_MULTISAMPLE);

				shading_simple_.set_selection_mode_enabled(true);
				render_scene(ShadingMode::simple);
				shading_simple_.set_selection_mode_enabled(false);

				shading_with_instancing_.set_selection_mode_enabled(true);
				render_scene(ShadingMode::with_instancing);
				shading_with_instancing_.set_selection_mode_enabled(false);

				//glEnable(GL_MULTISAMPLE);
			}

			unsigned char pixel[4]={0, 0, 0, 0};
			glReadPixels(static_cast<int>(mouse_cursor_x_), window_height_-static_cast<int>(mouse_cursor_y_), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
			const unsigned int color=static_cast<unsigned int>(pixel[0])*256*256+static_cast<unsigned int>(pixel[1])*256+static_cast<unsigned int>(pixel[2]);
			if(color!=0xFFFFFF)
			{
				on_selection(color, call_for_selection_, modkeys_status_.ctrl_any(), modkeys_status_.shift_any());
			}

			glClearColor(background_color_[0], background_color_[1], background_color_[2], 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		call_for_selection_=0;

		shading_simple_.enable();
		render_scene(ShadingMode::simple);

		shading_with_instancing_.enable();
		render_scene(ShadingMode::with_instancing);

		glViewport(0, 0, window_width_, window_height_);
		glUseProgram(0);
		on_draw_overlay();

		glfwSwapBuffers(window_);

		on_after_rendered_frame();
	}

	void render_scene(const ShadingMode::Mode shading_mode)
	{
		if(rendering_mode_==RenderingMode::stereo)
		{
			const int width=full_rendering_area_width()/2;
			{
				refresh_shading_projection(width, full_rendering_area_height(), shading_mode);
			}
			for(int i=0;i<2;i++)
			{
				{
					TransformationMatrixController viewtransform_matrix;
					viewtransform_matrix.add_scaling(zoom_value_);
					viewtransform_matrix.add_rotation((i==0 ? stereo_angle_ : -stereo_angle_), glm::vec3(0.0f, 1.0f, 0.0f));
					viewtransform_matrix.add_translation(glm::vec3((i==0 ? -stereo_offset_ : stereo_offset_), 0.0f, 0.0f));
					refresh_shading_viewtransform(viewtransform_matrix, shading_mode);
				}
				const int xpos=margin_left_+(width*i);
				glViewport(xpos, margin_bottom_, width, full_rendering_area_height());
				draw(shading_mode, 0);
			}
			refresh_shading_viewtransform(shading_mode);
			refresh_shading_projection(shading_mode);
		}
		else if(rendering_mode_==RenderingMode::grid && grid_size_>1)
		{
			int n_rows=1;
			int n_columns=1;
			Utilities::calculate_grid_dimensions(grid_size_, full_rendering_area_width(), full_rendering_area_height(), n_rows, n_columns);
			const int width=full_rendering_area_width()/n_columns;
			const int height=full_rendering_area_height()/n_rows;
			{
				refresh_shading_projection(width, height, shading_mode);
			}
			int grid_id=0;
			for(int i=0;(i<n_rows) && (grid_id<grid_size_);i++)
			{
				const int ypos=margin_bottom_+(height*(n_rows-1-i));
				for(int j=0;(j<n_columns) && (grid_id<grid_size_);j++)
				{
					const int xpos=margin_left_+(width*j);
					glViewport(xpos, ypos, width, height);
					draw(shading_mode, grid_id);
					grid_id++;
				}
			}
			refresh_shading_projection(shading_mode);
		}
		else
		{
			draw(shading_mode, 0);
		}
	}

	void draw(const ShadingMode::Mode shading_mode, const int grid_id)
	{
		if(shading_mode==ShadingMode::simple)
		{
			on_draw_simple(grid_id);
		}
		else if(shading_mode==ShadingMode::with_instancing)
		{
			on_draw_with_instancing(grid_id);
		}
	}

	void refresh_shading_projection(const TransformationMatrixController& projection_matrix, const ShadingMode::Mode shading_mode)
	{
		if(shading_mode==ShadingMode::simple)
		{
			shading_simple_.set_projection_matrix(projection_matrix.matrix_data());
		}
		else if(shading_mode==ShadingMode::with_instancing)
		{
			shading_with_instancing_.set_projection_matrix(projection_matrix.matrix_data());
		}
	}

	void refresh_shading_projection(const int new_width, const int new_height, const ShadingMode::Mode shading_mode)
	{
		const TransformationMatrixController projection_matrix(new_width, new_height, ortho_z_near_, ortho_z_far_);
		refresh_shading_projection(projection_matrix, shading_mode);
	}

	void refresh_shading_projection(const ShadingMode::Mode shading_mode)
	{
		refresh_shading_projection(full_rendering_area_width(), full_rendering_area_height(), shading_mode);
	}

	void refresh_shading_projection()
	{
		refresh_shading_projection(ShadingMode::simple);
		refresh_shading_projection(ShadingMode::with_instancing);
	}

	void refresh_shading_viewtransform(const TransformationMatrixController& viewtransform_matrix, const ShadingMode::Mode shading_mode)
	{
		if(shading_mode==ShadingMode::simple)
		{
			shading_simple_.set_viewtransform_matrix(viewtransform_matrix.matrix_data());
		}
		else if(shading_mode==ShadingMode::with_instancing)
		{
			shading_with_instancing_.set_viewtransform_matrix(viewtransform_matrix.matrix_data());
		}
	}

	void refresh_shading_viewtransform(const ShadingMode::Mode shading_mode)
	{
		TransformationMatrixController viewtransform_matrix;
		viewtransform_matrix.add_scaling(zoom_value_);
		refresh_shading_viewtransform(viewtransform_matrix, shading_mode);
	}

	void refresh_shading_viewtransform()
	{
		refresh_shading_viewtransform(ShadingMode::simple);
		refresh_shading_viewtransform(ShadingMode::with_instancing);
	}

	void refresh_shading_modeltransform(const TransformationMatrixController& modeltransform_matrix, const ShadingMode::Mode shading_mode)
	{
		if(shading_mode==ShadingMode::simple)
		{
			shading_simple_.set_modeltransform_matrix(modeltransform_matrix.matrix_data());
		}
		else if(shading_mode==ShadingMode::with_instancing)
		{
			shading_with_instancing_.set_modeltransform_matrix(modeltransform_matrix.matrix_data());
		}
	}

	void refresh_shading_modeltransform(const ShadingMode::Mode shading_mode)
	{
		refresh_shading_modeltransform(modeltransform_matrix_, shading_mode);
	}

	void refresh_shading_modeltransform()
	{
		refresh_shading_modeltransform(ShadingMode::simple);
		refresh_shading_modeltransform(ShadingMode::with_instancing);
	}

	bool perform_trackball_operation(const bool rotate, const bool translate, const bool scale, const bool cut)
	{
		if(!(rotate || translate || scale || cut))
		{
			return false;
		}

		if(mouse_cursor_x_==mouse_cursor_prev_x_ && mouse_cursor_y_==mouse_cursor_prev_y_)
		{
			return false;
		}

		if(!mouse_cursor_not_on_margin())
		{
			return false;
		}

		bool change_in_modeltransform=false;
		bool change_in_viewtransform=false;
		bool change_in_projection=false;

		const double cursor_x=(mouse_cursor_x_-static_cast<double>(margin_left_));
		const double cursor_y=(static_cast<double>(window_height_)-mouse_cursor_y_-static_cast<double>(margin_bottom_));
		const double cursor_prev_x=(mouse_cursor_prev_x_-static_cast<double>(margin_left_));
		const double cursor_prev_y=(static_cast<double>(window_height_)-mouse_cursor_prev_y_-static_cast<double>(margin_bottom_));

		const double area_width=static_cast<double>(window_width_-margin_left_-margin_right_);
		const double area_height=static_cast<double>(window_height_-margin_bottom_-margin_top_);
		const double trackball_size=std::min(area_width, area_height);
		const double area_width_in_trackball_size=area_width/trackball_size;
		const double area_height_in_trackball_size=area_height/trackball_size;

		const double p1_x=cursor_prev_x/trackball_size*2-area_width_in_trackball_size;
		const double p1_y=cursor_prev_y/trackball_size*2-area_height_in_trackball_size;

		const double p2_x=cursor_x/trackball_size*2-area_width_in_trackball_size;
		const double p2_y=cursor_y/trackball_size*2-area_height_in_trackball_size;

		if(rotate)
		{
			const double qr1=p1_x*p1_x+p1_y*p1_y;
			const double qr2=p2_x*p2_x+p2_y*p2_y;

			if(qr1<1.0 && qr2<1.0)
			{
				const glm::vec3 p1(static_cast<float>(p1_x), static_cast<float>(p1_y), static_cast<float>(sqrt(1-qr1)));
				const glm::vec3 p2(static_cast<float>(p2_x), static_cast<float>(p2_y), static_cast<float>(sqrt(1-qr2)));
				const glm::vec3 axis=glm::cross(p1, p2);
				const double axis_length=glm::length(axis);
				if(axis_length>0.0)
				{
					const float angle=asin(axis_length);
					if(angle!=0.0f)
					{
						modeltransform_matrix_.add_rotation(angle, glm::normalize(axis));
						change_in_modeltransform=true;
					}
				}
			}
		}

		if(translate)
		{
			const double mod=1.0/zoom_value_;
			const glm::vec3 offset(static_cast<float>((p2_x-p1_x)*mod), static_cast<float>((p2_y-p1_y)*mod), 0.0f);
			modeltransform_matrix_.add_translation(offset);
			change_in_modeltransform=true;
		}

		if(scale)
		{
			zoom_value_+=(p2_y-p1_y)*zoom_value_;
			change_in_viewtransform=true;
		}

		if(cut)
		{
			ortho_z_near_+=(p2_y-p1_y);
			change_in_projection=true;
		}

		if(change_in_modeltransform)
		{
			refresh_shading_modeltransform();
		}

		if(change_in_viewtransform)
		{
			refresh_shading_viewtransform();
		}

		if(change_in_projection)
		{
			refresh_shading_projection();
		}

		return (change_in_modeltransform || change_in_viewtransform || change_in_projection);
	}

	static int default_minimum_window_width()
	{
		return 320;
	}

	static int default_minimum_window_height()
	{
		return 240;
	}

	bool good_;
	GLFWwindow* window_;
	int minimum_window_width_;
	int minimum_window_height_;
	int window_width_;
	int window_height_;
	int margin_left_;
	int margin_right_;
	int margin_bottom_;
	int margin_top_;
	double zoom_value_;
	double zoom_value_step_;
	double ortho_z_near_;
	double ortho_z_far_;
	double ortho_z_step_;
	bool mouse_button_left_down_;
	bool mouse_button_right_down_;
	bool mouse_button_left_click_possible_;
	bool mouse_button_right_click_possible_;
	double mouse_cursor_x_;
	double mouse_cursor_y_;
	double mouse_cursor_prev_x_;
	double mouse_cursor_prev_y_;
	int call_for_selection_;
	float stereo_angle_;
	float stereo_offset_;
	int grid_size_;
	RenderingMode::Mode rendering_mode_;
	float background_color_[3];
	ShadingController shading_simple_;
	ShadingController shading_with_instancing_;
	TransformationMatrixController modeltransform_matrix_;
	ModKeysStatusController modkeys_status_;
};

}

#endif /* UV_VIEWER_APPLICATION_H_ */

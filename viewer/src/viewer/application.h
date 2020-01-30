#ifndef VIEWER_APPLICATION_H_
#define VIEWER_APPLICATION_H_

#include "../uv/viewer_application.h"

#include "../imgui/imgui_impl_glfw_gl3.h"

#include "script_execution_manager.h"
#include "widgets/console.h"
#include "widgets/cursor_label.h"
#include "widgets/waiting_indicator.h"

namespace voronota
{

namespace viewer
{

class Application : public uv::ViewerApplication
{
public:
	static Application& instance()
	{
		static Application app;
		return app;
	}

	void enqueue_script(const std::string& script)
	{
		static const std::string javascript_switch="js:";
		if(script.rfind(javascript_switch, 0)==0)
		{
			enqueue_job(Job(script.substr(javascript_switch.size()), Job::TYPE_JAVASCRIPT));
		}
		else
		{
			enqueue_job(Job(script, Job::TYPE_NATIVE));
		}
	}

	const std::string& execute_native_script(const std::string& script)
	{
		while(!job_queue_.empty())
		{
			dequeue_job();
		}
		return script_execution_manager_.execute_script_and_return_last_output_string(script, false);
	}

	void upload_file(const std::string& name, const std::string& data)
	{
		std::string object_name=name;
		if(object_name.empty())
		{
			object_name="uploaded.pdb";
		}
		std::string virtual_file_name=std::string("_virtual/")+object_name;
		scripting::VirtualFileStorage::set_file(virtual_file_name, data);
		enqueue_script(std::string("import --include-heteroatoms --file ")+virtual_file_name+" ; delete-virtual-files "+virtual_file_name);
	}

protected:
	void on_after_init_success()
	{
		if(good())
		{
			ImGui_ImplGlfwGL3_Init(window(), false);
			ImGuiIO& io=ImGui::GetIO();
			io.IniFilename=0;

			Environment::setup_javascript_bindings(script_execution_manager_);
		}
	}

	bool check_mouse_button_use_intercepted(int button, int action, int mods)
	{
		ImGui_ImplGlfwGL3_MouseButtonCallback(window(), button, action, mods);
		return (ImGui::IsMouseHoveringAnyWindow());
	}

	bool check_mouse_cursor_move_intercepted(double xpos, double ypos)
	{
		ImGui_ImplGlfwGL3_CursorPosCallback(window(), xpos, ypos);
		return (ImGui::IsMouseHoveringAnyWindow());
	}

	void on_mouse_cursor_moved(double xpos, double ypos)
	{
		cursor_label_.clear();
	}

	void on_key_used(int key, int scancode, int action, int mods)
	{
		if(action==GLFW_PRESS)
		{
			if(key==GLFW_KEY_F4)
			{
				RuntimeParameters::instance().enabled_menu=!RuntimeParameters::instance().enabled_menu;
				RuntimeParameters::instance().enabled_console=RuntimeParameters::instance().enabled_menu;
			}
			else if(key==GLFW_KEY_ENTER || key==GLFW_KEY_SPACE || key==GLFW_KEY_UP || key==GLFW_KEY_DOWN)
			{
				if(hovered())
				{
					console_.set_focused(true);
				}
			}
		}

		ImGui_ImplGlfwGL3_KeyCallback(window(), key, scancode, action, mods);
	}

	void on_character_used(unsigned int codepoint)
	{
		ImGui_ImplGlfwGL3_CharCallback(window(), codepoint);
	}

	void on_draw_simple(const int grid_id)
	{
		script_execution_manager_.draw(false, grid_id);
	}

	void on_draw_with_instancing(const int grid_id)
	{
		script_execution_manager_.draw(true, grid_id);
	}

	void on_draw_overlay_start()
	{
		ImGui_ImplGlfwGL3_NewFrame();

		execute_menu();

		cursor_label_.execute(mouse_x(),  mouse_y());

		{
			console_.set_enabled(RuntimeParameters::instance().enabled_console);
			const std::string console_result=console_.execute_on_bottom(window_width(), window_height(), 2);
			enqueue_script(console_result);
		}
	}

	void on_draw_overlay_middle(const int box_x, const int box_y, const int box_w, const int box_h, const bool stereo, const bool grid, const int id)
	{
		execute_info_box(box_x, box_y, box_w, box_h, stereo, grid, id);
	}

	void on_draw_overlay_end()
	{
		if(!widgets::WaitingIndicator::instance().decided())
		{
			widgets::WaitingIndicator::instance().set_enabled(check_big_job_ahead());
		}

		widgets::WaitingIndicator::instance().execute(window_width(), window_height());

		ImGui::Render();
	}

	void on_before_rendered_frame()
	{
		script_execution_manager_.setup_grid_parameters();
	}

	void on_after_rendered_frame()
	{
		if(widgets::WaitingIndicator::instance().enabled() && !widgets::WaitingIndicator::instance().executed())
		{
			return;
		}

		dequeue_job();

		widgets::WaitingIndicator::instance().reset();

		if(script_execution_manager_.exit_requested())
		{
			close();
		}
	}

	void on_selection(const unsigned int drawing_id, const int button_code, const bool mod_ctrl, const bool mod_shift)
	{
		if(button_code==2)
		{
			std::ostringstream output_label;
			if(script_execution_manager_.generate_click_label(drawing_id, output_label))
			{
				cursor_label_.set(output_label.str());
			}
		}
		{
			std::ostringstream output_script;
			if(script_execution_manager_.generate_click_script(drawing_id, button_code, mod_ctrl, mod_shift, output_script))
			{
				enqueue_script(output_script.str());
			}
		}
	}

private:
	struct Job
	{
		enum Type
		{
			TYPE_NATIVE,
			TYPE_JAVASCRIPT
		};

		std::string script;
		Type type;

		Job(const std::string& script, const Type type) : script(script), type(type)
		{
		}
	};

	Application()
	{
		set_background_color(0xCCCCCC);
	}

	~Application()
	{
		if(good())
		{
			ImGui_ImplGlfwGL3_Shutdown();
		}
	}

	void enqueue_job(const Job& job)
	{
		if(job.script.find_first_not_of(" \t\n")!=std::string::npos)
		{
			job_queue_.push_back(job);
		}
	}

	bool dequeue_job()
	{
		if(!job_queue_.empty())
		{
			const Job job=job_queue_.front();
			job_queue_.pop_front();
			if(job.type==Job::TYPE_NATIVE)
			{
				script_execution_manager_.execute_script(job.script, false);
			}
			else if(job.type==Job::TYPE_JAVASCRIPT)
			{
				Environment::execute_javascript(job.script);
			}
		}
		return (!job_queue_.empty());
	}

	void execute_menu()
	{
		if(!RuntimeParameters::instance().enabled_menu)
		{
			return;
		}

		if(ImGui::BeginMainMenuBar())
		{
			if(ImGui::BeginMenu("Settings"))
			{
				if(ImGui::BeginMenu("Background color"))
				{
					if(ImGui::MenuItem("White", ""))
					{
						enqueue_script("background white");
					}
					if(ImGui::MenuItem("Gray", ""))
					{
						enqueue_script("background 0xCCCCCC");
					}
					if(ImGui::MenuItem("Black", ""))
					{
						enqueue_script("background black");
					}
					ImGui::EndMenu();
				}
				if(ImGui::BeginMenu("Grid mode"))
				{
					if(ImGui::MenuItem("Disable", "", false, rendering_mode_is_grid()))
					{
						enqueue_script("mono");
					}
					if(ImGui::MenuItem("Enable grid by object", "", false, !rendering_mode_is_grid()))
					{
						enqueue_script("grid-by-object");
					}
					ImGui::EndMenu();
				}
				if(ImGui::BeginMenu("Projection mode"))
				{
					if(ImGui::MenuItem("Orthographic", "", false, !projection_mode_is_ortho()))
					{
						enqueue_script("ortho");
					}
					if(ImGui::MenuItem("Perspective", "", false, !projection_mode_is_perspective()))
					{
						enqueue_script("perspective");
					}
					ImGui::EndMenu();
				}
				if(ImGui::BeginMenu("Stereo"))
				{
					if(ImGui::MenuItem("Disable", "", false, rendering_mode_is_stereo()))
					{
						enqueue_script("mono");
					}
					if(ImGui::MenuItem("Enable", "", false, !rendering_mode_is_stereo()))
					{
						enqueue_script("stereo");
					}
					ImGui::EndMenu();
				}
				if(ImGui::BeginMenu("Object text description"))
				{
					if(ImGui::MenuItem("Disable", "", false, RuntimeParameters::instance().enabled_info_box))
					{
						RuntimeParameters::instance().enabled_info_box=false;
					}
					if(ImGui::MenuItem("Enable", "", false, !RuntimeParameters::instance().enabled_info_box))
					{
						RuntimeParameters::instance().enabled_info_box=true;
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void execute_info_box(const int box_x, const int box_y, const int box_w, const int box_h, const bool stereo, const bool grid, const int id)
	{
		if(!RuntimeParameters::instance().enabled_info_box)
		{
			return;
		}

		std::ostringstream output_text;
		if(!script_execution_manager_.generate_text_description(id, output_text))
		{
			return;
		}

		std::ostringstream window_name;
		{
			window_name << "text_description_";
			if(stereo)
			{
				window_name << "stereo_" << id;
			}
			if(grid)
			{
				window_name << "grid_" << id;
			}
		}

		{
			std::string raw_text=output_text.str();
			int text_height=1;
			int text_width=0;
			{
				int line_width=0;
				for(std::size_t i=0;i<raw_text.size();i++)
				{
					if(raw_text[i]=='\n')
					{
						text_height++;
						text_width=std::max(text_width, line_width);
						line_width=0;
					}
					else
					{
						line_width++;
					}
				}
				text_width=std::max(text_width, line_width);
			}

			ImGui::SetNextWindowPos(ImVec2(box_x+5, box_y+35), 0);
			ImGui::SetNextWindowSize(ImVec2(3+(text_width*8), 15+(text_height*15)));
			ImVec4 color_text=ImVec4(0.5f, 0.0f, 1.0f, 1.0f);
			ImGui::Begin(window_name.str().c_str(), 0, ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
			ImGui::PushStyleColor(ImGuiCol_Text, color_text);
			ImGui::Text("%s", raw_text.c_str());
			ImGui::PopStyleColor();
			ImGui::End();
		}
	}

	bool check_big_job_ahead() const
	{
		if(!job_queue_.empty())
		{
			const std::string& script=job_queue_.front().script;
			if(script.size()>500
					|| script.find("import")!=std::string::npos
					|| script.find("export")!=std::string::npos
					|| script.find("construct")!=std::string::npos
					|| script.find("calculate")!=std::string::npos
					|| script.find("make")!=std::string::npos
					|| script.find("add")!=std::string::npos
					|| script.find("restrict")!=std::string::npos
					|| script.find("move")!=std::string::npos
					|| script.find("copy")!=std::string::npos
					|| script.find("setup")!=std::string::npos
					|| script.find("source")!=std::string::npos
					|| script.find("score")!=std::string::npos
					|| script.find("voromqa")!=std::string::npos)
			{
				return true;
			}
		}
		return false;
	}

	ScriptExecutionManager script_execution_manager_;
	std::list<Job> job_queue_;
	widgets::Console console_;
	widgets::CursorLabel cursor_label_;
};

}

}

#endif /* VIEWER_APPLICATION_H_ */

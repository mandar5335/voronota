#ifndef WIDGETS_WAITING_INDICATOR_H_
#define WIDGETS_WAITING_INDICATOR_H_

#include "../imgui/imgui_impl_glfw_gl3.h"

namespace widgets
{

class WaitingIndicator
{
public:
	WaitingIndicator() :
		waiting_limit_(3),
		waiting_stage_(0)
	{
	}

	bool check_waiting()
	{
		waiting_stage_=std::min(waiting_stage_+1, waiting_limit_);
		return (waiting_stage_<waiting_limit_);
	}

	void keep_waiting(const bool need)
	{
		if(need)
		{
			waiting_stage_=(waiting_limit_-1);
		}
		else
		{
			waiting_stage_=0;
		}
	}

	void disable()
	{
		waiting_stage_=waiting_limit_;
	}

	void execute(const int window_width, const int window_height)
	{
		if(waiting_stage_<1 || waiting_stage_>=waiting_limit_)
		{
			return;
		}

		const int label_width=150;
		const int label_height=30;
		const int label_x_pos=(window_width/2)-(label_width/2);
		const int label_y_pos=(window_height/2)-(label_height/2);

		static bool open=false;

		ImGui::SetNextWindowPos(ImVec2(label_x_pos, label_y_pos));
		ImGui::SetNextWindowSize(ImVec2(label_width, label_height));
		ImGui::Begin("Waiting", &open, ImVec2(0, 0), 0.75f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("Please wait ...");
		ImGui::End();
	}

private:
	int waiting_limit_;
	int waiting_stage_;
};

}

#endif /* WIDGETS_WAITING_INDICATOR_H_ */

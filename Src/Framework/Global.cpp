#include "stdafx.h"
#include "global.h"


const char* const SAMPLE_NAME = "SVGF";
unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;
bool         camera_changed = true;

const float FPS_UPDATE_INTERVAL = 0.5;  


void displayFps(unsigned int frame_count,GLFWwindow* window)
{
	static double fps = -1.0;
	static unsigned last_frame_count = 0;
	static double last_update_time = glfwGetTime();
	static double current_time = 0.0;
	current_time = glfwGetTime();
	if (current_time - last_update_time > FPS_UPDATE_INTERVAL) {
		fps = (frame_count - last_frame_count) / (current_time - last_update_time);
		last_frame_count = frame_count;
		last_update_time = current_time;
	}
	if (frame_count > 0 && fps >= 0.0) {	
		static char fps_text[32];
		sprintf_s(fps_text, "fps: %7.2f", fps);
		glfwSetWindowTitle(window, fps_text);
	}
}

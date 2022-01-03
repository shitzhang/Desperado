#pragma once

#include <fstream>
#include <vector>

dlldecl extern unsigned int SCR_WIDTH;
dlldecl extern unsigned int SCR_HEIGHT;
extern bool         camera_changed;

extern const char* const SAMPLE_NAME;

extern void displayFps(unsigned int frame_count, GLFWwindow* window);

extern const float FPS_UPDATE_INTERVAL;
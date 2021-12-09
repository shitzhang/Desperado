#pragma once

#include <fstream>
#include <vector>

// settings
extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;
extern bool         camera_changed;

extern const char* const SAMPLE_NAME;

extern const std::vector<std::string> skyboxFaces;

//extern unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);
//extern unsigned int createConstantTexture(const unsigned char* color, bool gamma);
extern unsigned int loadTexture(char const* path);
extern unsigned int loadCubemap(std::vector<std::string> faces);
extern void displayFps(unsigned int frame_count, GLFWwindow* window);

extern const float FPS_UPDATE_INTERVAL;
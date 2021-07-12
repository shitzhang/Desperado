#pragma once

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"


#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

// settings
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;

extern const vector<string> skyboxFaces;

extern unsigned int TextureFromFile(const char* path, const string& directory, bool gamma);
extern unsigned int createConstantTexture(const unsigned char* color, bool gamma);
extern unsigned int loadTexture(char const* path);
extern unsigned int loadCubemap(vector<std::string> faces);
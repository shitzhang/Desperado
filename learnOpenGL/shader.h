#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Shader {
public:
	//����ID
	unsigned int ID;

	//��������ȡ��������ɫ��
	Shader(const char* vertexPath, const char* fragmentPath);
	//ʹ��/�������
	void use();
	//uniform���ߺ���
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec3(const std::string& name, glm::vec3 value) const;
	void setVec3(const std::string& name, float v1, float v2, float v3) const;
	void setMat4(const std::string& name, glm::mat4 value) const;
};

#pragma once

#include "pass.h"
#include "light.h"


class SceneDepthPass :public IRenderPass {
public:
	SceneDepthPass(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera);
	SceneDepthPass(const std::string& vPassName, int vExcutionOrder, std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera);

	virtual void initInfo();

	virtual void updateInfo();

	virtual void render();
private:
	unsigned int m_resolution = 2048;
};

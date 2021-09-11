#pragma once

#include <string>

#include "shader.h"
#include "scene.h"
#include "camera.h"

enum class ERenderPassType
{
	RenderPassType_Normal,
	RenderPassType_Once,
	RenderPassType_Parallel,
	RenderPassType_ParallelOnce,
	RenderPassType_Delay,
};

class IRenderPass
{
public:
	IRenderPass();
	IRenderPass(const std::string& vPassName, int vExcutionOrder);
	IRenderPass(const std::string& vPassName, int vExcutionOrder, ERenderPassType vtype);
	virtual ~IRenderPass();

	virtual void initInfo() = 0;
	virtual void updateInfo() = 0;
	virtual void render() = 0;

	bool operator<(const IRenderPass& vOtherPass) const;

	const std::string& getPassName() const { return m_PassName; }
	int getExecutionOrder() const { return m_ExecutionOrder; }

	void setPassName(const std::string& vPassName) { m_PassName = vPassName; }
	void setExecutionOrder(int vExecutionOrder) { m_ExecutionOrder = vExecutionOrder; }
	ERenderPassType getPassType();
	void setPassType(ERenderPassType vType);
	int getExecutionOrder();
	void finishExecute();
protected:
	std::shared_ptr<Shader> m_pShader;
	std::shared_ptr<Scene> m_pScene;
	std::shared_ptr<Camera> m_pCamera;

	unsigned int m_fbo = -1;
private:
	std::string m_PassName;
	ERenderPassType m_Type = ERenderPassType::RenderPassType_Normal;
	int m_ExecutionOrder = -1;
};
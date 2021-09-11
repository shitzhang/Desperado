#include "pass.h"


IRenderPass::IRenderPass()
{
}

IRenderPass::IRenderPass(const std::string& vPassName, int vExcutionOrder) :m_PassName(vPassName), m_ExecutionOrder(vExcutionOrder)
{
}
IRenderPass::IRenderPass(const std::string& vPassName, int vExcutionOrder, ERenderPassType vtype) : m_PassName(vPassName), m_ExecutionOrder(vExcutionOrder), m_Type(vtype)
{

}
IRenderPass::~IRenderPass()
{
}

bool IRenderPass::operator<(const IRenderPass& vOtherPass) const
{
	return m_ExecutionOrder <= vOtherPass.getExecutionOrder();//保证排序稳定性
}

ERenderPassType IRenderPass::getPassType()
{
	return m_Type;
}

void IRenderPass::setPassType(ERenderPassType vType)
{
	m_Type = vType;
}

int IRenderPass::getExecutionOrder()
{
	return m_ExecutionOrder;
}

void IRenderPass::finishExecute()
{
	m_ExecutionOrder = -1;
}
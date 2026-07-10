#include "Pipeline.h"
#include "DataTypes.h"

wgpu::Pipeline::Pipeline(const Shader& shader)
	: m_Shader{ &shader }
{
}

#ifndef _PIPELINE
#define _PIPELINE

#include "Shader.h"
#include "BindGroupLayout.h"

namespace wgpu
{
	class Pipeline
	{
	public:
		Pipeline(const Shader& shader);

	private:
		const Shader* m_Shader{ nullptr };

		WGPURenderPipeline m_Pipeline{ nullptr };
	};
}

#endif

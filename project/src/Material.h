#ifndef MATERIAL
#define MATERIAL

#include "Pipeline.h"

namespace wgpu
{
	class Material
	{
	public:
		Material(const Pipeline& pipeline);

		WGPURenderPipeline GetPipeline() const { return m_Pipeline->GetPipeline(); }

	private:
		const Pipeline* m_Pipeline{ nullptr };

	};
}

#endif

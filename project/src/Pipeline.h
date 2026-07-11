#ifndef _PIPELINE
#define _PIPELINE

#include "Shader.h"
#include "BindGroupLayout.h"

namespace wgpu
{
	class Pipeline final
	{
	public:
		Pipeline(const Shader& shader, const BindGroupLayout* bindGroupLayout = nullptr);
		~Pipeline() noexcept;

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator=(Pipeline&&) = delete;

		WGPURenderPipeline GetPipeline() const { return m_Pipeline; }
		const GraphicsContext* GetGraphicsContext() const { return m_Shader->GetGraphicsContext(); }

	private:
		const Shader* m_Shader{ nullptr };
		const BindGroupLayout* m_BindGroupLayout{ nullptr };

		WGPURenderPipeline m_Pipeline{ nullptr };
	};
}

#endif

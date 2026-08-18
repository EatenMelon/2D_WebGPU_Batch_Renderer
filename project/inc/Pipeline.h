#ifndef _PIPELINE
#define _PIPELINE

#include "Shader.h"
#include "BindGroupLayout.h"

namespace wgpu
{
	class Pipeline final
	{
	public:
		Pipeline(const Shader& shader, bool isOpaque = true, const BindGroupLayout* bindGroupLayout = nullptr);
		~Pipeline() noexcept;

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator=(Pipeline&&) = delete;

		WGPURenderPipeline GetPipeline() const { return m_Pipeline; }
		const GraphicsContext* GetGraphicsContext() const { return m_Shader->GetRenderer()->GetContext(); }
		const BindGroupLayout* GetBindGroupLayout() const { return m_BindGroupLayout; }
		bool WriteDepth() const { return m_IsOpaque; }

	private:
		const Shader* m_Shader{ nullptr };
		const BindGroupLayout* m_BindGroupLayout{ nullptr };
		bool m_IsOpaque{ true };

		WGPURenderPipeline m_Pipeline{ nullptr };
	};
}

#endif

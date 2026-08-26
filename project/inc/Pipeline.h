#ifndef _PIPELINE
#define _PIPELINE

#include "Shader.h"
#include "BindGroupLayout.h"

namespace wgpu
{
	class Pipeline final
	{
	public:
		enum class Type { GeometryOpaque, GeometryTransparent, PostProcessing };

		Pipeline(const Shader& shader, Type pipelineType, const BindGroupLayout* bindGroupLayout = nullptr);
		~Pipeline() noexcept;

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator=(Pipeline&&) = delete;

		WGPURenderPipeline GetPipeline() const { return m_Pipeline; }
		const GraphicsContext* GetContext() const;
		const BindGroupLayout* GetBindGroupLayout() const { return m_BindGroupLayout; }

		bool WriteDepth() const { return m_Type == Type::GeometryOpaque; }
		Type GetPipelineType() const { return m_Type; }

	private:
		const Shader* m_Shader{ nullptr };
		const BindGroupLayout* m_BindGroupLayout{ nullptr };
		Type m_Type{};

		WGPURenderPipeline m_Pipeline{ nullptr };
	};
}

#endif

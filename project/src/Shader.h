#ifndef _SHADER
#define _SHADER

#include <filesystem>
#include "GraphicsContext.h"

namespace wgpu
{
	class Shader final
	{
	public:
		Shader(const GraphicsContext& context, const std::filesystem::path& path);
		~Shader() noexcept;

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader(Shader&&) = delete;
		Shader& operator=(Shader&&) = delete;

		WGPUShaderModule GetShaderModule() const { return m_ShaderModule; }
		const GraphicsContext* GetGraphicsContext() const { return m_Context; }

	private:
		WGPUShaderModule m_ShaderModule{ nullptr };
		const GraphicsContext* m_Context{ nullptr };
	};
}

#endif

#ifndef _SHADER
#define _SHADER

#include <filesystem>

#include "Renderer2D.h"

namespace wgpu
{
	class Shader final
	{
	public:
		Shader(const Renderer2D& renderer, const std::filesystem::path& path);
		~Shader() noexcept;

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader(Shader&&) = delete;
		Shader& operator=(Shader&&) = delete;

		WGPUShaderModule GetShaderModule() const { return m_ShaderModule; }
		const Renderer2D* GetRenderer() const { return m_Renderer; }

	private:
		WGPUShaderModule m_ShaderModule{ nullptr };

		const Renderer2D* m_Renderer{ nullptr };
	};
}

#endif
